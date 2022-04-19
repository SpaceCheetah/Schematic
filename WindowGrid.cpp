#include "WindowGrid.h"
#include "Resources.h"
#include <wx/graphics.h>

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
                    switch (item.getRotation()) {
                        case Item::Rotation::up:
                        case Item::Rotation::down: {
                            wxSize textSize = dc.GetTextExtent(value);
                            dc.DrawBitmap(resistorScaled.Rotate90(), cellSize * c, cellSize * r);
                            dc.DrawRotatedText(value, cellSize * c + cellSize * 3 / 4, cellSize * r + cellSize / 2 - textSize.GetWidth() / 2, 270);
                            break;
                        }
                        case Item::Rotation::left:
                        case Item::Rotation::right: {
                            dc.DrawBitmap(resistorScaled, cellSize * c, cellSize * r);
                            dc.DrawLabel(value, wxRect{cellSize * c, cellSize * r + cellSize / 4, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                            break;
                        }
                    }
                    break;
                }
                case Item::ItemType::wire: {
                    break;
                }
            }
        }
    }
}

WindowGrid::WindowGrid(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size) : wxScrolledCanvas(parent, id, pos, size) {
    Bind(wxEVT_MOUSEWHEEL, &WindowGrid::onScroll, this);

    grid = Grid{};
    grid.set(0,0,Item{Item::ItemType::resistor, Item::Rotation::up, 1234908712345});
    grid.set(0,1,Item{Item::ItemType::resistor, Item::Rotation::left, 8734645325});
    grid.set(0,2, Item{Item::ItemType::resistor, Item::Rotation::right, 15435 });
    grid.set(1,0, Item{Item::ItemType::resistor, Item::Rotation::down, 76322});
    refresh(0, 0);
}

void WindowGrid::refresh(int xPos, int yPos) {
    wxScrolledCanvas::SetScrollbars(16, 16, static_cast<int>(grid.getWidth()) * (8 + zoomLevels), static_cast<int>(grid.getHeight()) * (8 + zoomLevels), xPos, yPos);
    font = wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT);
    font.SetPixelSize(wxSize{0, 16 + 2 * zoomLevels});
    resistorScaled = resources::getResistorImage().Scale(128 + 16 * zoomLevels, 128 + 16 * zoomLevels);
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