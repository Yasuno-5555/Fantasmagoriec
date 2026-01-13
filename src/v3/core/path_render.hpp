// Fantasmagorie v3 - Path Rendering Implementation
// Tessellates paths and renders as polylines
#pragma once

#include "../core/path.hpp"
#include "../core/render_context.hpp"

namespace fanta {

// ============================================================================
// Path Renderer
// ============================================================================

class PathRenderer {
public:
    void init(RenderContext& ctx);
    
    // Render stroked path
    void stroke(RenderContext& ctx, const Path& path, 
                Color color, float thickness, float zoom = 1.0f);
    
    // Render filled path (triangulates)
    void fill(RenderContext& ctx, const Path& path,
              Color color, float zoom = 1.0f);
    
private:
    ShaderProgram* shader_polyline_ = nullptr;
    uint32_t vao_ = 0;
    uint32_t vbo_ = 0;
    std::vector<float> vertex_buffer_;
};

// ============================================================================
// PathStroke/PathFill Command Implementations
// ============================================================================

namespace cmd {

inline void PathStroke::execute(RenderContext& ctx) const {
    if (!ctx.shader_line || !ctx.shader_line->valid()) return;
    
    // Tessellate path
    auto tessellated = tessellate_path(path, ctx.zoom_level);
    if (tessellated.points.size() < 2) return;
    
    // Render each segment
    for (size_t i = 0; i < tessellated.points.size() - 1; ++i) {
        Vec2 from = tessellated.points[i];
        Vec2 to = tessellated.points[i + 1];
        
        // Skip degenerate segments
        if ((to - from).length() < 0.001f) continue;
        
        ctx.shader_line->use();
        ctx.shader_line->set_vec2("u_resolution", (float)ctx.viewport_width, (float)ctx.viewport_height);
        ctx.shader_line->set_float("u_dpr", ctx.device_pixel_ratio);
        ctx.shader_line->set_vec2("u_from", from.x, from.y);
        ctx.shader_line->set_vec2("u_to", to.x, to.y);
        ctx.shader_line->set_float("u_thickness", thickness);
        ctx.shader_line->set_vec4("u_color", color.r, color.g, color.b, color.a);
        ctx.shader_line->set_int("u_cap_style", cap == Cap::Round ? 1 : (cap == Cap::Square ? 2 : 0));
        
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
}

inline void PathFill::execute(RenderContext& ctx) const {
    // TODO: Implement polygon triangulation for filled paths
    // For now, just stroke
    (void)ctx;
}

} // namespace cmd

} // namespace fanta
