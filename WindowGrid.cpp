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
    Item valueDialog(const Item& currentItem);
    int getDirection(const wxPoint& currentCell, const wxPoint& lastCell);
    int flip(int direction);
    int rotateCW(int direction);
    int rotateCCW(int direction);
}

void WindowGrid::OnDraw(wxDC& dc) {
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetPen(pen);
    dc.SetFont(font);
    wxRect updateRect = GetUpdateRegion().GetBox();
    wxPoint tl = CalcUnscrolledPosition(wxPoint{updateRect.GetLeft(), updateRect.GetTop()});
    wxPoint br = CalcUnscrolledPosition(wxPoint{updateRect.GetRight(), updateRect.GetBottom()});
    int cellSize = 128 + 16 * zoomLevels;
    wxPoint origin = dc.GetDeviceOrigin();
    for (int r = (tl.y - cellSize + 1) / cellSize; r < std::min((br.y + cellSize - 1) / cellSize, static_cast<int>(grid.getHeight())); r++) {
        for (int c = (tl.x - cellSize + 1) / cellSize; c < std::min((br.x + cellSize - 1) / cellSize, static_cast<int>(grid.getWidth())); c++) {
            dc.SetDeviceOrigin(origin.x + cellSize * c, origin.y + cellSize * r);
            grid.get(r, c).draw(dc, cellSize, dotSize, rotatedText, resistorBitmaps, capacitorBitmaps, ampSourceBitmaps, voltSourceBitmaps);
        }
    }
}

WindowGrid::WindowGrid(wxWindow *parent, wxWindowID id, const wxPoint &pos, const wxSize &size, const LoadStruct& load)
        : wxScrolledCanvas(parent, id, pos, size), grid{load.grid}, zoomLevels{load.zoom}, dotSize{load.dotSize}, rotatedText{load.rotatedText} {
    Bind(wxEVT_MOUSEWHEEL, &WindowGrid::onScroll, this);
    Bind(wxEVT_LEFT_DOWN, &WindowGrid::onLeftDown, this);
    Bind(wxEVT_MOTION, &WindowGrid::onMotion, this);
    Bind(wxEVT_RIGHT_DOWN, &WindowGrid::onRightDown, this);

    twoWayMenu.Append(id::set_value, "Set value");
    twoWayMenu.Append(id::rotate, "Rotate");
    wireMenu.Append(id::set_value, "Set value");
    wireMenu.Append(id::toggle_up, "Toggle up");
    wireMenu.Append(id::toggle_left, "Toggle left");
    wireMenu.Append(id::toggle_right, "Toggle right");
    wireMenu.Append(id::toggle_down, "Toggle down");
    fourWayMenu.Append(id::set_value, "Set value");
    fourWayMenu.Append(id::rotate_cw, "Rotate CW");
    fourWayMenu.Append(id::rotate_ccw, "Rotate CCW");
    fourWayMenu.Append(id::flip, "Flip");
    fourWayMenu.Append(id::toggle_dependent, "Toggle dependent");

    refreshAll(load.xScroll, load.yScroll);
}

void WindowGrid::refreshAll(int xPos, int yPos) {
    if(xPos != -1 && yPos != -1) {
        wxScrolledCanvas::SetScrollbars(16, 16, static_cast<int>(grid.getWidth()) * (8 + zoomLevels), static_cast<int>(grid.getHeight()) * (8 + zoomLevels),xPos, yPos);
    }
    font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.SetPixelSize(wxSize{0, 16 + 2 * zoomLevels});
    pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(22.0 / 1024 * (128 + 16 * zoomLevels)))};
    int size = 128 + zoomLevels * 16;
    resistorBitmaps[0] = resources::getResistorBitmap(size, false);
    resistorBitmaps[1] = resources::getResistorBitmap(size, true);
    voltSourceBitmaps[0] = resources::getVoltSourceBitmap(size, Item::UP, false);
    voltSourceBitmaps[1] = resources::getVoltSourceBitmap(size, Item::DOWN, false);
    voltSourceBitmaps[2] = resources::getVoltSourceBitmap(size, Item::RIGHT, false);
    voltSourceBitmaps[3] = resources::getVoltSourceBitmap(size, Item::LEFT, false);
    voltSourceBitmaps[4] = resources::getVoltSourceBitmap(size, Item::UP | Item::DEPENDENT, false);
    voltSourceBitmaps[5] = resources::getVoltSourceBitmap(size, Item::DOWN | Item::DEPENDENT, false);
    voltSourceBitmaps[6] = resources::getVoltSourceBitmap(size, Item::RIGHT | Item::DEPENDENT, false);
    voltSourceBitmaps[7] = resources::getVoltSourceBitmap(size, Item::LEFT | Item::DEPENDENT, false);
    ampSourceBitmaps[0] = resources::getAmpSourceBitmap(size, Item::UP, false);
    ampSourceBitmaps[1] = resources::getAmpSourceBitmap(size, Item::DOWN, false);
    ampSourceBitmaps[2] = resources::getAmpSourceBitmap(size, Item::RIGHT, false);
    ampSourceBitmaps[3] = resources::getAmpSourceBitmap(size, Item::LEFT, false);
    ampSourceBitmaps[4] = resources::getAmpSourceBitmap(size, Item::UP | Item::DEPENDENT, false);
    ampSourceBitmaps[5] = resources::getAmpSourceBitmap(size, Item::DOWN | Item::DEPENDENT, false);
    ampSourceBitmaps[6] = resources::getAmpSourceBitmap(size, Item::RIGHT | Item::DEPENDENT, false);
    ampSourceBitmaps[7] = resources::getAmpSourceBitmap(size, Item::LEFT | Item::DEPENDENT, false);
    capacitorBitmaps[0] = resources::getCapacitorBitmap(size, false);
    capacitorBitmaps[1] = resources::getCapacitorBitmap(size, true);
    Refresh();
}

