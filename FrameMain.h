#pragma once
#include <wx/wx.h>
#include <filesystem>
#include "WindowGrid.h"

class FrameMain : public wxFrame {
public:
    explicit FrameMain(const std::wstring& file = {});
private:
    void onSize(wxSizeEvent& evt);
    void onChar(wxKeyEvent& evt);
    void onSave(bool saveAs);
    void onLoad();
    void onNew();
    void onClose(wxCloseEvent& evt);
    bool confirmClose(const wxString& message);
    wxToolBar* toolbar;
    wxMenuBar* menuBar;
    WindowGrid* windowGrid;
    std::filesystem::path file{};
};