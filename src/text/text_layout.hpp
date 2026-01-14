#pragma once
#include "core/font_types.hpp"
#include "core/types_internal.hpp" // For Vec2
#include <string>

namespace fanta {
namespace internal {

    class TextLayout {
    public:
        // Main function: Convert string to positioned quads (TextRun)
        static TextRun transform(const std::string& text, FontID font_id, const TypographyRule& rule);
        static TextRun transform_wrapped(const std::string& text, FontID font_id, const TypographyRule& rule, float max_width);
        
        // Measure text without creating full quads
        static Vec2 measure(const std::string& text, FontID font_id, const TypographyRule& rule, float max_width = 0);
    };

} // namespace internal
} // namespace fanta
