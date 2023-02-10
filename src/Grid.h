#pragma once

#include <vector>
#include <cassert>

namespace numlin {

template <class T>
class Grid {
public:
    Grid(int height, int width) : data_(height * width), height_(height), width_(width) {}
    Grid(int height, int width, const T& def) : data_(height * width, def), height_(height), width_(width) {}
    Grid(const Grid<T>&) = default;

    int height() const {
        return height_;
    }

    int width() const {
        return width_;
    }

    const T& at(int y, int x) const {
        assert(0 <= y && y < height_ && 0 <= x && x < width_);
        return data_[y * width_ + x];
    }

    T& at(int y, int x) {
        assert(0 <= y && y < height_ && 0 <= x && x < width_);
        return data_[y * width_ + x];
    }

private:
    std::vector<T> data_;
    int height_;
    int width_;
};

}
