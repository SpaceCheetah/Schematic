#pragma once
#include <wx/wx.h>

class NewSchematicDialog : public wxDialog {
public:
    NewSchematicDialog(wxWindow* parent);
    wxSize getValue() const;
private:
    wxTextCtrl* numRowsCtrl;
    wxTextCtrl* numColsCtrl;
};
