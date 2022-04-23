#include "WindowGrid.h"
#include "Resources.h"
#include "id.h"
#include <wx/graphics.h>
#include <utility>
#include <sstream>
#include <fstream>
#include <wx/propgrid/props.h>

//Helper functions defined at end of file
namespace {
    Item valueDialog(const Item& currentItem, bool numeric);
    int getDirection(const wxPoint& currentCell, const wxPoint& lastCell);
    int getOppositeDirection(int direction);
}

void WindowGrid::OnDraw(wxDC &dc) {
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(pen);
    dc.SetFont(font);
    wxRect updateRect = GetUpdateRegion().GetBox();
    wxPoint tl = CalcUnscrolledPosition(wxPoint{updateRect.GetLeft(), updateRect.GetTop()});
    wxPoint br = CalcUnscrolledPosition(wxPoint{updateRect.GetRight(), updateRect.GetBottom()});
    int cellSize = 128 + 16 * zoomLevels;
    for (int r = (tl.y - cellSize + 1) / cellSize; r < std::min((br.y + cellSize - 1) / cellSize, static_cast<int>(grid.getHeight())); r++) {
        for (int c = (tl.x - cellSize + 1) / cellSize; c < std::min((br.x + cellSize - 1) / cellSize, static_cast<int>(grid.getWidth())); c++) {
            Item item = grid.get(r, c);
            switch (item.getType()) {
                case Item::ItemType::none: {
                    if(dotSize != -1) {
                        dc.DrawCircle(cellSize / 2 + cellSize * c, cellSize / 2 + cellSize * r, std::max(cellSize * dotSize / 128, 1));
                    }
                    break;
                }
                case Item::ItemType::resistor: {
                    std::wstring value = item.getValueStr();
                    if (item.getShape() == Item::HORIZONTAL) {
                        dc.DrawBitmap(resistorBitmaps[0], cellSize * c, cellSize * r);
                        dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r + cellSize * 4 / 17, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                    } else {
                        dc.DrawBitmap(resistorBitmaps[1], cellSize * c, cellSize * r);
                        wxSize textSize = dc.GetTextExtent(value);
                        dc.DrawRotatedText(value, cellSize * c + cellSize * 40 / 51, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
                    }
                    break;
                }
                case Item::ItemType::capacitor: {
                    std::wstring value = item.getValueStr();
                    if (item.getShape() == Item::HORIZONTAL) {
                        dc.DrawBitmap(capacitorBitmaps[0], cellSize * c, cellSize * r);
                        dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r + cellSize * 2 / 17, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                    } else {
                        dc.DrawBitmap(capacitorBitmaps[1], cellSize * c, cellSize * r);
                        wxSize textSize = dc.GetTextExtent(value);
                        dc.DrawRotatedText(value, cellSize * c + cellSize * 31 / 34, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
                    }
                    break;
                }
                case Item::ItemType::wire: {
                    int directions = 0;
                    wxPoint middle = wxPoint{cellSize / 2 + cellSize * c, cellSize / 2 + cellSize * r};
                    if (item.getShape() & Item::UP) {
                        dc.DrawLine(wxPoint{cellSize / 2 + cellSize * c, cellSize * r}, middle);
                        directions++;
                    }
                    if (item.getShape() & Item::DOWN) {
                        dc.DrawLine(wxPoint{cellSize / 2 + cellSize * c, cellSize * (r + 1)}, middle);
                        directions++;
                    }
                    if (item.getShape() & Item::LEFT) {
                        dc.DrawLine(wxPoint{cellSize * c, cellSize / 2 + cellSize * r}, middle);
                        directions++;
                    }
                    if (item.getShape() & Item::RIGHT) {
                        dc.DrawLine(wxPoint{cellSize * (c + 1), cellSize / 2 + cellSize * r}, middle);
                        directions++;
                    }
                    if (directions > 2) {
                        dc.DrawCircle(middle, std::max(cellSize * 3 / 128, 1));
                    }
                    break;
                }
                case Item::ItemType::amp_source: case Item::ItemType::volt_source: {
                    wxBitmap* bitmaps = item.getType() == Item::ItemType::amp_source ? ampSourceBitmaps : voltSourceBitmaps;
                    std::wstring value = item.getValueStr();
                    int bitmapIndex;
                    if(item.getShape() & Item::UP) bitmapIndex = 0;
                    else if(item.getShape() & Item::DOWN) bitmapIndex = 1;
                    else if(item.getShape() & Item::RIGHT) bitmapIndex = 2;
                    else bitmapIndex = 3;
                    if(item.getShape() & Item::DEPENDENT) bitmapIndex += 4;
                    dc.DrawBitmap(bitmaps[bitmapIndex], cellSize * c, cellSize * r);
                    if(item.getShape() & Item::DEPENDENT) {
                        if ((item.getShape() & Item::LEFT) || (item.getShape() & Item::RIGHT)) {
                            dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                        } else {
                            wxSize textSize = dc.GetTextExtent(value);
                            dc.DrawRotatedText(value, cellSize * (c + 1) + textSize.GetWidth() / 10, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
                        }
                    } else {
                        if ((item.getShape() & Item::LEFT) || (item.getShape() & Item::RIGHT)) {
                            dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r + cellSize * 7 / 34, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                        } else {
                            wxSize textSize = dc.GetTextExtent(value);
                            dc.DrawRotatedText(value, cellSize * c + cellSize * 83 / 102, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
                        }
                    }
                    break;
                }
            }
        }
    }
}

WindowGrid::WindowGrid(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const LoadStruct& load)
        : wxScrolledCanvas(parent, id, pos, size), grid{load.grid}, zoomLevels{load.zoom}, dotSize{load.dotSize} {
    Bind(wxEVT_MOUSEWHEEL, &WindowGrid::onScroll, this);
    Bind(wxEVT_LEFT_DOWN, &WindowGrid::onLeftDown, this);
    Bind(wxEVT_MOTION, &WindowGrid::onMotion, this);
    Bind(wxEVT_RIGHT_DOWN, &WindowGrid::onRightDown, this);

    twoWayMenu.Append(id::set_value, "Set value");
    twoWayMenu.Append(id::rotate, "Rotate");
    wireMenu.Append(id::toggle_up, "Toggle up");
    wireMenu.Append(id::toggle_left, "Toggle left");
    wireMenu.Append(id::toggle_right, "Toggle right");
    wireMenu.Append(id::toggle_down, "Toggle down");
    fourWayMenu.Append(id::set_value, "Set value");
    fourWayMenu.Append(id::rotate, "Rotate CW");
    fourWayMenu.Append(id::rotate_ccw, "Rotate CCW");
    fourWayMenu.Append(id::flip, "Flip");
    fourWayMenu.Append(id::dependent, "Toggle dependent");

    refresh(load.xScroll, load.yScroll);
}

void WindowGrid::refresh(int xPos, int yPos) {
    wxScrolledCanvas::SetScrollbars(16, 16, static_cast<int>(grid.getWidth()) * (8 + zoomLevels), static_cast<int>(grid.getHeight()) * (8 + zoomLevels), xPos, yPos);
    font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.SetPixelSize(wxSize{0, 16 + 2 * zoomLevels});
    pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(22.0 / 1024 * (128 + 16 * zoomLevels)))};
    int size = 128 + zoomLevels * 16;
    resistorBitmaps[0] = resources::getResistorBitmap(size, false);
    resistorBitmaps[1] = resources::getResistorBitmap(size, true);
    voltSourceBitmaps[0] = resources::getVoltSourceBitmap(size, Item::UP);
    voltSourceBitmaps[1] = resources::getVoltSourceBitmap(size, Item::DOWN);
    voltSourceBitmaps[2] = resources::getVoltSourceBitmap(size, Item::RIGHT);
    voltSourceBitmaps[3] = resources::getVoltSourceBitmap(size, Item::LEFT);
    voltSourceBitmaps[4] = resources::getVoltSourceBitmap(size, Item::UP | Item::DEPENDENT);
    voltSourceBitmaps[5] = resources::getVoltSourceBitmap(size, Item::DOWN | Item::DEPENDENT);
    voltSourceBitmaps[6] = resources::getVoltSourceBitmap(size, Item::RIGHT | Item::DEPENDENT);
    voltSourceBitmaps[7] = resources::getVoltSourceBitmap(size, Item::LEFT | Item::DEPENDENT);
    ampSourceBitmaps[0] = resources::getAmpSourceBitmap(size, Item::UP);
    ampSourceBitmaps[1] = resources::getAmpSourceBitmap(size, Item::DOWN);
    ampSourceBitmaps[2] = resources::getAmpSourceBitmap(size, Item::RIGHT);
    ampSourceBitmaps[3] = resources::getAmpSourceBitmap(size, Item::LEFT);
    ampSourceBitmaps[4] = resources::getAmpSourceBitmap(size, Item::UP | Item::DEPENDENT);
    ampSourceBitmaps[5] = resources::getAmpSourceBitmap(size, Item::DOWN | Item::DEPENDENT);
    ampSourceBitmaps[6] = resources::getAmpSourceBitmap(size, Item::RIGHT | Item::DEPENDENT);
    ampSourceBitmaps[7] = resources::getAmpSourceBitmap(size, Item::LEFT | Item::DEPENDENT);
    capacitorBitmaps[0] = resources::getCapacitorBitmap(size, false);
    capacitorBitmaps[1] = resources::getCapacitorBitmap(size, true);
    Refresh();
}

void WindowGrid::reload(const WindowGrid::LoadStruct& load) {
    grid = load.grid;
    zoomLevels = load.zoom;
    dotSize = load.dotSize;
    dirty = false;
    refresh(load.xScroll, load.yScroll);
}


void WindowGrid::onScroll(wxMouseEvent &event) {
    if (event.GetModifiers() == wxMOD_CONTROL) {
        static int rotation = 0;
        rotation += event.GetWheelRotation();
        int rows = rotation / event.GetWheelDelta();
        if (rows != 0) {
            int mouseX, mouseY, scrollX, scrollY;
            event.GetPosition(&mouseX, &mouseY);
            GetViewStart(&scrollX, &scrollY);
            scrollX *= 16; //scroll is in scroll units, not pixels
            scrollY *= 16;
            //Fractional location of the mouse in the grid
            double mouseXFraction = static_cast<double>(mouseX + scrollX) / (grid.getWidth() * (8 + zoomLevels)) / 16;
            double mouseYFraction = static_cast<double>(mouseY + scrollY) / (grid.getHeight() * (8 + zoomLevels)) / 16;
            rotation %= event.GetWheelDelta();
            int zoomLevelsPrev = zoomLevels;
            zoomLevels += rows;
            if (zoomLevels > 20) {
                zoomLevels = 20;
            } else if (zoomLevels < -6) {
                zoomLevels = -6;
            }
            if(zoomLevelsPrev != zoomLevels) {
                double mouseXLogical = (grid.getWidth() * (8 + zoomLevels)) * 16 * mouseXFraction;
                double mouseYLogical = (grid.getHeight() * (8 + zoomLevels)) * 16 * mouseYFraction;
                int newScrollX = static_cast<int>(std::max(mouseXLogical - mouseX, 0.0));
                int newScrollY = static_cast<int>(std::max(mouseYLogical - mouseY, 0.0));
                refresh(newScrollX / 16, newScrollY / 16);
            }
        }
    } else {
        event.Skip();
    }
}

void WindowGrid::onMotion(wxMouseEvent &event) {
    static wxPoint lastMousePos{-1, -1};
    static wxPoint lastScrolledMousePos{0, 0};
    wxPoint mousePos = event.GetPosition();
    int cellSize = 128 + 16 * zoomLevels;
    wxPoint logicalPos = CalcUnscrolledPosition(mousePos);
    wxPoint cell = logicalPos / cellSize;
    if (!(cell.x >= 0 && cell.y >= 0 && cell.x < grid.getWidth() && cell.y < grid.getHeight())) {
        cell = wxPoint{-1, -1};
    }
    if (cell != currentCell) {
        lastCell = currentCell;
        currentCell = cell;
        if (event.LeftIsDown() && currentCell != wxPoint{-1,-1}) {
            switch (selectedTool) {
                case Item::ItemType::none:
                    placePartial(currentCell, Item{});
                    break;
                case Item::ItemType::resistor: {
                    int shape = Item::VERTICAL;
                    if (lastCell.x != currentCell.x) {
                        shape = Item::HORIZONTAL;
                    }
                    placePartial(currentCell, Item{Item::ItemType::resistor, shape, 100});
                    break;
                }
                case Item::ItemType::wire: {
                    int shape = getDirection(lastCell, currentCell);
                    placePartial(currentCell, Item{Item::ItemType::wire, shape, 0});
                    if(lastCell != wxPoint{-1,-1}) {
                        placePartial(lastCell, Item{Item::ItemType::wire, getOppositeDirection(shape), 0});
                    }
                    break;
                }
                case Item::ItemType::volt_source: {
                    placePartial(currentCell, Item{Item::ItemType::volt_source, getDirection(currentCell, lastCell), 10});
                    break;
                }
                case Item::ItemType::amp_source: {
                    placePartial(currentCell, Item{Item::ItemType::amp_source, getDirection(currentCell, lastCell), 10});
                    break;
                }
                case Item::ItemType::capacitor:
                    int shape = Item::VERTICAL;
                    if (lastCell.x != currentCell.x) {
                        shape = Item::HORIZONTAL;
                    }
                    placePartial(currentCell, Item{Item::ItemType::capacitor, shape, 1e-4});
                    break;
            }
        }
    }
    if (event.MiddleIsDown()) {
        wxPoint diff = lastScrolledMousePos - mousePos;
        wxPoint scrollCurrent = GetViewStart();
        wxPoint scrollChange{0, 0};
        if (diff.x >= 16 || diff.x <= -16) {
            scrollChange.x = diff.x / 16;
            lastScrolledMousePos.x = mousePos.x;
        }
        if (diff.y >= 16 || diff.y <= -16) {
            scrollChange.y = diff.y / 16;
            lastScrolledMousePos.y = mousePos.y;
        }
        if (scrollChange != wxPoint{0, 0}) {
            Scroll(scrollCurrent + scrollChange);
        }
    } else {
        lastScrolledMousePos = mousePos;
    }
    lastMousePos = mousePos;
    event.Skip();
}

void WindowGrid::placePartial(wxPoint cell, const Item& item) {
    int cellSize = 128 + 16 * zoomLevels;
    wxRect affectedRect{CalcScrolledPosition(cellSize * cell) - wxPoint{5,5}, wxSize{cellSize + 10, cellSize + 10}};
    Item currentItem = grid.get(cell.y, cell.x);
    switch (item.getType()) {
        case Item::ItemType::none: {
            if (currentItem.getType() != Item::ItemType::none) {
                grid.set(cell.y, cell.x, item);
                dirty = true;
                RefreshRect(affectedRect);
            }
            break;
        }
        case Item::ItemType::wire: {
            if (currentItem.getType() == Item::ItemType::none) {
                grid.set(cell.y, cell.x, item);
                dirty = true;
                RefreshRect(affectedRect);
            } else if (currentItem.getType() == Item::ItemType::wire && !(currentItem.getShape() & item.getShape())) {
                Item newItem{Item::ItemType::wire, item.getShape() | currentItem.getShape(), 0};
                grid.set(cell.y, cell.x, newItem);
                dirty = true;
                RefreshRect(affectedRect);
            }
            break;
        }
        case Item::ItemType::resistor: case Item::ItemType::volt_source: case Item::ItemType::amp_source: case Item::ItemType::capacitor: {
            if (currentItem.getType() == item.getType()) {
                if (currentItem.getShape() != item.getShape()) {
                    Item newItem{item.getType(), item.getShape(), currentItem.getValue(), currentItem.getExtraData()};
                    grid.set(cell.y, cell.x, newItem);
                    dirty = true;
                    RefreshRect(affectedRect);
                }
            } else if (currentItem.getType() == Item::ItemType::none || currentItem.getType() == Item::ItemType::wire) {
                grid.set(cell.y, cell.x, item);
                dirty = true;
                RefreshRect(affectedRect);
            }
            break;
        }
    }
}

void WindowGrid::onLeftDown(wxMouseEvent &event) {
    if (currentCell != wxPoint{-1, -1}) {
        switch (selectedTool) {
            case Item::ItemType::none:
                placePartial(currentCell, Item{});
                break;
            case Item::ItemType::resistor: {
                int shape = Item::VERTICAL;
                if (lastCell.x != currentCell.x) {
                    shape = Item::HORIZONTAL;
                }
                placePartial(currentCell, Item{Item::ItemType::resistor, shape, 100});
                break;
            }
            case Item::ItemType::wire: {
                placePartial(currentCell, Item{Item::ItemType::wire, getDirection(lastCell, currentCell), 0});
                break;
            }
            case Item::ItemType::volt_source: {
                placePartial(currentCell, Item{Item::ItemType::volt_source, getDirection(currentCell, lastCell), 10});
                break;
            }
            case Item::ItemType::amp_source: {
                placePartial(currentCell, Item{Item::ItemType::amp_source, getDirection(currentCell, lastCell), 10});
                break;
            }
            case Item::ItemType::capacitor: {
                int shape = Item::VERTICAL;
                if (lastCell.x != currentCell.x) {
                    shape = Item::HORIZONTAL;
                }
                placePartial(currentCell, Item{Item::ItemType::capacitor, shape, 1e-4});
                break;
            }
        }
    }
    event.Skip();
}

void WindowGrid::onRightDown(wxMouseEvent &event) {
    Item currentItem = grid.get(currentCell.y, currentCell.x);
    int cellSize = 128 + 16 * zoomLevels;
    wxRect affectedRect{CalcScrolledPosition(cellSize * currentCell) - wxPoint{5,5}, wxSize{cellSize + 10, cellSize + 10}};
    switch (currentItem.getType()) {
        case Item::ItemType::none: break;
        case Item::ItemType::resistor: case Item::ItemType::capacitor: {
            int response = GetPopupMenuSelectionFromUser(twoWayMenu);
            switch (response) {
                case id::rotate: {
                    Item item{currentItem.getType(), currentItem.getShape() == Item::HORIZONTAL ? Item::VERTICAL : Item::HORIZONTAL, 0};
                    placePartial(currentCell, item);
                    break;
                }
                case id::set_value: {
                    Item item{valueDialog(currentItem, true)};
                    if(item.getType() != Item::ItemType::none) {
                        grid.set(currentCell.y, currentCell.x, item);
                        dirty = true;
                        RefreshRect(affectedRect);
                    }
                    break;
                }
                default: {}
            }
            break;
        }
        case Item::ItemType::wire: {
            int response = GetPopupMenuSelectionFromUser(wireMenu);
            bool changed = true;
            Item item;
            switch (response) {
                case id::toggle_up: {
                    item = {Item::ItemType::wire, currentItem.getShape() ^ Item::UP, 0};
                    break;
                }
                case id::toggle_down: {
                    item = {Item::ItemType::wire, currentItem.getShape() ^ Item::DOWN, 0};
                    break;
                }
                case id::toggle_left: {
                    item = {Item::ItemType::wire, currentItem.getShape() ^ Item::LEFT, 0};
                    break;
                }
                case id::toggle_right: {
                    item = {Item::ItemType::wire, currentItem.getShape() ^ Item::RIGHT, 0};
                    break;
                }
                default: {
                    changed = false;
                }
            }
            if (changed) {
                grid.set(currentCell.y, currentCell.x, item);
                dirty = true;
                RefreshRect(affectedRect);
            }
            break;
        }
        case Item::ItemType::amp_source:
        case Item::ItemType::volt_source: {
            int response = GetPopupMenuSelectionFromUser(fourWayMenu);
            switch (response) {
                case id::rotate: {
                    int shape = currentItem.getShape() & Item::DEPENDENT;
                    if(currentItem.getShape() & Item::UP) shape |= Item::RIGHT;
                    else if(currentItem.getShape() & Item::RIGHT) shape |= Item::DOWN;
                    else if(currentItem.getShape() & Item::DOWN) shape |= Item::LEFT;
                    else shape |= Item::UP;
                    Item item{currentItem.getType(), shape, 0};
                    placePartial(currentCell, item);
                    break;
                }
                case id::rotate_ccw: {
                    int shape = currentItem.getShape() & Item::DEPENDENT;
                    if(currentItem.getShape() & Item::UP) shape |= Item::LEFT;
                    else if(currentItem.getShape() & Item::RIGHT) shape |= Item::UP;
                    else if(currentItem.getShape() & Item::DOWN) shape |= Item::RIGHT;
                    else shape |= Item::DOWN;
                    Item item{currentItem.getType(), shape, 0};
                    placePartial(currentCell, item);
                    break;
                }
                case id::flip: {
                    int shape = currentItem.getShape() & Item::DEPENDENT;
                    if(currentItem.getShape() & Item::UP) shape |= Item::DOWN;
                    else if(currentItem.getShape() & Item::RIGHT) shape |= Item::LEFT;
                    else if(currentItem.getShape() & Item::DOWN) shape |= Item::UP;
                    else shape |= Item::RIGHT;
                    Item item{currentItem.getType(), shape, 0};
                    placePartial(currentCell, item);
                    break;
                }
                case id::set_value: {
                    Item item{valueDialog(currentItem, !(currentItem.getShape() & Item::DEPENDENT))};
                    if(item.getType() != Item::ItemType::none) {
                        grid.set(currentCell.y, currentCell.x, item);
                        dirty = true;
                        RefreshRect(affectedRect);
                    }
                    break;
                }
                case id::dependent: {
                    Item item{currentItem.getType(), currentItem.getShape() ^ Item::DEPENDENT, 0};
                    placePartial(currentCell, item);
                    break;
                }
                default: {}
            }
            break;
        }
    }
}

void WindowGrid::save(std::ofstream& ofstream) {
    ofstream.write("schematic", 10);
    int xScroll, yScroll;
    GetViewStart(&xScroll, &yScroll);
    uint32_t toWrite[] = {static_cast<uint32_t>(zoomLevels), static_cast<uint32_t>(xScroll), static_cast<uint32_t>(yScroll), static_cast<uint32_t>(dotSize), grid.getWidth(), grid.getHeight()};
    ofstream.write(reinterpret_cast<const char*>(toWrite), sizeof(toWrite));
    size_t numElements = grid.gridMap.size();
    ofstream.write(reinterpret_cast<const char*>(&numElements), sizeof(size_t));
    for(auto pair : grid.gridMap) {
        ofstream.write(reinterpret_cast<const char *>(&pair.first), sizeof(uint64_t));
        pair.second.save(ofstream);
    }
    dirty = false;
}

WindowGrid::LoadStruct WindowGrid::load(std::ifstream& ifstream) {
    char str[10];
    ifstream.read(str, 10);
    if(std::string{str} != "schematic") {
        throw std::runtime_error{"File invalid"};
    }
    uint32_t readArr[6];
    ifstream.read(reinterpret_cast<char *>(readArr), sizeof(readArr));
    size_t numElements;
    ifstream.read(reinterpret_cast<char *>(&numElements), sizeof(size_t));
    std::unordered_map<uint64_t, Item> gridMap{numElements};
    for (size_t i = 0; i < numElements; i++) {
        std::pair<uint64_t, Item> pair{};
        ifstream.read(reinterpret_cast<char *>(&pair.first), sizeof(uint64_t));
        pair.second = Item{ifstream};
        gridMap.insert(pair);
    }
    Grid grid{readArr[4], readArr[5], gridMap};
    return WindowGrid::LoadStruct{grid, static_cast<int>(readArr[0]), static_cast<int>(readArr[1]), static_cast<int>(readArr[2]), static_cast<int>(readArr[3])};
}

int WindowGrid::getDotSize() const {
    return dotSize;
}

void WindowGrid::setDotSize(int size) {
    dotSize = size;
    dirty = true;
    Refresh();
}

void WindowGrid::undo() {
    if(grid.undo()) {
        dirty = true;
        Refresh();
    }
}

void WindowGrid::redo() {
    if(grid.redo()) {
        dirty = true;
        Refresh();
    }
}

WindowGrid::LoadStruct::LoadStruct(Grid grid, int zoom, int xScroll, int yScroll, int dotSize) : grid{std::move(grid)}, zoom{zoom}, xScroll{xScroll}, yScroll{yScroll}, dotSize{dotSize} {}

namespace {
    Item valueDialog(const Item& currentItem, bool numeric) {
        wxString valueStr;
        if(currentItem.getExtraData().empty() && numeric) {
            std::stringstream ss{};
            ss << currentItem.getValue();
            valueStr = ss.str();
        } else {
            valueStr = currentItem.getExtraData();
        }
        wxTextEntryDialog dialog{nullptr, "Value:", "Set Value", valueStr};
        if (dialog.ShowModal() == wxID_OK) {
            if(numeric && !dialog.GetValue().IsEmpty()) {
                try {
                    double value = std::stod(dialog.GetValue().utf8_string());
                    return Item{currentItem.getType(), currentItem.getShape(), value};
                } catch (std::exception &e) {
                    return Item{currentItem.getType(), currentItem.getShape(), 0, dialog.GetValue().wc_str()};
                }
            } else {
                return Item{currentItem.getType(), currentItem.getShape(), 0, dialog.GetValue().wc_str()};
            }
        }
        return Item{};
    }
    int getDirection(const wxPoint& currentCell, const wxPoint& lastCell) {
        int shape = Item::DOWN;
        if (lastCell.x < currentCell.x) {
            shape = Item::RIGHT;
        } else if (lastCell.x > currentCell.x) {
            shape = Item::LEFT;
        } else if (lastCell.y > currentCell.y) {
            shape = Item::UP;
        }
        return shape;
    }
    int getOppositeDirection(int direction) {
        switch(direction) {
            case Item::UP: return Item::DOWN;
            case Item::RIGHT: return Item::LEFT;
            case Item::DOWN: return Item::UP;
            case Item::LEFT: return Item::RIGHT;
            default: throw std::invalid_argument("Bad direction"); //IDE says this is unreachable, but says not all code paths return value if I remove it
        }
    }
}