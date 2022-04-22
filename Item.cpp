#include <sstream>
#include <iomanip>
#include <utility>
#include "Item.h"

Item::Item(Item::ItemType type, int shape, double value, std::wstring extraData) : type{type}, shape{shape}, value{value}, extraData{std::move(extraData)} {}

Item::Item(std::ifstream& ifstream) {
    size_t stringSize;
    ifstream.read(reinterpret_cast<char*>(&type), sizeof(Item::ItemType));
    ifstream.read(reinterpret_cast<char*>(&shape), sizeof(int));
    ifstream.read(reinterpret_cast<char*>(&value), sizeof(double));
    ifstream.read(reinterpret_cast<char*>(&stringSize), sizeof(size_t));
    extraData = std::wstring(stringSize, 0); //using () to avoid initializer-list constructor
    ifstream.read(reinterpret_cast<char*>(extraData.data()), stringSize * sizeof(wchar_t));
}

void Item::save(std::ofstream& ofstream) {
    ofstream.write(reinterpret_cast<const char*>(&type), sizeof(Item::ItemType));
    ofstream.write(reinterpret_cast<const char*>(&shape), sizeof(int));
    ofstream.write(reinterpret_cast<const char*>(&value), sizeof(double));
    size_t stringSize = extraData.size();
    ofstream.write(reinterpret_cast<const char*>(&stringSize), sizeof(size_t));
    ofstream.write(reinterpret_cast<char*>(extraData.data()), stringSize * sizeof(wchar_t));
}


Item::ItemType Item::getType() const {
    return type;
}

int Item::getShape() const {
    return shape;
}

double Item::getValue() const {
    return value;
}

namespace {
    std::pair<double, wchar_t> getSI(double value) {
        double absValue = abs(value);
        if(absValue >= 1.0E24) {
            return {value / 1.0E24, 'Y'};
        } else if(absValue >= 1.0E21) {
            return {value / 1.0E21, 'Z'};
        } else if(absValue >= 1.0E18) {
            return {value / 1.0E18, 'E'};
        } else if(absValue >= 1.0E15) {
            return {value / 1.0E15, 'P'};
        } else if(absValue >= 1.0E12) {
            return {value / 1.0E12, 'T'};
        } else if(absValue >= 1.0E9) {
            return {value / 1.0E9, 'G'};
        } else if(absValue >= 1.0E6) {
            return {value / 1.0E6, 'M'};
        } else if(absValue >= 1.0E3) {
            return {value / 1.0E3, 'k'};
        } else if(absValue >= 1) {
            return {value, 0};
        } else if(absValue >= 1.0E-3) {
            return {value * 1.0E3, 'm'};
        } else if(absValue >= 1.0E-6) {
            return {value * 1.0E6, L'\u03bc'};
        } else if(absValue >= 1.0E-9) {
            return {value * 1.0E9, 'n'};
        } else if(absValue >= 1.0E-12) {
            return {value * 1.0E12, 'p'};
        } else if(absValue >= 1.0E-15) {
            return {value * 1.0E15, 'f'};
        } else if(absValue >= 1.0E-18) {
            return {value * 1.0E18, 'a'};
        } else if(absValue >= 1.0E-21) {
            return {value * 1.0E21, 'z'};
        } else if(absValue >= 1.0E-24) {
            return {value * 1.0E24, 'y'};
        }
        return {0, 0};
    }
    std::wstring valueToStr(double value, wchar_t unit) {
        std::wstringstream stream{};
        stream << std::setprecision(4);
        std::pair<double, wchar_t> si = getSI(value);
        stream << si.first;
        if (si.second != 0) {
            stream << si.second;
        }
        stream << unit;
        return stream.str();
    }
}

std::wstring Item::getValueStr() const {
    if(!extraData.empty()) {
        return extraData;
    }
    switch(type) {
        case ItemType::resistor: {
            return valueToStr(value, L'\u03A9');
        }
        case ItemType::volt_source:
            return valueToStr(value, 'V');
        case ItemType::amp_source:
            return valueToStr(value, 'A');
        default:
            return L"";
    }
}

std::wstring Item::getExtraData() const {
    return extraData;
}