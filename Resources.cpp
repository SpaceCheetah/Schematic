#include "Resources.h"

wxImage resources::getResistorImage() {
    static wxImage resistorImage{"res/resistor.png", wxBITMAP_TYPE_PNG};
    return resistorImage;
}

wxImage resources::getWireImage() {
    static wxImage wireImage{"res/wire.png", wxBITMAP_TYPE_PNG};
    return wireImage;
}

wxIconBundle resources::getResistorIconBundle() {
    static wxIconBundle resistorBundle{"res/resistor-multires.ico", wxBITMAP_TYPE_ICO};
    return resistorBundle;
}
