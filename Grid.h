#pragma once
#include <unordered_map>
#include "Item.h"

class Grid {
    uint32_t width;
    uint32_t height;
    void rangeCheck(uint32_t row, uint32_t col) const;
public:
    //Using map with uint64_t key as an efficient 2d array with a large number of elements
    std::unordered_map<uint64_t, Item> gridMap;
    explicit Grid(uint32_t width = 100, uint32_t height = 100, std::unordered_map<uint64_t,Item> gridMap = {});
    //Using get-set instead of operator[][], because maps would create an empty item with [][] for a new key
    Item get(uint32_t row, uint32_t col) const;
    void set(uint32_t row, uint32_t col, Item item);
    uint32_t getWidth() const;
    uint32_t getHeight() const;
};