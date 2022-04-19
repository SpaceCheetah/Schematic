#pragma once
#include <wx/wx.h>
#include "Grid.h"

class WindowGrid : public wxScrolledCanvas {
public:
    explicit WindowGrid(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
private:
    void OnDraw(wxDC& dc) override;
    void onScroll(wxMouseEvent& event);
    void refresh(int xPos, int yPos);
    Grid grid;
    wxFont font;
    wxImage resistorScaled;
    int zoomLevels = 0;
};