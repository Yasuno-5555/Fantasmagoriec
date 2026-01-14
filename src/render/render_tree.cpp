#include "render_tree.hpp"
#include "../text/text_layout.hpp"
#include <algorithm>
#include <cmath>

namespace fanta {
namespace internal {

static bool resolve_interaction(const ElementState& el, const ComputedLayout& layout, InputContext& input, float world_tx, float world_ty, float world_scale) {
    // Transform mouse to local space of the current world transform
    float mx = (input.mouse_x - world_tx) / world_scale;
    float my = (input.mouse_y - world_ty) / world_scale;
    return (mx >= layout.x && mx < layout.x + layout.w &&
            my >= layout.y && my < layout.y + layout.h);
}

static void apply_animation(const ElementState& el, ComputedLayout& layout, StateStore& store, float dt, bool hovered, InputContext& input, ColorF& draw_color, float& draw_elevation) {
    if (el.elevation > 0) {
        auto& p_states = store.persistent_states;
        auto& p_data = p_states[el.id];
        
        if (!p_data.elevation.initialized) p_data.elevation.init(el.elevation);
        
        float target = el.elevation;
        if (hovered) {
             target += 10.0f; // Lift Up effect
             
             // Highlight color
             draw_color.r = std::min(1.0f, draw_color.r * 1.1f + 0.05f);
             draw_color.g = std::min(1.0f, draw_color.g * 1.1f + 0.05f);
             draw_color.b = std::min(1.0f, draw_color.b * 1.1f + 0.05f);
        }
        
        p_data.elevation.update(dt, SpringConfig::Default());
        draw_elevation = p_data.elevation.value;
        
        // Scale Animation (Click)
        float target_scale = 1.0f;
        if (hovered && input.state.is_pressed()) {
             target_scale = 0.95f; // Press down
        }
        
        if (!p_data.scale.initialized) p_data.scale.init(1.0f);
        p_data.scale.target = target_scale;
        p_data.scale.update(dt, SpringConfig::Stiff()); // Fast response
        
        float current_scale = p_data.scale.value;
        
        // Apply Scale to Layout (Centered)
        if (std::abs(current_scale - 1.0f) > 0.001f) {
            float cx = layout.x + layout.w * 0.5f;
            float cy = layout.y + layout.h * 0.5f;
            layout.w *= current_scale;
            layout.h *= current_scale;
            layout.x = cx - layout.w * 0.5f;
            layout.y = cy - layout.h * 0.5f;
        }
    }
}

static void emit_primitives(const ElementState& el, const ComputedLayout& layout, DrawList& dl, const ColorF& draw_color, float draw_elevation, float world_tx, float world_ty) {
    // Draw Background
    if (draw_color.a > 0) {
        if (el.is_line) {
            float mid_y = layout.y + layout.h * 0.5f;
            dl.add_line(
                {layout.x - world_tx, mid_y - world_ty},
                {layout.x + layout.w - world_tx, mid_y - world_ty},
                layout.h,
                draw_color
            );
        } else if (el.corner_radius > 0) {
            float min_dim = std::min(layout.w, layout.h);
            float max_radius = min_dim * 0.5f;
            
            if (el.corner_radius >= max_radius && std::abs(layout.w - layout.h) < 0.1f) {
                dl.add_circle(
                    {layout.x + layout.w * 0.5f - world_tx, layout.y + layout.h * 0.5f - world_ty},
                    max_radius,
                    draw_color,
                    draw_elevation
                );
            } else {
                dl.add_rounded_rect(
                    {layout.x - world_tx, layout.y - world_ty},
                    {layout.w, layout.h},
                    el.corner_radius,
                    draw_color,
                    draw_elevation
                );
            }
        } else {
            dl.add_rect(
                {layout.x - world_tx, layout.y - world_ty},
                {layout.w, layout.h},
                draw_color,
                draw_elevation
            );
        }
    }
    
    // Draw Text
    if (!el.label.empty()) {
        TextRun run = TextLayout::transform(el.label, 0, el.font_size);
        if (!run.empty()) {
            float content_w = run.bounds.w;
            float content_h = run.bounds.h;
            
            float target_x = layout.x + (layout.w - content_w) * 0.5f - world_tx;
            float target_y = layout.y + (layout.h - content_h) * 0.5f - world_ty;
            
            float offset_x = target_x - run.bounds.x;
            float offset_y = target_y - run.bounds.y;
            
            for (auto& q : run.quads) {
                q.x0 += offset_x; q.x1 += offset_x;
                q.y0 += offset_y; q.y1 += offset_y;
            }
            run.bounds.x += offset_x;
            run.bounds.y += offset_y;
            
            dl.add_text(run, el.text_color);
        }
    }

    // Phase 7: Wire Primitives
    if (el.is_wire) {
        dl.add_bezier(
            {el.wire_p0.x - world_tx, el.wire_p0.y - world_ty}, 
            {el.wire_p1.x - world_tx, el.wire_p1.y - world_ty},
            {el.wire_p2.x - world_tx, el.wire_p2.y - world_ty}, 
            {el.wire_p3.x - world_tx, el.wire_p3.y - world_ty},
            el.wire_thickness, el.text_color
        );
    }
}

static void apply_container_transform(
    const ElementState& el, 
    const ComputedLayout& layout, 
    DrawList& dl, 
    StateStore& store, 
    RuntimeState& runtime, 
    InputContext& input, 
    bool hovered, 
    float world_tx, float world_ty, float world_scale,
    float& next_tx, float& next_ty, float& next_scale, 
    bool& pushed_transform, bool& pushed_clip
) {
    next_tx = world_tx;
    next_ty = world_ty;
    next_scale = world_scale;

    if (el.is_scrollable_v || el.is_scrollable_h) {
        dl.push_clip({layout.x - world_tx, layout.y - world_ty}, {layout.w, layout.h});
        pushed_clip = true;

        auto& p_data = store.persistent_states[el.id];
        if (!p_data.scroll_y.initialized) p_data.scroll_y.init(0);
        if (!p_data.scroll_x.initialized) p_data.scroll_x.init(0);

        if (hovered) {
            if (el.is_scrollable_v) p_data.scroll_y.target -= input.mouse_wheel * 40.0f;
            if (el.is_scrollable_h) p_data.scroll_x.target -= input.mouse_wheel * 40.0f;
        }

        p_data.scroll_y.update(runtime.dt, SpringConfig::Default());
        p_data.scroll_x.update(runtime.dt, SpringConfig::Default());

        float sx = p_data.scroll_x.value;
        float sy = p_data.scroll_y.value;

        dl.push_transform({layout.x - world_tx - sx, layout.y - world_ty - sy}, 1.0f);
        pushed_transform = true;

        next_tx = world_tx + (layout.x - world_tx - sx) * world_scale;
        next_ty = world_ty + (layout.y - world_ty - sy) * world_scale;
        next_scale = world_scale;

    } else if (el.is_canvas) {
        dl.push_clip({layout.x - world_tx, layout.y - world_ty}, {layout.w, layout.h});
        pushed_clip = true;

        float px = el.canvas_pan.x;
        float py = el.canvas_pan.y;
        float pz = el.canvas_zoom;

        dl.push_transform({layout.x - world_tx + px, layout.y - world_ty + py}, pz);
        pushed_transform = true;

        next_tx = world_tx + (layout.x - world_tx + px) * world_scale;
        next_ty = world_ty + (layout.y - world_ty + py) * world_scale;
        next_scale = world_scale * pz;
    }
}

void RenderTree(
    const ElementState& el,
    const std::map<ID, ComputedLayout>& layout_results,
    const std::vector<ElementState>& all_elements,
    const std::map<ID, size_t>& id_map,
    DrawList& dl,
    RuntimeState& runtime,
    InputContext& input,
    StateStore& store,
    float world_tx, float world_ty, float world_scale,
    bool is_overlay
) {
    auto it = layout_results.find(el.id);
    if (it == layout_results.end()) return;

    ComputedLayout layout = it->second;
    
    // Phase 8: Popup handling
    if (el.is_popup && !is_overlay) { 
         runtime.overlay_ids.push_back(el.id);
         return; 
    }
    
    // Interaction
    bool hovered = resolve_interaction(el, layout, input, world_tx, world_ty, world_scale);

    // Animation
    ColorF draw_color = el.bg_color;
    float draw_elevation = el.elevation;
    apply_animation(el, layout, store, runtime.dt, hovered, input, draw_color, draw_elevation);
    
    // Emit Primitives
    emit_primitives(el, layout, dl, draw_color, draw_elevation, world_tx, world_ty);

    // Container Transform
    bool pushed_transform = false;
    bool pushed_clip = false;
    float next_tx, next_ty, next_scale;
    apply_container_transform(el, layout, dl, store, runtime, input, hovered, world_tx, world_ty, world_scale, next_tx, next_ty, next_scale, pushed_transform, pushed_clip);

    // Render Children
    for (ID child_id : el.children) {
        auto child_it = id_map.find(child_id);
        if (child_it == id_map.end()) continue;
        const auto& child = all_elements[child_it->second];
        RenderTree(child, layout_results, all_elements, id_map, dl, runtime, input, store, next_tx, next_ty, next_scale, is_overlay);
    }

    if (pushed_transform) dl.pop_transform();
    if (pushed_clip) dl.pop_clip();
}

} // namespace internal
} // namespace fanta
