#include "FrameMain.h"
#include "id.h"
#include "Resources.h"

FrameMain::FrameMain() : wxFrame(nullptr, wxID_ANY, "Schematic", wxDefaultPosition, wxDefaultSize,
                                 wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {
    wxTopLevelWindowMSW::SetIcons(resources::getResistorIconBundle());
    Bind(wxEVT_SIZE, &FrameMain::onSize, this);


    toolbar = wxFrame::CreateToolBar(wxTB_VERTICAL | wxTB_FLAT | wxTB_NODIVIDER, wxID_ANY);
    toolbar->AddTool(id::tool_resistor, "Resistor", wxBitmap{resources::getResistorImage().Scale(32, 32)});
    toolbar->Realize();
    toolbar->Fit();

    windowGrid = new WindowGrid(this, wxID_ANY, wxDefaultPosition, GetClientSize());
    windowGrid->SetBackgroundColour(wxTheColourDatabase->Find("LIGHT GREY"));
}

void FrameMain::onSize(wxSizeEvent &evt) {
    windowGrid->SetSize(GetClientSize());
}
