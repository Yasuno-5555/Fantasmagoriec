#pragma once
#include <string>
#include <vector>
#include <cstdint>
#include "core/font_types.hpp"

namespace fanta {
namespace internal {

    struct ShapedGlyph {
        uint32_t glyph_index;
        float x_advance, y_advance;
        float x_offset, y_offset;
        uint32_t cluster;
    };

    class TextShaper {
    public:
        static std::vector<ShapedGlyph> shape(const std::string& text, FontID font);
    };

} // namespace internal
} // namespace fanta
