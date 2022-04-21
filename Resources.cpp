#include "Resources.h"

wxImage resources::getWireImage() {
    static wxImage wireImage{"res/wire.png", wxBITMAP_TYPE_PNG};
    return wireImage;
}

wxImage resources::getBinImage() {
    static wxImage binImage{"res/bin.png", wxBITMAP_TYPE_PNG};
    return binImage;
}

wxIconBundle resources::getResistorIconBundle() {
    static wxIconBundle resistorBundle{"res/resistor-multires.ico", wxBITMAP_TYPE_ICO};
    return resistorBundle;
}

wxBitmap resources::getResistorBitmap(int size, bool rotated) {
    return wxBitmap();
}
