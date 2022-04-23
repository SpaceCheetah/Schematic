#pragma once
#include <string>
#include <fstream>

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

    Item() = default;
    Item(ItemType type, int shape, double value, std::wstring extraData = std::wstring{});
    explicit Item(std::ifstream& ifstream);
    void save(std::ofstream& ofstream);
    ItemType getType() const;
    int getShape() const;
    double getValue() const;
    std::wstring getValueStr() const;
    std::wstring getExtraData() const;
private:
    ItemType type;
    int shape;
    double value;
    std::wstring extraData;
};