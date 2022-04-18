#include "Resources.h"

wxImage resources::getResistorImage() {
    static wxImage resistorImage{"res/resistor.png", wxBITMAP_TYPE_PNG};
    return resistorImage;
}