void WindowGrid::reload(const WindowGrid::LoadStruct& load) {
    grid = load.grid;
    zoomLevels = load.zoom;
    dotSize = load.dotSize;
    rotatedText = load.rotatedText;
    dirty = false;
    refreshAll(load.xScroll, load.yScroll);
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
                refreshAll(newScrollX / 16, newScrollY / 16);
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
                        placePartial(lastCell, Item{Item::ItemType::wire, flip(shape), 0});
                    }
                    break;
                }
                case Item::ItemType::volt_source: case Item::ItemType::amp_source:
                    placePartial(currentCell, Item{selectedTool, getDirection(currentCell, lastCell), 10});
                    break;
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
    switch (item.type) {
        case Item::ItemType::none: {
            if (currentItem.type != Item::ItemType::none) {
                grid.set(cell.y, cell.x, item);
                dirty = true;
                RefreshRect(affectedRect);
            }
            break;
        }
        case Item::ItemType::wire: {
            if (currentItem.type == Item::ItemType::none) {
                grid.set(cell.y, cell.x, item);
                dirty = true;
                RefreshRect(affectedRect);
            } else if (currentItem.type == Item::ItemType::wire && !(currentItem.shape & item.shape)) {
                currentItem.shape |= item.shape;
                grid.set(cell.y, cell.x, currentItem);
                dirty = true;
                RefreshRect(affectedRect);
            }
            break;
        }
        case Item::ItemType::resistor: case Item::ItemType::volt_source: case Item::ItemType::amp_source: case Item::ItemType::capacitor: {
            if (currentItem.type == item.type) {
                if (currentItem.shape != item.shape) {
                    currentItem.shape = item.shape;
                    grid.set(cell.y, cell.x, currentItem);
                    dirty = true;
                    RefreshRect(affectedRect);
                }
            } else if (currentItem.type == Item::ItemType::none || currentItem.type == Item::ItemType::wire) {
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
            case Item::ItemType::wire:
                placePartial(currentCell, Item{Item::ItemType::wire, getDirection(lastCell, currentCell), 0});
                break;
            case Item::ItemType::volt_source: case Item::ItemType::amp_source:
                placePartial(currentCell, Item{selectedTool, getDirection(currentCell, lastCell), 10});
                break;
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
    wxMenu* menu;
    switch(currentItem.type) {
        case Item::ItemType::none:
            return;
        case Item::ItemType::resistor: case Item::ItemType::capacitor:
            menu = &twoWayMenu;
            break;
        case Item::ItemType::wire:
            menu = &wireMenu;
            break;
        case Item::ItemType::volt_source: case Item::ItemType::amp_source:
            menu = &fourWayMenu;
            break;
    }
    int shape = currentItem.shape;
    Item directItem{};
    switch(GetPopupMenuSelectionFromUser(*menu)) {
        case id::rotate: 
            shape = shape == Item::HORIZONTAL ? Item::VERTICAL : Item::HORIZONTAL;
            break;
        case id::set_value: 
            directItem = valueDialog(currentItem);
            break;
        case id::toggle_up:
            shape ^= Item::UP;
            break;
        case id::toggle_right:
            shape ^= Item::RIGHT;
            break;
        case id::toggle_down:
            shape ^= Item::DOWN;
            break;
        case id::toggle_left:
            shape ^= Item::LEFT;
            break;
        case id::toggle_dependent:
            shape ^= Item::DEPENDENT;
            break;
        case id::rotate_cw:
            shape = (shape & Item::DEPENDENT) | rotateCW(shape & (Item::UP | Item::RIGHT | Item::DOWN | Item::LEFT));
            break;
        case id::rotate_ccw:
            shape = (shape & Item::DEPENDENT) | rotateCCW(shape & (Item::UP | Item::RIGHT | Item::DOWN | Item::LEFT));
            break;
        case id::flip:
            shape = (shape & Item::DEPENDENT) | flip(shape & (Item::UP | Item::RIGHT | Item::DOWN | Item::LEFT));
            break;
    }
    if(shape != currentItem.shape) {
        directItem = currentItem;
        directItem.shape = shape;
    }
    if(directItem.type != Item::ItemType::none) {
        grid.set(currentCell.y, currentCell.x, directItem);
        dirty = true;
        RefreshRect(affectedRect);
    }
}

void WindowGrid::save(std::ofstream& ofstream) {
    ofstream.write("schematic", 10);
    int xScroll, yScroll;
    GetViewStart(&xScroll, &yScroll);
    uint32_t toWrite[] = {static_cast<uint32_t>(zoomLevels), static_cast<uint32_t>(xScroll), static_cast<uint32_t>(yScroll), static_cast<uint32_t>(dotSize), grid.getWidth(), grid.getHeight()};
    ofstream.write(reinterpret_cast<const char*>(toWrite), sizeof(toWrite));
    uint8_t boolOptions = 0; //doing it this way to make it easier to add more bool options in the future without breaking the file format again
    if(rotatedText) boolOptions |= 1;
    ofstream.write(reinterpret_cast<const char *>(&boolOptions), sizeof(uint8_t));
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
    uint8_t boolOptions;
    ifstream.read(reinterpret_cast<char *>(&boolOptions), sizeof(uint8_t));
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
    return WindowGrid::LoadStruct{grid, static_cast<int>(readArr[0]), static_cast<int>(readArr[1]), static_cast<int>(readArr[2]), static_cast<int>(readArr[3]), (boolOptions & 1) == 1};
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

void WindowGrid::toggleRotatedText() {
    rotatedText = !rotatedText;
    dirty = true;
    refreshAll();
}

WindowGrid::LoadStruct::LoadStruct(Grid grid, int zoom, int xScroll, int yScroll, int dotSize, bool rotatedText) : grid{std::move(grid)}, zoom{zoom}, xScroll{xScroll}, yScroll{yScroll}, dotSize{dotSize}, rotatedText{rotatedText} {}

namespace {
    Item valueDialog(const Item& currentItem) {
        bool numeric = !((currentItem.shape & Item::DEPENDENT) || currentItem.type == Item::ItemType::wire);
        wxString valueStr;
        if(currentItem.extraData.empty() && numeric) {
            std::stringstream ss{};
            ss << currentItem.value;
            valueStr = ss.str();
        } else {
            valueStr = currentItem.extraData;
        }
        wxTextEntryDialog dialog{nullptr, "Value:", "Set Value", valueStr};
        if (dialog.ShowModal() == wxID_OK) {
            if(numeric && !dialog.GetValue().IsEmpty()) {
                try {
                    double value = std::stod(dialog.GetValue().utf8_string());
                    return Item{currentItem.type, currentItem.shape, value};
                } catch (std::exception &e) {
                    return Item{currentItem.type, currentItem.shape, 0, dialog.GetValue().wc_str()};
                }
            } else {
                return Item{currentItem.type, currentItem.shape, 0, dialog.GetValue().wc_str()};
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
    int flip(int direction) {
        switch(direction) {
            case Item::UP: return Item::DOWN;
            case Item::RIGHT: return Item::LEFT;
            case Item::DOWN: return Item::UP;
            case Item::LEFT: return Item::RIGHT;
            default: throw std::invalid_argument("Bad direction");
        }
    }
    int rotateCW(int direction) {
        switch(direction) {
            case Item::UP: return Item::RIGHT;
            case Item::RIGHT: return Item::DOWN;
            case Item::DOWN: return Item::LEFT;
            case Item::LEFT: return Item::UP;
            default: throw std::invalid_argument("Bad direction");
        }
    }
    int rotateCCW(int direction) {
        switch(direction) {
            case Item::UP: return Item::LEFT;
            case Item::RIGHT: return Item::UP;
            case Item::DOWN: return Item::RIGHT;
            case Item::LEFT: return Item::DOWN;
            default: throw std::invalid_argument("Bad direction");
        }
    }
}