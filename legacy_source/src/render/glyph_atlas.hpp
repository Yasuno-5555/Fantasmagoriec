// Fantasmagorie v3 - Glyph Atlas
// Dynamic texture packer for font glyphs
#pragma once

#include "core/types.hpp"
#include <vector>
#include <algorithm>

namespace fanta {

struct AtlasRect {
    int x, y, w, h;
};

class GlyphAtlas {
public:
    GlyphAtlas(int width = 1024, int height = 1024)
        : width_(width), height_(height) {
        // Simple shelf packer
        shelves_.push_back({0, 0, width, 0});
    }

    // Allocate space in atlas
    bool allocate(int w, int h, AtlasRect& out) {
        // Add padding
        int pw = w + 2;
        int ph = h + 2;

        for (auto& shelf : shelves_) {
            if (shelf.x + pw <= shelf.w && ph <= (height_ - shelf.y)) {
                // Found space on existing shelf
                out = {shelf.x + 1, shelf.y + 1, w, h};
                shelf.x += pw;
                shelf.h = std::max(shelf.h, ph);
                used_area_ += w * h;
                return true;
            }
        }

        // Try start new shelf
        Shelf& last = shelves_.back();
        int next_y = last.y + last.h;
        if (next_y + ph <= height_) {
            shelves_.push_back({pw, next_y, width_, ph});
            out = {1, next_y + 1, w, h};
            used_area_ += w * h;
            return true;
        }

        return false; // Out of space
    }

    int width() const { return width_; }
    int height() const { return height_; }
    float utilization() const { return (float)used_area_ / (width_ * height_); }

    void clear() {
        shelves_.clear();
        shelves_.push_back({0, 0, width_, 0});
        used_area_ = 0;
    }

private:
    struct Shelf {
        int x, y, w, h;
    };

    int width_, height_;
    long long used_area_ = 0;
    std::vector<Shelf> shelves_;
};

} // namespace fanta


