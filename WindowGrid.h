#pragma once
#include <wx/wx.h>
#include "Grid.h"

class WindowGrid : public wxScrolledCanvas {
public:
    struct LoadStruct {
        Grid grid;
        int zoom, xScroll, yScroll;
        explicit LoadStruct(Grid  grid = Grid{}, int zoom = 0, int xScroll = 0, int yScroll = 0);
    };
    explicit WindowGrid(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, const LoadStruct& load = LoadStruct{});
    Item::ItemType selectedTool{Item::ItemType::wire};
    int zoomLevels = 0;
    bool dirty = false;
    void save(std::ofstream& ofstream);
    void reload(const LoadStruct& load);
    static LoadStruct load(std::ifstream& ifstream);
private:
    void OnDraw(wxDC& dc) override;
    void onScroll(wxMouseEvent& event);
    void onMotion(wxMouseEvent& event);
    void onLeftDown(wxMouseEvent& event);
    void onRightDown(wxMouseEvent& event);
    void refresh(int xPos, int yPos);
    void placePartial(wxPoint cell, Item item);
    Grid grid;
    wxFont font;
    wxImage resistorScaled;
    wxPoint lastCell{-1,-1};
    wxPoint currentCell{-1,-1};
    wxPen pen;
    wxMenu resistorMenu{};
    wxMenu wireMenu{};
};