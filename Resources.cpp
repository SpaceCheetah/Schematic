#include "Resources.h"

wxImage resources::getResistorImage() {
    static wxImage resistorImage{"res/resistor.png", wxBITMAP_TYPE_PNG};
    return resistorImage;
}

wxIconBundle resources::getResistorIconBundle() {
    static wxIconBundle resistorBundle{"res/resistor-multires.ico", wxBITMAP_TYPE_ICO};
    return resistorBundle;
}
