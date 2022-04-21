#include "WindowGrid.h"
#include "Resources.h"
#include "id.h"
#include <wx/graphics.h>
#include <utility>
#include <sstream>
#include <fstream>
#include <wx/propgrid/props.h>

//TODO: create backing bitmap for scrolling; render in separate thread (leaving standard rendering as backup), then when done switch to bliting
//perhaps even pre-render each zoom level? obviously using multiple threads... could be fun!

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
                    dc.DrawCircle(cellSize / 2 + cellSize * c, cellSize / 2 + cellSize * r, std::max(cellSize * 3 / 128, 1));
                    break;
                }
                case Item::ItemType::resistor: {
                    std::wstring value = item.getValueStr();
                    if (item.getShape() == Item::HORIZONTAL) {
                        dc.DrawBitmap(resistorBitmaps[zoomLevels + 7], cellSize * c, cellSize * r);
                        dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r + cellSize * 4 / 17, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                    } else {
                        dc.DrawBitmap(resistorBitmaps[zoomLevels + 35], cellSize * c, cellSize * r);
                        wxSize textSize = dc.GetTextExtent(value);
                        dc.DrawRotatedText(value, cellSize * c + cellSize * 40 / 51, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
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
            }
        }
    }
}

WindowGrid::WindowGrid(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const LoadStruct& load)
        : wxScrolledCanvas(parent, id, pos, size), grid{load.grid}, zoomLevels{load.zoom} {
    Bind(wxEVT_MOUSEWHEEL, &WindowGrid::onScroll, this);
    Bind(wxEVT_LEFT_DOWN, &WindowGrid::onLeftDown, this);
    Bind(wxEVT_MOTION, &WindowGrid::onMotion, this);
    Bind(wxEVT_RIGHT_DOWN, &WindowGrid::onRightDown, this);

    resistorMenu.Append(id::rotate, "Rotate");
    resistorMenu.Append(id::set_value, "Set resistance");
    wireMenu.Append(id::toggle_up, "Toggle up");
    wireMenu.Append(id::toggle_left, "Toggle left");
    wireMenu.Append(id::toggle_right, "Toggle right");
    wireMenu.Append(id::toggle_down, "Toggle down");

    for(int i = 0; i < 28; i ++) {
        resistorBitmaps[i] = resources::getResistorBitmap(16 + i * 16, false);
    }
    for(int i = 0; i < 28; i ++) {
        resistorBitmaps[i + 28] = resources::getResistorBitmap(16 + i * 16, true);
    }

    refresh(load.xScroll, load.yScroll);
}

void WindowGrid::refresh(int xPos, int yPos) {
    wxScrolledCanvas::SetScrollbars(16, 16, static_cast<int>(grid.getWidth()) * (8 + zoomLevels), static_cast<int>(grid.getHeight()) * (8 + zoomLevels), xPos, yPos);
    font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.SetPixelSize(wxSize{0, 16 + 2 * zoomLevels});
    pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(22.0 / 1024 * (128 + 16 * zoomLevels)))};
    Refresh();
}

void WindowGrid::reload(const WindowGrid::LoadStruct& load) {
    grid = load.grid;
    zoomLevels = load.zoom;
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
            zoomLevels += rows;
            if (zoomLevels > 20) {
                zoomLevels = 20;
            } else if (zoomLevels < -7) {
                zoomLevels = -7;
            }
            double mouseXLogical = (grid.getWidth() * (8 + zoomLevels)) * 16 * mouseXFraction;
            double mouseYLogical = (grid.getHeight() * (8 + zoomLevels)) * 16 * mouseYFraction;
            int newScrollX = static_cast<int>(std::max(mouseXLogical - mouseX, 0.0));
            int newScrollY = static_cast<int>(std::max(mouseYLogical - mouseY, 0.0));
            refresh(newScrollX / 16, newScrollY / 16);
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
                    int shape = Item::UP;
                    int oppositeShape = Item::DOWN;
                    if (lastCell.x < currentCell.x) {
                        shape = Item::LEFT;
                        oppositeShape = Item::RIGHT;
                    } else if (lastCell.x > currentCell.x) {
                        shape = Item::RIGHT;
                        oppositeShape = Item::LEFT;
                    } else if (lastCell.y > currentCell.y) {
                        shape = Item::DOWN;
                        oppositeShape = Item::UP;
                    }
                    placePartial(currentCell, Item{Item::ItemType::wire, shape, 0});
                    if(lastCell != wxPoint{-1,-1}) {
                        placePartial(lastCell, Item{Item::ItemType::wire, oppositeShape, 0});
                    }
                    break;
                }
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

