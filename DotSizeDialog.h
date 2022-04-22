#pragma once
#include <wx/wx.h>
#include "WindowGrid.h"

class DotSizeDialog : public wxDialog {
public:
    DotSizeDialog(wxWindow* parent, WindowGrid& grid);
private:
    void onEvtSlider(wxCommandEvent& evt);
    WindowGrid& grid;
    wxSlider* slider;
};
