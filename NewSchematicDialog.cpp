#include "NewSchematicDialog.h"
#include "id.h"
#include "wx/propgrid/props.h"

NewSchematicDialog::NewSchematicDialog(wxWindow* parent) : wxDialog(parent, wxID_ANY, "New Schematic") {
    wxNumericPropertyValidator validator{wxNumericPropertyValidator::Unsigned};
    auto* sizeMessage = new wxStaticText{this, wxID_ANY, "Size: "};
    numRowsCtrl = new wxTextCtrl{this, wxID_ANY, "100", wxDefaultPosition, wxDefaultSize, 0, validator};
    auto* xSeparator = new wxStaticText{this, wxID_ANY, L"\u00D7"};
    wxFont font = xSeparator->GetFont();
    font.SetPointSize(15);
    xSeparator->SetFont(font);
    numColsCtrl = new wxTextCtrl{this, wxID_ANY, "100", wxDefaultPosition, wxDefaultSize, 0, validator};
    auto* upperSizer = new wxBoxSizer{wxHORIZONTAL};
    int dip5 = FromDIP(5);
    upperSizer->Add(sizeMessage, 0, wxLEFT | wxTOP | wxBOTTOM | wxALIGN_CENTER_VERTICAL, dip5);
    upperSizer->Add(numRowsCtrl, 1, wxTOP | wxBOTTOM | wxALIGN_CENTER_VERTICAL, dip5);
    upperSizer->Add(xSeparator, 0, wxALL | wxALIGN_CENTER_VERTICAL, dip5);
    upperSizer->Add(numColsCtrl, 1, wxRIGHT | wxTOP | wxBOTTOM | wxALIGN_CENTER_VERTICAL, dip5);
    auto* mainSizer = new wxBoxSizer{wxVERTICAL};
    mainSizer->Add(upperSizer);
    wxSizer* buttonSizer = wxDialog::CreateButtonSizer(wxOK | wxCANCEL);
    if(buttonSizer != nullptr) {
        buttonSizer->Layout();
        mainSizer->Add(buttonSizer, 0, wxBOTTOM | wxALIGN_CENTER_HORIZONTAL, dip5);
    }
    mainSizer->Layout();
    SetSizerAndFit(mainSizer);

    Bind(wxEVT_BUTTON, [this](wxCommandEvent& evt) {wxDialog::EndModal(wxID_OK);}, wxID_OK);
    Bind(wxEVT_BUTTON, [this](wxCommandEvent& evt) {wxDialog::EndModal(wxID_CANCEL);}, wxID_CANCEL);
}

wxSize NewSchematicDialog::getValue() const {
    wxMessageDialog invalidDialog{nullptr, "", "", wxOK | wxCENTRE | wxICON_WARNING};
    if(numRowsCtrl->GetValue().IsEmpty() || numColsCtrl->GetValue().IsEmpty()) {
        invalidDialog.SetMessage("Empty field");
        invalidDialog.ShowModal();
        return {0,0};
    }
    try {
        int cols = std::stoi(numColsCtrl->GetValue().utf8_string());
        int rows = std::stoi(numRowsCtrl->GetValue().utf8_string());
        if(cols == 0 || rows == 0) {
            invalidDialog.SetMessage("Dimension may not be 0");
            invalidDialog.ShowModal();
            return {0, 0};
        } else if(cols > 100000 || rows > 100000) {
            invalidDialog.SetMessage("Dimension may not be greater than 100000");
            invalidDialog.ShowModal();
            return {0,0};
        } else {
            return {cols, rows};
        }
    } catch (std::exception &e) { //Probably shouldn't happen
        invalidDialog.SetMessage("Failed to parse (shouldn't see this)");
        invalidDialog.ShowModal();
        return {0,0};
    }
}
