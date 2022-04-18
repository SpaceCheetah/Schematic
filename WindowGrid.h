#pragma once
#include <wx/wx.h>
#include "Grid.h"

class WindowGrid : public wxScrolledCanvas {
public:
    explicit WindowGrid(wxWindow* parent, wxWindowID id = wxID_ANY, const wxPoint& pos = wxDefaultPosition, const wxSize& size = wxDefaultSize);
private:
    void OnDraw(wxDC& dc) override;
    Grid grid;
    wxFont bigFont;
};