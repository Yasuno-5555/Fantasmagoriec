#pragma once
#include "core/font_types.hpp"
#include "core/types_internal.hpp" // For Vec2
#include <string>

namespace fanta {
namespace internal {

    class TextLayout {
    public:
        // Main function: Convert string to positioned quads (TextRun)
        static TextRun transform(const std::string& text, FontID font_id, float font_size);
        
        // Measure text without creating full quads
        // Added unused params max_width/line_height to match usage in layout.cpp
        static Vec2 measure(const std::string& text, FontID font_id, float font_size, float max_width = 0, float line_height = 0);
    };

} // namespace internal
} // namespace fanta
