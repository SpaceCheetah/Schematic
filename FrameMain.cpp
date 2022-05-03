#include "FrameMain.h"
#include "id.h"
#include "Resources.h"
#include "NewSchematicDialog.h"
#include "DotSizeDialog.h"
#include <fstream>

FrameMain::FrameMain(const std::wstring& fileIn) : wxFrame(nullptr, wxID_ANY, "Schematic", wxDefaultPosition, wxDefaultSize,wxDEFAULT_FRAME_STYLE) {
    this->Maximize();
    windowGrid = nullptr; //toolbar->AddRadioTool sends a Size event, so need to clear windowGrid so that it doesn't try to set the size of an invalid pointer

    Bind(wxEVT_SIZE, &FrameMain::onSize, this);
    Bind(wxEVT_CHAR_HOOK, &FrameMain::onChar, this);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::wire;}, id::tool_wire);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::resistor;}, id::tool_resistor);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::volt_source;}, id::tool_volt_source);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::amp_source;}, id::tool_amp_source);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::capacitor;}, id::tool_capacitor);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::toggle;}, id::tool_switch);
    Bind(wxEVT_TOOL, [this](wxCommandEvent& evt) {windowGrid->selectedTool = Item::ItemType::none;}, id::tool_bin);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onSave(false);}, id::file_save);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onSave(true);}, id::file_save_as);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onLoad();}, id::file_load);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {onNew();}, id::file_new);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {(new DotSizeDialog{this, *windowGrid})->Show();}, id::view_dot_size);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {windowGrid->toggleRotatedText();}, id::view_rotated_text);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {windowGrid->undo();}, wxID_UNDO);
    Bind(wxEVT_MENU, [this](wxCommandEvent& evt) {windowGrid->redo();}, wxID_REDO);
    Bind(wxEVT_CLOSE_WINDOW, &FrameMain::onClose, this);

    wxIconBundle bundle = resources::getResistorIconBundle();
    wxTopLevelWindowMSW::SetIcons(bundle);

    int dip32 = FromDIP(32);
    toolbar = wxFrame::CreateToolBar(wxTB_VERTICAL | wxTB_FLAT | wxTB_NODIVIDER, wxID_ANY);
    toolbar->AddRadioTool(id::tool_wire, "Wire", resources::getWireBitmap(dip32), wxNullBitmap, "Wire");
    toolbar->AddRadioTool(id::tool_resistor, "Resistor", resources::getResistorBitmap(dip32, false), wxNullBitmap, "Resistor");
    toolbar->AddRadioTool(id::tool_volt_source, "Voltage Source", resources::getVoltSourceBitmap(dip32, Item::RIGHT, true), wxNullBitmap, "Voltage Source");
    toolbar->AddRadioTool(id::tool_amp_source, "Current Source", resources::getAmpSourceBitmap(dip32, Item::RIGHT, true), wxNullBitmap, "Current Source");
    toolbar->AddRadioTool(id::tool_capacitor, "Capacitor", resources::getCapacitorBitmap(dip32, false), wxNullBitmap, "Capacitor");
    toolbar->AddRadioTool(id::tool_switch, "Switch", resources::getSwitchBitmap(dip32, false, false), wxNullBitmap, "Switch");
    toolbar->AddRadioTool(id::tool_bin, "Delete", resources::getBinBitmap(dip32), wxNullBitmap, "Delete");
    toolbar->Realize();
    toolbar->Fit();

    auto* fileMenu = new wxMenu();
    fileMenu->Append(id::file_save, "Save (CTRL+S)");
    fileMenu->Append(id::file_save_as, "Save As");
    fileMenu->Append(id::file_load, "Load (CTRL+L)");
    fileMenu->Append(id::file_new, "New (CTRL+N)");
    auto* viewMenu = new wxMenu();
    viewMenu->Append(id::view_dot_size, "Set grid dot size");
    viewMenu->Append(id::view_rotated_text, "Toggle rotated text");
    auto* editMenu = new wxMenu();
    editMenu->Append(wxID_UNDO, "Undo (CTRL+Z)");
    editMenu->Append(wxID_REDO, "Redo (CTRL+Y)");
    menuBar = new wxMenuBar();
    menuBar->Append(fileMenu, "File");
    menuBar->Append(viewMenu, "View");
    menuBar->Append(editMenu, "Edit");
    wxFrame::SetMenuBar(menuBar);

    if(fileIn.empty()) {
        windowGrid = new WindowGrid(this, wxID_ANY, wxDefaultPosition, GetClientSize());
    } else {
        std::filesystem::path path{fileIn};
        std::ifstream ifstream{path, std::ios_base::binary};
        WindowGrid::LoadStruct load = WindowGrid::load(ifstream);
        file = path;
        windowGrid = new WindowGrid(this, wxID_ANY, wxDefaultPosition, GetClientSize(), load);
    }
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
            toolbar->ToggleTool(id::tool_volt_source, true);
            windowGrid->selectedTool = Item::ItemType::volt_source;
            break;
        case '4':
            toolbar->ToggleTool(id::tool_amp_source, true);
            windowGrid->selectedTool = Item::ItemType::amp_source;
            break;
        case '5':
            toolbar->ToggleTool(id::tool_capacitor, true);
            windowGrid->selectedTool = Item::ItemType::capacitor;
            break;
        case '6':
            toolbar->ToggleTool(id::tool_switch, true);
            windowGrid->selectedTool = Item::ItemType::toggle;
            break;
        case '7':
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
        case 'Z':
            if(evt.GetModifiers() == wxMOD_CONTROL) {
                windowGrid->undo();
            }
            break;
        case 'Y':
            if(evt.GetModifiers() == wxMOD_CONTROL) {
                windowGrid->redo();
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
        } else {
            return;
        }
    }
    std::ofstream ofstream{file, std::ios_base::binary};
    if(ofstream.fail()) {
        wxMessageDialog{this, "Could not save", "Error", wxOK | wxICON_ERROR}.ShowModal();
    } else {
        windowGrid->save(ofstream);
    }
}

void FrameMain::onLoad() {
    if(!windowGrid->dirty || confirmClose("Are you sure you want to load a new file?")) {
        wxFileDialog dialog{this, "Load Schematic", "", "", "Schematic files (*.schematic)|*.schematic", wxFD_OPEN | wxFD_FILE_MUST_EXIST};
        if(dialog.ShowModal() == wxID_OK) {
            std::filesystem::path path{std::wstring_view{dialog.GetPath().wc_str()}};
            std::ifstream ifstream{path, std::ios_base::binary};
            try {
                WindowGrid::LoadStruct load = WindowGrid::load(ifstream);
                windowGrid->reload(load);
                file = path;
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
            file = std::filesystem::path{};
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