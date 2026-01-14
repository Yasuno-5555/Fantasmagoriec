#include <fanta.h>
#include <array>

namespace fanta {

// Helper for constructing 0xRRGGBBAA within this translation unit
static constexpr uint32_t RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255) {
    return (r << 24) | (g << 16) | (b << 8) | a;
}

Theme Theme::Dark() {
    Theme t;
    auto& p = t.palette;
    using CT = ColorToken;
    p[(int)CT::None] = 0;
    p[(int)CT::SystemBackground] = RGBA(30, 30, 30);
    p[(int)CT::SecondarySystemBackground] = RGBA(45, 45, 45);
    p[(int)CT::TertiarySystemBackground] = RGBA(60, 60, 60);
    p[(int)CT::Label] = RGBA(255, 255, 255);
    p[(int)CT::SecondaryLabel] = RGBA(235, 235, 245, 153); // 60%
    p[(int)CT::TertiaryLabel] = RGBA(235, 235, 245, 76);  // 30%
    p[(int)CT::QuaternaryLabel] = RGBA(235, 235, 245, 45); // 18%
    p[(int)CT::Fill] = RGBA(120, 120, 128, 51); // 20%
    p[(int)CT::SecondaryFill] = RGBA(120, 120, 128, 40); // 16%
    p[(int)CT::Separator] = RGBA(84, 84, 88, 153); // 60%
    p[(int)CT::Accent] = RGBA(10, 132, 255); // System Blue
    p[(int)CT::Destructive] = RGBA(255, 69, 58); // System Red
    
    // Typography defaults
    auto& ty = t.typography;
    using TT = TextToken;
    ty[(int)TT::LargeTitle] = {34.0f};
    ty[(int)TT::Title1] = {28.0f};
    ty[(int)TT::Title2] = {22.0f};
    ty[(int)TT::Title3] = {20.0f};
    ty[(int)TT::Headline] = {17.0f};
    ty[(int)TT::Body] = {17.0f};
    ty[(int)TT::Callout] = {16.0f};
    ty[(int)TT::Subhead] = {15.0f};
    ty[(int)TT::Footnote] = {13.0f};
    ty[(int)TT::Caption1] = {12.0f};
    ty[(int)TT::Caption2] = {11.0f};
    
    return t;
}

Theme Theme::Light() {
    Theme t = Theme::Dark(); // Copy typography
    auto& p = t.palette;
    using CT = ColorToken;
    p[(int)CT::SystemBackground] = RGBA(255, 255, 255);
    p[(int)CT::SecondarySystemBackground] = RGBA(242, 242, 247);
    p[(int)CT::TertiarySystemBackground] = RGBA(255, 255, 255);
    p[(int)CT::Label] = RGBA(0, 0, 0);
    p[(int)CT::SecondaryLabel] = RGBA(60, 60, 67, 153);
    p[(int)CT::TertiaryLabel] = RGBA(60, 60, 67, 76);
    p[(int)CT::QuaternaryLabel] = RGBA(60, 60, 67, 45);
    p[(int)CT::Fill] = RGBA(120, 120, 128, 51);
    p[(int)CT::SecondaryFill] = RGBA(120, 120, 128, 40);
    p[(int)CT::Separator] = RGBA(60, 60, 67, 73);
    p[(int)CT::Accent] = RGBA(0, 122, 255);
    p[(int)CT::Destructive] = RGBA(255, 59, 48);
    return t;
}

Theme Theme::Fantasmagorie() {
    Theme t = Theme::Dark();
    auto& p = t.palette;
    using CT = ColorToken;
    // Deep purple/slate background
    p[(int)CT::SystemBackground] = RGBA(20, 20, 28);
    p[(int)CT::SecondarySystemBackground] = RGBA(30, 30, 40);
    p[(int)CT::Accent] = RGBA(180, 100, 255); // Neon Purple
    return t;
}

Theme Theme::HighContrast() {
    Theme t = Theme::Dark();
    auto& p = t.palette;
    using CT = ColorToken;
    p[(int)CT::SystemBackground] = RGBA(0, 0, 0);
    p[(int)CT::SecondarySystemBackground] = RGBA(0, 0, 0);
    p[(int)CT::Label] = RGBA(255, 255, 255);
    p[(int)CT::SecondaryLabel] = RGBA(255, 255, 0); // Yellow
    p[(int)CT::Accent] = RGBA(0, 255, 255); // Cyan
    p[(int)CT::Separator] = RGBA(255, 255, 255);
    return t;
}

Theme Theme::AppleHIG() { return Theme::Light(); }

} // namespace fanta
