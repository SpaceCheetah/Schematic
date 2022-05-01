#pragma once
#include <string>
#include <fstream>
#include "wx/dc.h"

class Item {
public:
    enum class ItemType {
        none, resistor, wire, volt_source, amp_source, capacitor
    };

    constexpr static int HORIZONTAL = 0;
    constexpr static int VERTICAL = 1;
    constexpr static int UP = 1;
    constexpr static int DOWN = 2;
    constexpr static int RIGHT = 4;
    constexpr static int LEFT = 8;
    constexpr static int DEPENDENT = 16;

    ItemType type;
    int shape;
    double value;
    std::wstring extraData;

    Item() = default;
    Item(ItemType type, int shape, double value, std::wstring extraData = std::wstring{});
    explicit Item(std::ifstream& ifstream);
    void save(std::ofstream& ofstream);
    void draw(wxDC& dc, int cellSize, int dotSize, bool rotatedText, wxBitmap* resistorBitmaps, wxBitmap* capacitorBitmaps, wxBitmap* ampSourceBitmaps, wxBitmap* voltSourceBitmaps);
private:
    std::wstring getValueStr(int split = 0) const;
};