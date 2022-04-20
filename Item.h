#pragma once
#include <string>

class Item {
public:
    enum class ItemType {
        none, resistor, wire
    };

    constexpr static int HORIZONTAL = 0;
    constexpr static int VERTICAL = 1;
    constexpr static int UP = 1;
    constexpr static int DOWN = 2;
    constexpr static int RIGHT = 4;
    constexpr static int LEFT = 8;

    Item() = default;
    Item(ItemType type, int shape, double value);
    ItemType getType() const;
    int getShape() const;
    double getValue() const;
    std::wstring getValueStr() const;
private:
    ItemType type;
    int shape;
    double value;
};