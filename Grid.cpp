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

void Grid::set(uint32_t row, uint32_t col, const Item& item) {
    Item previous = get(row, col);
    uint64_t key = (static_cast<uint64_t>(row) << 32) | col;
    if(item.type == Item::ItemType::none) {
        gridMap.erase(key);
    } else {
        gridMap[key] = item;
    }
    undoHistory[undoOperations] = {key, previous};
    undoOperations ++;
    redoOperations = 0;
    if(undoOperations == 100) {
        for(int i = 0; i < 99; i ++) {
            undoHistory[i] = undoHistory[i + 1];
        }
        undoOperations = 99;
    }
}

void Grid::rangeCheck(uint32_t row, uint32_t col) const {
    if (row > height || col > width) {
        throw std::out_of_range(
                std::string("Range check: (") + std::to_string(row) + "," + std::to_string(col) + ") outside (" +
                std::to_string(height) + "," + std::to_string(width) + ")");
    }
}

//swap the current state of the map and operation.second at operation.first
void Grid::doOperation(std::pair<uint64_t, Item> &operation) {
    std::pair<uint64_t, Item> operationCopy = operation;
    auto iterator = gridMap.find(operation.first);
    if(iterator == gridMap.end()) {
        operation = {operation.first, Item{}};
        if(operationCopy.second.type != Item::ItemType::none) {
            gridMap[operation.first] = operationCopy.second;
        }
    } else {
        operation = {operation.first, iterator->second};
        if(operationCopy.second.type == Item::ItemType::none) {
            gridMap.erase(iterator);
        } else {
            gridMap[operation.first] = operationCopy.second;
        }
    }
}

bool Grid::undo() {
    if(undoOperations > 0) {
        undoOperations --;
        redoOperations ++;
        doOperation(undoHistory[undoOperations]);
        return true;
    }
    return false;
}

bool Grid::redo() {
    if(redoOperations > 0) {
        doOperation(undoHistory[undoOperations]);
        redoOperations --;
        undoOperations ++;
        return true;
    }
    return false;
}
