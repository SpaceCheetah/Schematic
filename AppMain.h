#pragma once
#include <wx/wx.h>

class AppMain : public wxApp {
public:
    bool OnInit() override;
    int OnExit() override;
};