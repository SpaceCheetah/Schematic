#pragma once
#include <unordered_map>
#include "Item.h"

class Grid {
    uint32_t width;
    uint32_t height;
    std::unordered_map<uint64_t, Item> gridMap;
    class GridRow {
        Grid& grid;
        uint32_t row;
    public:
        GridRow(Grid& grid, uint32_t row);
        Item& operator[](uint32_t col);
    };
public:
    //Using map with uint64_t key as an efficient 2d array with a large number of elements
    explicit Grid(uint32_t width = 100, uint32_t height = 100, std::unordered_map<uint64_t,Item> gridMap = {});
    GridRow operator[](uint32_t row);
    uint32_t getWidth() const;
    uint32_t getHeight() const;
};