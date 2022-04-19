#include <sstream>
#include <iomanip>
#include "Item.h"

Item::Item(Item::ItemType type, int shape, double value) : type{type}, shape{shape}, value{value} {}

Item::ItemType Item::getType() const {
    return type;
}

int Item::getShape() const {
    return shape;
}

double Item::getValue() const {
    return value;
}

Item::Item() : Item(Item::ItemType::none, 0, 0) {}

namespace {
    std::pair<double, wchar_t> getSI(double value) {
        double absValue = abs(value);
        if(absValue > 1.0E24) {
            return {value / 1.0E24, 'Y'};
        } else if(absValue > 1.0E21) {
            return {value / 1.0E21, 'Z'};
        } else if(absValue > 1.0E18) {
            return {value / 1.0E18, 'E'};
        } else if(absValue > 1.0E15) {
            return {value / 1.0E15, 'P'};
        } else if(absValue > 1.0E12) {
            return {value / 1.0E12, 'T'};
        } else if(absValue > 1.0E9) {
            return {value / 1.0E9, 'G'};
        } else if(absValue > 1.0E6) {
            return {value / 1.0E6, 'M'};
        } else if(absValue > 1.0E3) {
            return {value / 1.0E3, 'k'};
        } else if(absValue > 1) {
            return {value, 0};
        } else if(absValue > 1.0E-3) {
            return {value * 1.0E3, 'm'};
        } else if(absValue > 1.0E-6) {
            return {value * 1.0E6, L'\u03bc'};
        } else if(absValue > 1.0E-9) {
            return {value * 1.0E9, 'n'};
        } else if(absValue > 1.0E-12) {
            return {value * 1.0E12, 'p'};
        } else if(absValue > 1.0E-15) {
            return {value * 1.0E15, 'f'};
        } else if(absValue > 1.0E-18) {
            return {value * 1.0E18, 'a'};
        } else if(absValue > 1.0E-21) {
            return {value * 1.0E21, 'z'};
        } else if(absValue > 1.0E-24) {
            return {value * 1.0E24, 'y'};
        }
        return {0, 0};
    }
}

std::wstring Item::getValueStr() const {
    std::wstringstream stream{};
    stream << std::setprecision(4);
    switch(type) {
        case ItemType::none: {
            return L"";
        }
        case ItemType::resistor: {
            std::pair<double, wchar_t> si = getSI(value);
            stream << si.first;
            if (si.second != 0) {
                stream << si.second;
            }
            stream << L'\u03A9';
            return stream.str();
        }
        case ItemType::wire: {
            return L"";
        }
    }
    return L"";
}
