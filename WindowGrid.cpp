#include "WindowGrid.h"
#include "Resources.h"
#include <wx/graphics.h>

void WindowGrid::OnDraw(wxDC& dc) {
    dc.SetBrush(*wxBLACK_BRUSH);
    wxRect updateRect = GetUpdateRegion().GetBox();
    wxPoint tl = CalcUnscrolledPosition(wxPoint{updateRect.GetLeft(), updateRect.GetTop()});
    wxPoint br = CalcUnscrolledPosition(wxPoint{updateRect.GetRight(), updateRect.GetBottom()});
    for(int r = (tl.y - 127) / 128; r < (br.y + 127) / 128; r ++) {
        for(int c = (tl.x - 127) / 128; c < (br.x + 127) / 128; c ++) {
            Item item = grid[r][c];
            switch(item.getType()) {
                case Item::ItemType::none: {
                    dc.DrawCircle(64 + 128 * c, 64 + 128 * r, 8);
                    break;
                }
                case Item::ItemType::resistor: {
                    std::wstring value = item.getValueStr();
                    wxBitmap bitmap = wxBitmap{resources::getResistorImage()};
                    wxMemoryDC memDC{bitmap};
                    //need a GraphicsContext because MemoryDC doesn't properly support alpha
                    wxGraphicsContext* gc = wxGraphicsContext::Create(memDC);
                    gc->SetFont(bigFont, *wxBLACK);
                    double width;
                    double height;//throwing away height, but do need to pass it a valid double location to write to
                    gc->GetTextExtent(value, &width, &height);
                    gc->DrawText(item.getValueStr(), 512 - width / 2, 300);
                    delete gc;
                    memDC.SelectObject(wxNullBitmap);
                    wxImage image = bitmap.ConvertToImage().Scale(128, 128);
                    switch (item.getRotation()) {
                        case Item::Rotation::up:
                        case Item::Rotation::down: {
                            dc.DrawBitmap(image.Rotate90(), 128 * c, 128 * r);
                            break;
                        }
                        case Item::Rotation::left:
                        case Item::Rotation::right: {
                            dc.DrawBitmap(image, 128 * c, 128 * r);
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
    grid = Grid{};
    grid[0][0] = Item{Item::ItemType::resistor, Item::Rotation::up, 1234908712345};
    grid[0][1] = Item{Item::ItemType::resistor, Item::Rotation::left, 8734645325};
    grid[0][2] = Item{Item::ItemType::resistor, Item::Rotation::right, 15432};
    grid[1][0] = Item{Item::ItemType::resistor, Item::Rotation::down, 76322};
    wxScrolledCanvas::SetScrollbars(16, 16, grid.getWidth() * 8, grid.getHeight() * 8);
    bigFont = wxSystemSettings::GetFont(wxSYS_SYSTEM_FONT);
    bigFont.SetPointSize(80);
}
