#pragma once
#include <wx/wx.h>
#include "WindowGrid.h"

class FrameMain : public wxFrame {
public:
    FrameMain();
private:
    void onSize(wxSizeEvent& evt);
    void onChar(wxKeyEvent& evt);
    wxToolBar* toolbar;
    WindowGrid* windowGrid;
};