void WindowGrid::placePartial(wxPoint cell, Item item) {
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
        case Item::ItemType::resistor: {
            if (currentItem.getType() == Item::ItemType::resistor) {
                if (currentItem.getShape() != item.getShape()) {
                    Item newItem{Item::ItemType::resistor, item.getShape(), currentItem.getValue()};
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
                int shape = Item::UP;
                if (lastCell.x < currentCell.x) {
                    shape = Item::LEFT;
                } else if (lastCell.x > currentCell.x) {
                    shape = Item::RIGHT;
                } else if (lastCell.y > currentCell.y) {
                    shape = Item::DOWN;
                }
                placePartial(currentCell, Item{Item::ItemType::wire, shape, 0});
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
        case Item::ItemType::none:
            event.Skip();
            break;
        case Item::ItemType::resistor: {
            int response = GetPopupMenuSelectionFromUser(resistorMenu);
            switch (response) {
                case id::rotate: {
                    Item item{Item::ItemType::resistor, currentItem.getShape() == Item::HORIZONTAL ? Item::VERTICAL : Item::HORIZONTAL, 0};
                    placePartial(currentCell, item);
                    break;
                }
                case id::set_value: {
                    //using stringstream instead of to_string because to_string with double outputs necessary 0s
                    std::stringstream ss{};
                    ss << currentItem.getValue();
                    wxTextEntryDialog dialog{this, "Resistance:", "Set Resistance", ss.str()};
                    dialog.SetTextValidator(wxNumericPropertyValidator{wxNumericPropertyValidator::Float});
                    if (dialog.ShowModal() == wxID_OK && !dialog.GetValue().IsEmpty()) {
                        try {
                            double value = std::stod(dialog.GetValue().utf8_string());
                            Item item{Item::ItemType::resistor, currentItem.getShape(), value};
                            grid.set(currentCell.y, currentCell.x, item);
                            dirty = true;
                            RefreshRect(affectedRect);
                        } catch (std::exception &e) { //unlikely, with the validator
                            wxMessageDialog(this, std::string("Cannot parse \"") + dialog.GetValue() + "\" as a double", "",
                                            wxOK | wxCENTRE | wxICON_WARNING).ShowModal();
                        }
                    }
                    break;
                }
                default: {
                }
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
    }
}

void WindowGrid::save(std::ofstream& ofstream) {
    //Just making sure serializing an Item to a file is allowed, and I don't accidentally break it
    static_assert(std::is_trivially_copyable<Item>());

    ofstream.write("schematic", 10);
    int xScroll, yScroll;
    GetViewStart(&xScroll, &yScroll);
    uint32_t toWrite[] = {static_cast<uint32_t>(zoomLevels), static_cast<uint32_t>(xScroll), static_cast<uint32_t>(yScroll), grid.getWidth(), grid.getHeight()};
    ofstream.write(reinterpret_cast<const char*>(toWrite), sizeof(toWrite));
    size_t numElements = grid.gridMap.size();
    ofstream.write(reinterpret_cast<const char*>(&numElements), sizeof(size_t));
    for(auto pair : grid.gridMap) {
        ofstream.write(reinterpret_cast<const char *>(&pair.first), sizeof(uint64_t));
        ofstream.write(reinterpret_cast<const char *>(&pair.second), sizeof(Item));
    }
    dirty = false;
}

WindowGrid::LoadStruct WindowGrid::load(std::ifstream& ifstream) {
    char str[10];
    ifstream.read(str, 10);
    if(std::string{str} != "schematic") {
        throw std::runtime_error{"File invalid"};
    }
    uint32_t readArr[5];
    ifstream.read(reinterpret_cast<char *>(readArr), sizeof(readArr));
    size_t numElements;
    ifstream.read(reinterpret_cast<char *>(&numElements), sizeof(size_t));
    std::unordered_map<uint64_t, Item> gridMap{numElements};
    for (size_t i = 0; i < numElements; i++) {
        std::pair<uint64_t, Item> pair{};
        ifstream.read(reinterpret_cast<char *>(&pair.first), sizeof(uint64_t));
        ifstream.read(reinterpret_cast<char *>(&pair.second), sizeof(Item));
        gridMap.insert(pair);
    }
    Grid grid{readArr[3], readArr[4], gridMap};
    return WindowGrid::LoadStruct{grid, static_cast<int>(readArr[0]), static_cast<int>(readArr[1]), static_cast<int>(readArr[2])};
}

WindowGrid::LoadStruct::LoadStruct(Grid grid, int zoom, int xScroll, int yScroll) : grid{std::move(grid)}, zoom{zoom}, xScroll{xScroll}, yScroll{yScroll} {}