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
    std::wstring valueToStr(double value, wchar_t unit, int split) {
        std::wstringstream stream{};
        stream << std::setprecision(4);
        std::pair<double, wchar_t> si = getSI(value);
        stream << si.first;
        int valueLength = static_cast<int>(stream.tellp());
        if(split && valueLength + 1 + (si.second == 0 ? 0 : 1) > split) {
            stream << '\n';
        }
        if (si.second != 0) {
            stream << si.second;
        }
        stream << unit;
        return stream.str();
    }
}

std::wstring Item::getValueStr(int split) const {
    if(!extraData.empty()) {
        if(split != 0 && extraData.size() > split) {
            std::wstring splitData{};
            for(int i = 0; i < extraData.size(); i ++) {
                if(i % split == 0 && i != 0) splitData += '\n';
                splitData += extraData[i];
            }
            return splitData;
        }
        return extraData;
    }
    switch(type) {
        case ItemType::resistor: {
            return valueToStr(value, L'\u03A9', split);
        }
        case ItemType::volt_source:
            return valueToStr(value, 'V', split);
        case ItemType::amp_source:
            return valueToStr(value, 'A', split);
        case ItemType::capacitor:
            return valueToStr(value, 'F', split);
        default:
            return L"";
    }
}

void Item::draw(wxDC& dc, int cellSize, int dotSize, bool rotatedText, wxBitmap* resistorBitmaps, wxBitmap* capacitorBitmaps, wxBitmap* ampSourceBitmaps, wxBitmap* voltSourceBitmaps, wxBitmap* switchBitmaps) {
    switch(type) {
        case ItemType::none:
            if(dotSize != -1) {
                dc.DrawCircle(cellSize / 2 , cellSize / 2, std::max(cellSize * dotSize / 128, 1));
            }
            break;
        case Item::ItemType::resistor: {
            if (shape == Item::HORIZONTAL) {
                dc.DrawBitmap(resistorBitmaps[0], 0, 0);
                dc.DrawLabel(getValueStr(), wxRect{0, cellSize * 4 / 17, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
            } else {
                dc.DrawBitmap(resistorBitmaps[1], 0, 0);
                if(rotatedText) {
                    std::wstring valueStr = getValueStr();
                    wxSize textSize = dc.GetTextExtent(valueStr);
                    dc.DrawRotatedText(valueStr, cellSize * 40 / 51, cellSize / 2 - textSize.GetWidth() / 2, 270);
                } else {
                    dc.DrawLabel(getValueStr(5), wxRect{cellSize * 11 / 17, 0, 0, cellSize}, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
                }
            }
            break;
        }
        case Item::ItemType::capacitor: {
            if (shape == Item::HORIZONTAL) {
                dc.DrawBitmap(capacitorBitmaps[0], 0, 0);
                dc.DrawLabel(getValueStr(), wxRect{0, cellSize * 2 / 17, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
            } else {
                dc.DrawBitmap(capacitorBitmaps[1], 0, 0);
                if(rotatedText) {
                    std::wstring valueStr = getValueStr();
                    wxSize textSize = dc.GetTextExtent(valueStr);
                    dc.DrawRotatedText(valueStr, cellSize * 31 / 34, cellSize / 2 - textSize.GetWidth() / 2, 270);
                } else {
                    dc.DrawLabel(getValueStr(6), wxRect{cellSize * 4 / 7, 0, 0, cellSize / 2}, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
                }
            }
            break;
        }
        case Item::ItemType::wire: {
            int directions = 0;
            bool up, down, left, right;
            up = down = left = right = false;
            wxPoint middle = wxPoint{cellSize / 2, cellSize / 2};
            if (shape & Item::UP) {
                dc.DrawLine(wxPoint{cellSize / 2, 0}, middle);
                directions++;
                up = true;
            }
            if (shape & Item::DOWN) {
                dc.DrawLine(wxPoint{cellSize / 2, cellSize}, middle);
                directions++;
                down = true;
            }
            if (shape & Item::LEFT) {
                dc.DrawLine(wxPoint{0, cellSize / 2}, middle);
                directions++;
                left = true;
            }
            if (shape & Item::RIGHT) {
                dc.DrawLine(wxPoint{cellSize, cellSize / 2}, middle);
                directions++;
                right = true;
            }
            if (directions > 2) {
                dc.DrawCircle(middle, std::max(cellSize * 3 / 128, 1));
            }
            if(!extraData.empty()) {
                wxSize textSize = dc.GetTextExtent(extraData);
                if(directions == 4 || (up && right && directions == 2) || (right && directions == 1) || (up && directions == 1 && !rotatedText)) { //draw in top right corner
                    dc.DrawLabel(getValueStr(6), wxRect{cellSize * 13/24, 0, 0, cellSize / 2}, wxALIGN_BOTTOM | wxALIGN_LEFT);
                } else if(left && right) { //draw horizontally centered
                    if(up) {
                        dc.DrawLabel(extraData, wxRect{0, cellSize / 2, cellSize, 0}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
                    } else {
                        dc.DrawLabel(extraData, wxRect{0, 0, cellSize, cellSize / 2}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_BOTTOM);
                    }
                } else if(up && down) { //draw vertically centered
                    if(rotatedText) {
                        if(right) {
                            dc.DrawRotatedText(extraData, cellSize / 2, cellSize / 2 - textSize.GetWidth() / 2, 270);
                        } else {
                            dc.DrawRotatedText(extraData, cellSize * 13 / 24 + textSize.GetHeight(), cellSize / 2 - textSize.GetWidth() / 2, 270);
                        }
                    } else {
                        std::wstring valueStr = getValueStr(6);
                        if(right) {
                            dc.DrawLabel(valueStr, wxRect{0, 0, cellSize * 11 / 24, cellSize}, wxALIGN_CENTER_VERTICAL | wxALIGN_RIGHT);
                        } else {
                            dc.DrawLabel(valueStr, wxRect{cellSize * 13 / 24, 0, 0, cellSize}, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
                        }
                    }
                } else if(right || (down && directions == 1 && !rotatedText)) { //draw in bottom right corner
                    dc.DrawLabel(getValueStr(6), wxRect{cellSize * 13/24, cellSize * 13 / 24, 0, 0}, wxALIGN_TOP | wxALIGN_LEFT);
                } else if(left && down) { //Draw in bottom left corner
                    dc.DrawLabel(getValueStr(6), wxRect{0, cellSize * 13 / 24, cellSize * 11 / 24, 0}, wxALIGN_RIGHT | wxALIGN_TOP);
                } else if(left) { //Draw in top left corner
                    dc.DrawLabel(getValueStr(6), wxRect{0, 0, cellSize * 11 / 24, cellSize * 11 / 24}, wxALIGN_RIGHT | wxALIGN_BOTTOM);
                } else if(up) { //Draw in top right corner, rotated
                    dc.DrawRotatedText(extraData, cellSize * 13 / 24 + textSize.GetHeight(), cellSize / 4 - textSize.GetWidth() / 2, 270);
                } else if(down) { //Draw in bottom right corner, rotated
                    dc.DrawRotatedText(extraData, cellSize * 13 / 24 + textSize.GetHeight(), cellSize * 3 / 4 - textSize.GetWidth() / 2, 270);
                } else { //Draw in center
                    dc.DrawLabel(extraData, wxRect{0, 0, cellSize, cellSize}, wxALIGN_CENTER);
                }
            }
            break;
        }
        case Item::ItemType::amp_source: case Item::ItemType::volt_source: {
            wxBitmap* bitmaps = type == Item::ItemType::amp_source ? ampSourceBitmaps : voltSourceBitmaps;
            int bitmapIndex;
            if(shape & Item::UP) bitmapIndex = 0;
            else if(shape & Item::DOWN) bitmapIndex = 1;
            else if(shape & Item::RIGHT) bitmapIndex = 2;
            else bitmapIndex = 3;
            if(shape & Item::DEPENDENT) bitmapIndex += 4;
            dc.DrawBitmap(bitmaps[bitmapIndex], 0, 0);
            if((shape & Item::LEFT) || (shape & Item::RIGHT)) {
                dc.DrawLabel(getValueStr(), wxRect{0, cellSize * 5 / 34, cellSize, cellSize}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
            } else if(rotatedText) {
                std::wstring valueStr = getValueStr();
                wxSize textSize = dc.GetTextExtent(valueStr);
                dc.DrawRotatedText(valueStr, cellSize * 15 / 17, cellSize / 2 - textSize.GetWidth() / 2, 270);
            } else {
                dc.DrawLabel(getValueStr(4), wxRect{cellSize * 25 / 34, 0, cellSize, cellSize}, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
            }
            break;
        }
        case Item::ItemType::toggle: {
            bool vertical = false;
            bool closed = false;
            int bitmapIndex = 0;
            if(shape & Item::VERTICAL) {
                bitmapIndex = 1;
                vertical = true;
            }
            if(shape & Item::CLOSED) {
                bitmapIndex += 2;
                closed = true;
            }
            dc.DrawBitmap(switchBitmaps[bitmapIndex], 0, 0);
            if(vertical) {
                if(rotatedText) {
                    wxSize textSize = dc.GetTextExtent(extraData);
                    dc.DrawRotatedText(extraData, cellSize * 13 / 17, cellSize / 2 - textSize.GetWidth() / 2, 270);
                } else {
                    dc.DrawLabel(getValueStr(5), wxRect{cellSize * 10 / 17, 0, 0, cellSize}, wxALIGN_CENTER_VERTICAL | wxALIGN_LEFT);
                }
            } else if(closed) {
                dc.DrawLabel(extraData, wxRect{0, cellSize * 5 / 17, cellSize, 0}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
            } else {
                dc.DrawLabel(extraData, wxRect{0, cellSize * 10 / 17, cellSize, 0}, wxALIGN_CENTER_HORIZONTAL | wxALIGN_TOP);
            }
        }
    }
}

double Item::defaultValue(Item::ItemType type) {
    switch(type) {
        case ItemType::resistor:
            return 100;
        case ItemType::volt_source: case Item::ItemType::amp_source:
            return 10;
        case ItemType::capacitor:
            return 1e-4;
        default:
            return 0;
    }
}
