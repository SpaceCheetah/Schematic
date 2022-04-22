#pragma once
#include <wx/wx.h>

namespace resources {
    wxImage getBinImage();
    wxBitmap getWireBitmap(int size);
    wxBitmap getResistorBitmap(int size, bool rotated);
    wxBitmap getVoltSourceBitmap(int size, int shape);
    wxBitmap getAmpSourceBitmap(int size, int shape);
}