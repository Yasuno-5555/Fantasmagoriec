#pragma once
// L2: DrawList (Intermediate Representation)
// Dependencies: types_core.hpp (L1), <vector> ONLY.
// This file MUST be extractable to a separate project.

#include "core/types_core.hpp"
#include <vector>
#include <cstdint>

namespace fanta {
namespace internal {

    // --- Path Types ---
    enum class PathVerb { MoveTo, LineTo, QuadTo, CubicTo, Close };
    
    struct PathPoint {
        PathVerb verb;
        Vec2 p0, p1, p2;
    };
    
    struct Path {
        std::vector<PathPoint> points;
        float stroke_width = 1.0f;
        bool is_fill = false;
        ColorF color;
    };

    // --- Text Run (Font Metrics) ---
    // Forward declaration to break circular dependency
    struct TextRun;
    using FontID = uint32_t;

    // --- Blend Mode ---
    enum class BlendMode {
        SourceOver,
        SourceIn, SourceOut, SourceAtop,
        DestOver, DestIn, DestOut, DestAtop,
        Xor, Plus, Multiply, Screen, Overlay,
        Darken, Lighten, ColorDodge, ColorBurn,
        HardLight, SoftLight, Difference, Exclusion
    };

    // --- Draw Command ---
    enum class DrawCmdType {
        Rectangle,
        RoundedRect,
        Circle,
        Line,
        Text,
        Glyph,
        Bezier,
        BlurRect,
        PushTransform,
        PopTransform,
        PushClip,
        PopClip,
        Path,
        MeshGradient
    };

    struct DrawCmd {
        DrawCmdType type;
        BlendMode blend = BlendMode::SourceOver;
        
        union {
            struct {
                float pos_x, pos_y;
                float size_x, size_y;
                float color_r, color_g, color_b, color_a;
                float elevation;
            } rectangle;
            
            struct {
                float pos_x, pos_y;
                float size_x, size_y;
                float radius;
                float color_r, color_g, color_b, color_a;
                float elevation;
                bool is_squircle;
                float border_width;
                float border_color_r, border_color_g, border_color_b, border_color_a;
                float glow_strength;
                float glow_color_r, glow_color_g, glow_color_b, glow_color_a;
                float blur_strength;
            } rounded_rect;
            
            struct {
                float center_x, center_y;
                float r;
                float color_r, color_g, color_b, color_a;
                float elevation;
            } circle;
            
            struct {
                float p0_x, p0_y;
                float p1_x, p1_y;
                float thickness;
                float color_r, color_g, color_b, color_a;
            } line;
            
            struct {
                uint32_t run_index;
                float color_r, color_g, color_b, color_a;
            } text;

            struct {
                float pos_x, pos_y;
                float size_x, size_y;
                float u0, v0, u1, v1;
                float color_r, color_g, color_b, color_a;
            } glyph;
            
            struct {
                float p0_x, p0_y;
                float p1_x, p1_y;
                float p2_x, p2_y;
                float p3_x, p3_y;
                float thickness;
                float color_r, color_g, color_b, color_a;
            } bezier;
            
            struct {
                float pos_x, pos_y;
                float size_x, size_y;
                float radius;
                float strength;
            } blur_rect;
            
            struct {
                float tx, ty;
                float scale;
            } transform;
            
            struct {
                float x, y, w, h;
            } clip;
            
            struct {
                uint32_t path_index;
            } path_cmd;
            
            struct {
                float pos_x, pos_y;
                float size_x, size_y;
                uint32_t gradient_index; 
            } mesh_gradient;
        };
    };

    enum class GradientType {
        Mesh,
        Linear, 
        Radial
    };

    struct GradientPoint {
        Vec2 pos; // Normalized 0..1 relative to rect
        float radius = 0.5f; // Added radius
        ColorF color;
    };

    struct GradientData {
        GradientType type;
        float time_offset = 0.0f;
        std::vector<GradientPoint> points; // Metaballs
    };

    // --- DrawList ---
    struct DrawList {
        DrawList();
        ~DrawList();

        std::vector<DrawCmd> commands;
        std::vector<TextRun> text_runs;
        std::vector<Path> paths;
        std::vector<GradientData> gradients;
        BlendMode current_blend = BlendMode::SourceOver;

        void set_blend_mode(BlendMode mode) { current_blend = mode; }
        
        void add_rect(const Vec2& pos, const Vec2& size, const ColorF& c, float elevation = 0, const Vec2& wobble = {0,0});
        void add_rounded_rect(const Vec2& pos, const Vec2& size, float r, const ColorF& c, 
            float elevation = 0, bool squircle = false, 
            float border_width = 0, const ColorF& border_color = {0,0,0,0},
            float glow_strength = 0, const ColorF& glow_color = {0,0,0,0},
            float blur_strength = 0
        );
        void add_mesh_gradient(const Vec2& pos, const Vec2& size, const GradientData& data);
        void add_circle(const Vec2& center, float r, const ColorF& c, float elevation = 0);
        void add_line(const Vec2& p0, const Vec2& p1, float thickness, const ColorF& c);
        void add_text(const TextRun& run, const ColorF& c);
        void add_text(const Vec2& pos, const Vec2& size, const internal::Vec4& uv, const ColorF& c);
        void add_bezier(const Vec2& p0, const Vec2& p1, const Vec2& p2, const Vec2& p3, float thickness, const ColorF& c);
        void add_blur_rect(const Vec2& pos, const Vec2& size, float r, float strength);
        void push_transform(const Vec2& translate, float scale);
        void pop_transform();
        void push_clip(const Vec2& pos, const Vec2& size);
        void pop_clip();
        void add_path(const Path& path);
        
        void clear();
        size_t size() const { return commands.size(); }
        bool empty() const { return commands.empty(); }
    };

} // namespace internal
} // namespace fanta
