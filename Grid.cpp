#include "Grid.h"
#include <utility>
#include <stdexcept>
#include <string>

Grid::Grid(uint32_t width, uint32_t height, std::unordered_map<uint64_t, Item> gridMap) : width{width}, height{height},
                                                                                gridMap{std::move(gridMap)} {}

uint32_t Grid::getWidth() const {
    return width;
}

uint32_t Grid::getHeight() const {
    return height;
}

Item Grid::get(uint32_t row, uint32_t col) const {
    rangeCheck(row, col);
    auto iterator = gridMap.find((static_cast<uint64_t>(row) << 32) | col);
    if(iterator == gridMap.end()) {
        return Item{};
    }
    return iterator->second;
}

void Grid::set(uint32_t row, uint32_t col, Item item) {
    rangeCheck(row, col);
    gridMap[(static_cast<uint64_t>(row) << 32) | col] = item;
}

void Grid::rangeCheck(uint32_t row, uint32_t col) const {
    if (row > height || col > width) {
        throw std::out_of_range(
                std::string("Range check: (") + std::to_string(row) + "," + std::to_string(col) + ") outside (" +
                std::to_string(height) + "," + std::to_string(width) + ")");
    }
}