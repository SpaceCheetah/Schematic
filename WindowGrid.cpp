#include "WindowGrid.h"
#include "Resources.h"
#include <wx/graphics.h>

#include <utility>

void WindowGrid::OnDraw(wxDC& dc) {
    dc.SetBrush(*wxBLACK_BRUSH);
    dc.SetFont(font);
    wxRect updateRect = GetUpdateRegion().GetBox();
    wxPoint tl = CalcUnscrolledPosition(wxPoint{updateRect.GetLeft(), updateRect.GetTop()});
    wxPoint br = CalcUnscrolledPosition(wxPoint{updateRect.GetRight(), updateRect.GetBottom()});
    int cellSize = 128 + 16 * zoomLevels;
    for(int r = (tl.y - cellSize + 1) / cellSize; r < std::min((br.y + cellSize - 1) / cellSize, static_cast<int>(grid.getHeight())); r ++) {
        for(int c = (tl.x - cellSize + 1) / cellSize; c < std::min((br.x + cellSize - 1) / cellSize, static_cast<int>(grid.getWidth())); c ++) {
            Item item = grid.get(r, c);
            switch(item.getType()) {
                case Item::ItemType::none: {
                    dc.DrawCircle(cellSize / 2 + cellSize * c, cellSize / 2 + cellSize * r, std::max(cellSize * 3 / 128, 1));
                    break;
                }
                case Item::ItemType::resistor: {
                    std::wstring value = item.getValueStr();
                    if(item.getShape() == Item::HORIZONTAL) {
                        dc.DrawBitmap(resistorScaled, cellSize * c, cellSize * r);
                        dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r + cellSize / 4, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                    } else {
                        wxSize textSize = dc.GetTextExtent(value);
                        dc.DrawBitmap(resistorScaled.Rotate90(), cellSize * c, cellSize * r);
                        dc.DrawRotatedText(value, cellSize * c + cellSize * 3 / 4, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
                    }
                    break;
                }
                case Item::ItemType::wire: {
                    dc.SetPen(pen);
                    int directions = 0;
                    wxPoint middle = wxPoint{cellSize / 2 + cellSize * c, cellSize / 2 + cellSize * r};
                    if(item.getShape() & Item::UP) {
                        dc.DrawLine(wxPoint{cellSize / 2 + cellSize * c, cellSize * r}, middle);
                        directions ++;
                    }
                    if(item.getShape() & Item::DOWN) {
                        dc.DrawLine(wxPoint{cellSize / 2 + cellSize * c, cellSize * (r + 1)}, middle);
                        directions ++;
                    }
                    if(item.getShape() & Item::LEFT) {
                        dc.DrawLine(wxPoint{cellSize * c, cellSize / 2 + cellSize * r}, middle);
                        directions ++;
                    }
                    if(item.getShape() & Item::RIGHT) {
                        dc.DrawLine(wxPoint{cellSize * (c + 1), cellSize / 2 + cellSize * r}, middle);
                        directions ++;
                    }
                    if(directions > 2) {
                        dc.DrawCircle(middle, std::max(cellSize * 3 / 128, 1));
                    }
                    break;
                }
            }
        }
    }
}

WindowGrid::WindowGrid(Grid grid, wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) : wxScrolledCanvas(parent, id, pos, size), grid{std::move(grid)} {
    Bind(wxEVT_MOUSEWHEEL, &WindowGrid::onScroll, this);
    Bind(wxEVT_LEFT_DOWN, &WindowGrid::onLeftDown, this);
    Bind(wxEVT_MOTION, &WindowGrid::onMotion, this);
    refresh(0, 0);
}

void WindowGrid::refresh(int xPos, int yPos) {
    wxScrolledCanvas::SetScrollbars(16, 16, static_cast<int>(grid.getWidth()) * (8 + zoomLevels), static_cast<int>(grid.getHeight()) * (8 + zoomLevels), xPos, yPos);
    font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.SetPixelSize(wxSize{0, 16 + 2 * zoomLevels});
    resistorScaled = resources::getResistorImage().Scale(128 + 16 * zoomLevels, 128 + 16 * zoomLevels);
    pen = wxPen{wxPenInfo(*wxBLACK, std::ceil(22.0 / 1024 * (128 + 16 * zoomLevels)))};
    Refresh();
}


void WindowGrid::onScroll(wxMouseEvent& event) {
    if(event.GetModifiers() == wxMOD_CONTROL) {
        static int rotation = 0;
        rotation += event.GetWheelRotation();
        int rows = rotation / event.GetWheelDelta();
        if(rows != 0) {
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
            if(zoomLevels > 20) {
                zoomLevels = 20;
            } else if(zoomLevels < -7) {
                zoomLevels = -7;
            }
            double mouseXLogical = (grid.getWidth() * (8 + zoomLevels)) * 16 * mouseXFraction;
            double mouseYLogical = (grid.getHeight() * (8 + zoomLevels)) * 16 * mouseYFraction;
            int newScrollX = static_cast<int>(std::max(mouseXLogical - mouseX, 0.0));
            int newScrollY = static_cast<int>(std::max(mouseYLogical - mouseY, 0.0));
            refresh(newScrollX / 16, newScrollY / 16);
            OutputDebugStringA(std::to_string(zoomLevels).c_str());
            OutputDebugStringA("\n");
        }
    } else {
        event.Skip();
    }
}

void WindowGrid::onMotion(wxMouseEvent& event) {
    static wxPoint lastMousePos{-1,-1};
    static wxPoint lastScrolledMousePos{0,0};
    wxPoint mousePos = event.GetPosition();
    int cellSize = 128 + 16 * zoomLevels;
    wxPoint logicalPos = CalcUnscrolledPosition(mousePos);
    wxPoint cell = logicalPos / cellSize;
    if(!(cell.x >= 0 && cell.y >= 0 && cell.x < grid.getWidth() && cell.y < grid.getHeight())) {
        cell = wxPoint{-1,-1};
    }
    if(cell != currentCell) {
        lastCell = currentCell;
        currentCell = cell;
        if(event.LeftIsDown()) {
            if(selectedTool == Item::ItemType::resistor) {
                int shape = Item::VERTICAL;
                if (lastCell.x != currentCell.x) {
                    shape = Item::HORIZONTAL;
                }
                placePartial(currentCell, Item{Item::ItemType::resistor, shape, 100});
            } else if(selectedTool == Item::ItemType::wire) {
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
                placePartial(lastCell, Item{Item::ItemType::wire, oppositeShape, 0});
            }
        }
    }
    if(event.MiddleIsDown()) {
        wxPoint diff = lastScrolledMousePos - mousePos;
        wxPoint scrollCurrent = GetViewStart();
        wxPoint scrollChange{0,0};
        if(diff.x >= 16 || diff.x <= -16) {
            scrollChange.x = diff.x / 16;
            lastScrolledMousePos.x = mousePos.x;
        }
        if(diff.y >= 16 || diff.y <= -16) {
            scrollChange.y = diff.y / 16;
            lastScrolledMousePos.y = mousePos.y;
        }
        if(scrollChange != wxPoint{0,0}) {
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
    wxRect affectedRect{CalcScrolledPosition(cellSize * cell), wxSize{cellSize, cellSize}};\
    Item currentItem = grid.get(cell.y, cell.x);
    if(item.getType() == Item::ItemType::resistor) {
        if(currentItem.getType() == Item::ItemType::resistor) {
            if(currentItem.getShape() != item.getShape()) {
                Item newItem{Item::ItemType::resistor, item.getShape(), currentItem.getValue()};
                grid.set(cell.y, cell.x, newItem);
                RefreshRect(affectedRect);
            }
        } else if (currentItem.getType() == Item::ItemType::none || currentItem.getType() == Item::ItemType::wire) {
            grid.set(cell.y, cell.x, item);
            RefreshRect(affectedRect);
        }
    } else if(item.getType() == Item::ItemType::wire) {
        if(currentItem.getType() == Item::ItemType::none) {
            grid.set(cell.y, cell.x, item);
            RefreshRect(affectedRect);
        } else if(currentItem.getType() == Item::ItemType::wire && !(currentItem.getShape() & item.getShape())) {
            Item newItem{Item::ItemType::wire, item.getShape() | currentItem.getShape(), 0};
            grid.set(cell.y, cell.x, newItem);
            RefreshRect(affectedRect);
        }
    }
}

void WindowGrid::onLeftDown(wxMouseEvent& event) {
    if (currentCell != wxPoint{-1, -1}) {
        if (selectedTool == Item::ItemType::resistor) {
            int shape = Item::VERTICAL;
            if (lastCell.x != currentCell.x) {
                shape = Item::HORIZONTAL;
            }
            placePartial(currentCell, Item{Item::ItemType::resistor, shape, 100});
        } else if (selectedTool == Item::ItemType::wire) {
            int shape = Item::UP;
            if (lastCell.x < currentCell.x) {
                shape = Item::LEFT;
            } else if (lastCell.x > currentCell.x) {
                shape = Item::RIGHT;
            } else if (lastCell.y > currentCell.y) {
                shape = Item::DOWN;
            }
            placePartial(currentCell, Item{Item::ItemType::wire, shape, 0});
        }
    }
    event.Skip();
}
