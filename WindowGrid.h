#pragma once
#include <wx/wx.h>
#include "Grid.h"

class WindowGrid : public wxScrolledCanvas {
public:
    explicit WindowGrid(Grid grid = Grid{},wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
    Item::ItemType selectedTool{Item::ItemType::wire};
private:
    void OnDraw(wxDC& dc) override;
    void onScroll(wxMouseEvent& event);
    void onMotion(wxMouseEvent& event);
    void onLeftDown(wxMouseEvent& event);
    void refresh(int xPos, int yPos);
    void placePartial(wxPoint cell, Item item);
    Grid grid;
    wxFont font;
    wxImage resistorScaled;
    wxPoint lastCell{-1,-1};
    wxPoint currentCell{-1,-1};
    wxPen pen;
    int zoomLevels = 0;
};