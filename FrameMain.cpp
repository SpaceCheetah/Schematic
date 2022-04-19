#include "FrameMain.h"
#include "id.h"
#include "Resources.h"

FrameMain::FrameMain() : wxFrame(nullptr, wxID_ANY, "Schematic", wxDefaultPosition, wxDefaultSize,
                                 wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {
    wxTopLevelWindowMSW::SetIcons(resources::getResistorIconBundle());
    Bind(wxEVT_SIZE, &FrameMain::onSize, this);
    Bind(wxEVT_CHAR_HOOK, &FrameMain::onChar, this);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::wire;}, id::tool_wire);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::resistor;}, id::tool_resistor);

    windowGrid = nullptr; //toolbar->AddRadioTool sends a Size event, so need to clear windowGrid so that it doesn't try to set the size of an invalid pointer
    toolbar = wxFrame::CreateToolBar(wxTB_VERTICAL | wxTB_FLAT | wxTB_NODIVIDER, wxID_ANY);
    toolbar->AddRadioTool(id::tool_wire, "Wire", wxBitmap{resources::getWireImage()}, wxNullBitmap, "Wire");
    toolbar->AddRadioTool(id::tool_resistor, "Resistor", wxBitmap{resources::getResistorImage().Scale(32, 32)}, wxNullBitmap, "Resistor");
    toolbar->Realize();
    toolbar->Fit();

    windowGrid = new WindowGrid(Grid{}, this, wxID_ANY, wxDefaultPosition, GetClientSize());
    windowGrid->SetBackgroundColour(wxTheColourDatabase->Find("LIGHT GREY"));
}

void FrameMain::onSize(wxSizeEvent& evt) {
    if(windowGrid) {
        windowGrid->SetSize(GetClientSize());
    }
}

void FrameMain::onChar(wxKeyEvent& evt) {
    switch(evt.GetUnicodeKey()) {
        case '1':
            toolbar->ToggleTool(id::tool_wire, true);
            windowGrid->selectedTool = Item::ItemType::wire;
            break;
        case '2':
            toolbar->ToggleTool(id::tool_resistor, true);
            windowGrid->selectedTool = Item::ItemType::resistor;
            break;
    }
    evt.Skip();
}
