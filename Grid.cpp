#include "Grid.h"
#include <utility>
#include <stdexcept>
#include <string>

Grid::Grid(uint32_t width, uint32_t height, std::unordered_map<uint64_t, Item> gridMap) : width{width}, height{height},
                                                                                gridMap{std::move(gridMap)} {}

Grid::GridRow Grid::operator[](uint32_t row) {
    return {*this, row};
}

uint32_t Grid::getWidth() const {
    return width;
}

uint32_t Grid::getHeight() const {
    return height;
}

Grid::GridRow::GridRow(Grid &grid, uint32_t row) : grid{grid}, row{row} {}

Item &Grid::GridRow::operator[](uint32_t col) {
    if (row > grid.getHeight() || col > grid.getWidth()) {
        throw std::out_of_range(
                std::string("Range check: (") + std::to_string(row) + "," + std::to_string(col) + ") outside (" +
                std::to_string(grid.getHeight()) + "," + std::to_string(grid.getWidth()) + ")");
    }
    return grid.gridMap[(static_cast<uint64_t>(row) << 32) | col];
}
