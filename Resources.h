#pragma once
#include <wx/wx.h>

namespace resources {
    wxImage getWireImage();
    wxImage getBinImage();
    wxIconBundle getResistorIconBundle();
    wxBitmap getResistorBitmap(int size, bool rotated);
}