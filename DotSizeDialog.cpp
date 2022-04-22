#include "DotSizeDialog.h"
#include "id.h"

DotSizeDialog::DotSizeDialog(wxWindow *parent, WindowGrid &grid) : wxDialog(parent, wxID_ANY, "Dot Size"), grid{grid} {
    Bind(wxEVT_SLIDER, &DotSizeDialog::onEvtSlider, this, id::dot_size_slider);
    slider = new wxSlider(this, id::dot_size_slider, grid.getDotSize(), -1, 20);
    auto* sizer = new wxBoxSizer{wxHORIZONTAL};
    sizer->Add(slider);
    sizer->Layout();
    SetSizerAndFit(sizer);
}

void DotSizeDialog::onEvtSlider(wxCommandEvent &evt) {
    grid.setDotSize(slider->GetValue());
}
