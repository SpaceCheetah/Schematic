#pragma once
#include <string>

class Item {
public:
    enum class ItemType {
        none, resistor, wire
    };
    enum class Rotation {
        up, down, left, right
    };

    Item();
    Item(ItemType type, Rotation rotation, double value);
    ItemType getType() const;
    Rotation getRotation() const;
    double getValue() const;
    std::wstring getValueStr() const;
private:
    ItemType type;
    Rotation rotation;
    double value;
};