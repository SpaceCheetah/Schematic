#include "FrameMain.h"
#include "id.h"
#include "Resources.h"
#include "NewSchematicDialog.h"
#include <fstream>

FrameMain::FrameMain() : wxFrame(nullptr, wxID_ANY, "Schematic", wxDefaultPosition, wxDefaultSize,wxDEFAULT_FRAME_STYLE | wxMAXIMIZE) {
    wxTopLevelWindowMSW::SetIcons(resources::getResistorIconBundle());
    Bind(wxEVT_SIZE, &FrameMain::onSize, this);
    Bind(wxEVT_CHAR_HOOK, &FrameMain::onChar, this);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::wire;}, id::tool_wire);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::resistor;}, id::tool_resistor);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::none;}, id::tool_bin);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onSave(false);}, id::file_save);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onSave(true);}, id::file_save_as);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onLoad();}, id::file_load);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onNew();}, id::file_new);
    Bind(wxEVT_CLOSE_WINDOW, &FrameMain::onClose, this);

    windowGrid = nullptr; //toolbar->AddRadioTool sends a Size event, so need to clear windowGrid so that it doesn't try to set the size of an invalid pointer
    toolbar = wxFrame::CreateToolBar(wxTB_VERTICAL | wxTB_FLAT | wxTB_NODIVIDER, wxID_ANY);
    toolbar->AddRadioTool(id::tool_wire, "Wire", wxBitmap{resources::getWireImage()}, wxNullBitmap, "Wire");
    toolbar->AddRadioTool(id::tool_resistor, "Resistor", wxBitmap{resources::getResistorImage().Scale(32, 32)}, wxNullBitmap, "Resistor");
    toolbar->AddRadioTool(id::tool_bin, "Delete", wxBitmap{resources::getBinImage()}, wxNullBitmap, "Delete");
    toolbar->Realize();
    toolbar->Fit();

    fileMenu = new wxMenu();
    fileMenu->Append(id::file_save, "Save (CTRL+S)");
    fileMenu->Append(id::file_save_as, "Save As");
    fileMenu->Append(id::file_load, "Load (CTRL+L)");
    fileMenu->Append(id::file_new, "New (CTRL+N)");
    menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "File");
    wxFrame::SetMenuBar(menuBar);

    windowGrid = new WindowGrid(this, wxID_ANY, wxDefaultPosition, GetClientSize());
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
        case '3':
            toolbar->ToggleTool(id::tool_bin, true);
            windowGrid->selectedTool = Item::ItemType::none;
            break;
        case 'S':
            if(evt.GetModifiers() == wxMOD_CONTROL) {
                onSave(false);
            }
            break;
        case 'N':
            if(evt.GetModifiers() == wxMOD_CONTROL) {
                onNew();
            }
            break;
        case 'L':
            if(evt.GetModifiers() == wxMOD_CONTROL) {
                onLoad();
            }
            break;
    }
    evt.Skip();
}

void FrameMain::onSave(bool saveAs) {
    if(saveAs || file == std::filesystem::path{}) {
        wxFileDialog dialog{this, "Save Schematic", "", "", "Schematic files (*.schematic)|*.schematic", wxFD_SAVE | wxFD_OVERWRITE_PROMPT};
        if(dialog.ShowModal() == wxID_OK) {
            file = std::filesystem::path{std::wstring_view{dialog.GetPath().wc_str()}};
            onSave(false);
        }
    } else {
        std::ofstream ofstream{file, std::ios_base::binary};
        if(ofstream.fail()) {
            wxMessageDialog{this, "Could not save", "Error", wxOK | wxICON_ERROR}.ShowModal();
        } else {
            windowGrid->save(ofstream);
        }
    }
}

void FrameMain::onLoad() {
    if(!windowGrid->dirty || confirmClose("Are you sure you want to load a new file?")) {
        wxFileDialog dialog{this, "Load Schematic", "", "", "Schematic files (*.schematic)|*.schematic", wxFD_OPEN | wxFD_FILE_MUST_EXIST};
        if(dialog.ShowModal() == wxID_OK) {
            std::ifstream ifstream{std::filesystem::path{std::wstring_view{dialog.GetPath().wc_str()}}, std::ios_base::binary};
            try {
                WindowGrid::LoadStruct load = WindowGrid::load(ifstream);
                windowGrid->reload(load);
            } catch(std::runtime_error& e) {
                wxMessageDialog(this, "Invalid File", "",wxOK | wxCENTRE | wxICON_WARNING).ShowModal();
            }
        }
    }

}

void FrameMain::onNew() {
    if(!windowGrid->dirty || confirmClose("Are you sure you want to create a new file?")) {
        NewSchematicDialog dialog{this};
        if(dialog.ShowModal() == wxID_OK) {
            wxSize size = dialog.getValue();
            Grid grid{static_cast<uint32_t>(size.x), static_cast<uint32_t>(size.y)};
            windowGrid->reload(WindowGrid::LoadStruct{grid});
        }
    }
}

bool FrameMain::confirmClose(const wxString& message) {
    wxMessageDialog dialog{this, message, "Unsaved work", wxYES_NO | wxICON_WARNING};
    return dialog.ShowModal() == wxID_YES;
}

void FrameMain::onClose(wxCloseEvent& evt) {
    if(evt.CanVeto() && windowGrid->dirty && !confirmClose("Are you sure you want to exit?")) {
        evt.Veto();
        return;
    }
    Destroy();
}
