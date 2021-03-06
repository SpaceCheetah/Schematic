#pragma once
#include <wx/wx.h>
#include "Grid.h"

class WindowGrid : public wxScrolledCanvas {
public:
    struct LoadStruct {
        Grid grid;
        int zoom, xScroll, yScroll, dotSize;
        bool rotatedText;
        bool shadedBackground;
        explicit LoadStruct(Grid  grid = Grid{}, int zoom = 0, int xScroll = 0, int yScroll = 0, int dotSize = 3, bool rotatedText = false, bool shadedBackground = true);
    };
    explicit WindowGrid(wxWindow* parent = nullptr, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize, const LoadStruct& load = LoadStruct{});
    Item::ItemType selectedTool{Item::ItemType::wire};
    int zoomLevels = 0;
    bool dirty = false;
    void save(std::ofstream& ofstream);
    void reload(const LoadStruct& load);
    static LoadStruct load(std::ifstream& ifstream);
    int getDotSize() const;
    void setDotSize(int size);
    void toggleRotatedText();
    void toggleShadedBackground();
    void undo();
    void redo();
private:
    void OnDraw(wxDC& dc) override;
    void onScroll(wxMouseEvent& event);
    void onMotion(wxMouseEvent& event);
    void onLeftDown(wxMouseEvent& event);
    void onRightDown(wxMouseEvent& event);
    void refreshAll(int xPos = -1, int yPos = -1);
    void placePartial(wxPoint cell, const Item& item);
    Grid grid;
    wxFont font;
    wxPoint lastCell{-1,-1};
    wxPoint currentCell{-1,-1};
    wxPen pen;
    wxMenu twoWayMenu{};
    wxMenu wireMenu{};
    wxMenu fourWayMenu{};
    wxMenu switchMenu{};
    wxBitmap resistorBitmaps[2];
    wxBitmap voltSourceBitmaps[8];
    wxBitmap ampSourceBitmaps[8];
    wxBitmap capacitorBitmaps[2];
    wxBitmap switchBitmaps[4];
    int dotSize;
    bool rotatedText;
    bool shadedBackground;
};