#pragma once
#include <wx/wx.h>
#include <filesystem>

namespace resources {
    wxBitmap getBinBitmap(int size);
    wxBitmap getWireBitmap(int size);
    wxBitmap getResistorBitmap(int size, bool rotated);
    wxIconBundle getResistorIconBundle();
    wxBitmap getVoltSourceBitmap(int size, int shape);
    wxBitmap getAmpSourceBitmap(int size, int shape);
    wxBitmap getCapacitorBitmap(int size, bool rotated);
}