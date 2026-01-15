#include "render_tree.hpp"
#include "../text/text_layout.hpp"
#include "../core/contexts_internal.hpp"
#include "../core/types_internal.hpp" // Added explicit include
#include <algorithm>
#include <cmath>

namespace fanta {
namespace internal {

static void resolve_interaction(ElementState& el, const ComputedLayout& layout, InputContext& input, float scroll_tx, float scroll_ty, float world_scale) { 
    // Transform mouse to local space of the current world transform
    float mx = (input.mouse_x + scroll_tx) / world_scale;
    float my = (input.mouse_y + scroll_ty) / world_scale;
    
    bool hit = (mx >= layout.x && mx < layout.x + layout.w &&
                my >= layout.y && my < layout.y + layout.h);
    
    el.is_hovered = hit;
    el.is_active = el.is_hovered && input.mouse_down;
    
    if (el.is_disabled) {
        el.is_hovered = false;
        el.is_active = false;
        el.is_focusable = false;
    }

    // Phase 6.2: Event Consumption (Bubbling Control)
    // If a child already claimed focus or consumed the click, we shouldn't steal it here.
    // However, RenderTree is Top-Down, so parents see events FIRST. 
    // This is problematic for traditional "Capture/Bubble".
    // Usually, we want to resolve BOTTOM-UP for clicks.
    
    // For now, since we are doing a single pass top-down:
    // We'll letchildren override. 
    // Actually, in EndFrame, we call RenderTree which recurses.
    
    if (el.is_focusable && el.is_active && input.state.is_just_pressed()) {
        input.focused_id = el.id;
        
        // Phase 5.1: Click to Select
        if (!IsKeyDown(0x10)) { // No Shift: clear others
            for (auto& store_el : GetStore().elements) store_el.is_selected = false;
        }
        for (auto& store_el : GetStore().elements) {
            if (store_el.id == el.id) store_el.is_selected = true;
        }
    }
    el.is_focused = (input.focused_id == el.id);
}

static void apply_animation(const ElementState& el, ComputedLayout& layout, StateStore& store, float dt, InputContext& input, ColorF& draw_bg, ColorF& draw_text, float& draw_elevation) {
    // ... (apply_animation logic is mostly about target resolve, but it modifies layout.x/y)
    // It uses layout.x/y as absolute values. 
    auto& p_states = store.persistent_states;
    auto& p_data = p_states[el.id];
    
    float target_elevation = el.elevation;
    float target_scale = 1.0f;
    
    auto resolve = [&](const StateColor& sc) -> ColorF {
        return GetThemeCtx().current.Resolve(sc.rgba, sc.token, sc.is_semantic);
    };

    if (el.is_active && el.active_style) {
        if (el.active_style->bg) draw_bg = resolve(*el.active_style->bg);
        if (el.active_style->text) draw_text = resolve(*el.active_style->text);
        target_elevation += el.active_style->elevation_delta;
        target_scale += el.active_style->scale_delta;
    } else if (el.is_hovered && el.hover_style) {
        if (el.hover_style->bg) draw_bg = resolve(*el.hover_style->bg);
        if (el.hover_style->text) draw_text = resolve(*el.hover_style->text);
        target_elevation += el.hover_style->elevation_delta;
        target_scale += el.hover_style->scale_delta;
    } else if (el.is_focused && el.focus_style) {
        if (el.focus_style->bg) draw_bg = resolve(*el.focus_style->bg);
        if (el.focus_style->text) draw_text = resolve(*el.focus_style->text);
        target_elevation += el.focus_style->elevation_delta;
        target_scale += el.focus_style->scale_delta;
    }

    if (el.is_disabled) {
        draw_bg.a *= 0.5f;
        draw_text.a *= 0.5f;
    }

    if (!p_data.elevation.initialized) p_data.elevation.init(el.elevation);
    p_data.elevation.target = target_elevation;
    p_data.elevation.update(dt, SpringConfig::Default());
    draw_elevation = p_data.elevation.value;
    
    if (!p_data.scale.initialized) p_data.scale.init(1.0f);
    p_data.scale.target = target_scale;
    p_data.scale.update(dt, SpringConfig::Stiff());
    
    float current_scale = p_data.scale.value;
    
    if (std::abs(current_scale - 1.0f) > 0.001f) {
        float cx = layout.x + layout.w * 0.5f;
        float cy = layout.y + layout.h * 0.5f;
        layout.w *= current_scale;
        layout.h *= current_scale;
        layout.x = cx - layout.w * 0.5f;
        layout.y = cy - layout.h * 0.5f;
    }
}

static void emit_primitives(const ElementState& el, const ComputedLayout& layout, DrawList& dl, const ColorF& draw_bg, const ColorF& draw_text, float draw_elevation, float viewport_tx, float viewport_ty) {
    // subtract viewport_tx (camera) from absolute layout.x
    if (draw_bg.a > 0) {
        if (el.is_line) {
            float mid_y = layout.y + layout.h * 0.5f;
            dl.add_line(
                {layout.x - viewport_tx, mid_y - viewport_ty},
                {layout.x + layout.w - viewport_tx, mid_y - viewport_ty},
                layout.h,
                draw_bg
            );
        } else if (el.corner_radius > 0) {
            float min_dim = std::min(layout.w, layout.h);
            float max_radius = min_dim * 0.5f;
            
            if (el.corner_radius >= max_radius && std::abs(layout.w - layout.h) < 0.1f) {
                dl.add_circle(
                    {layout.x + layout.w * 0.5f - viewport_tx, layout.y + layout.h * 0.5f - viewport_ty},
                    max_radius,
                    draw_bg,
                    draw_elevation
                );
            } else {
                dl.add_rounded_rect(
                    {layout.x - viewport_tx, layout.y - viewport_ty},
                    {layout.w, layout.h},
                    el.corner_radius,
                    draw_bg,
                    draw_elevation
                );
            }
        } else {
            dl.add_rect(
                {layout.x - viewport_tx, layout.y - viewport_ty},
                {layout.w, layout.h},
                draw_bg,
                draw_elevation
            );
        }
    }
    
    float resolved_font_size = GetThemeCtx().current.ResolveFont(el.font_size, el.text_token);

    if (!el.label.empty() && el.has_visible_label) {
        float max_w = el.text_wrap ? layout.w - el.p * 2 : 0;
        TextRun run = TextLayout::transform_wrapped(el.label, 0, resolved_font_size, max_w, 0);
        if (!run.empty()) {
            float content_w = run.bounds.w;
            float content_h = run.bounds.h;
            
            // Vertical Centering: Center text within the layout box (minus padding context if appropriate)
            // For now, we center strictly within the content area.
            run.offset_x = layout.x + el.p - viewport_tx;
            
            // Vertical Centering within layout.h with Visual Bias
            // (H - h) / 2 - 2.0f for "Professional" look as requested
            float target_y = layout.y + (layout.h - content_h) / 2.0f - 2.0f;
            
            // Safety: Don't go above padding top unless box is too small
            if (target_y < layout.y + el.p) target_y = layout.y + el.p;
            
            run.offset_y = target_y - viewport_ty;
            
            dl.add_text(run, draw_text);
        }
    }

    if (el.is_wire) {
        dl.add_bezier(
            {el.wire_p0.x - viewport_tx, el.wire_p0.y - viewport_ty}, 
            {el.wire_p1.x - viewport_tx, el.wire_p1.y - viewport_ty},
            {el.wire_p2.x - viewport_tx, el.wire_p2.y - viewport_ty}, 
            {el.wire_p3.x - viewport_tx, el.wire_p3.y - viewport_ty},
            el.wire_thickness, draw_text
        );
    }

    // Phase 5.1: Selection Visualization
    if (el.is_selected) {
        float lx = layout.x - viewport_tx;
        float ly = layout.y - viewport_ty;
        float lw = layout.w;
        float lh = layout.h;
        fanta::internal::ColorF sel_color = {0.2f, 0.4f, 1.0f, 0.7f}; // Blue
        // Draw slightly larger outline
        float padding = 2.0f;
        dl.add_line({lx - padding, ly - padding}, {lx + lw + padding, ly - padding}, 2.0f, sel_color);
        dl.add_line({lx + lw + padding, ly - padding}, {lx + lw + padding, ly + lh + padding}, 2.0f, sel_color);
        dl.add_line({lx + lw + padding, ly + lh + padding}, {lx - padding, ly + lh + padding}, 2.0f, sel_color);
        dl.add_line({lx - padding, ly + lh + padding}, {lx - padding, ly - padding}, 2.0f, sel_color);
    }

    // Phase 5.2: Wire Rendering
    if (el.is_wire) {
        // Apply layout offset (wires usually absolute, but parent transform applies)
        float tx = layout.x - viewport_tx;
        float ty = layout.y - viewport_ty;
        
        // Wire points are typically relative to the element (or if element is canvas, relative to canvas)
        // If element is just a container for the wire, we use its position.
        // Assuming wire points are local to the element's layout box:
        Vec2 p0 = {tx + el.wire_p0.x, ty + el.wire_p0.y};
        Vec2 p1 = {tx + el.wire_p1.x, ty + el.wire_p1.y};
        Vec2 p2 = {tx + el.wire_p2.x, ty + el.wire_p2.y};
        Vec2 p3 = {tx + el.wire_p3.x, ty + el.wire_p3.y};
        
        // Use text_color as wire color for now, or fallback to default
        fanta::internal::ColorF c = {el.text_color.rgba};
        if (c.a == 0) c = {0.8f, 0.8f, 0.8f, 1.0f}; // Default white-ish
        
        dl.add_bezier(p0, p1, p2, p3, el.wire_thickness, c);
    }
        ColorF hit_color = {1.0f, 0.0f, 1.0f, 0.4f}; // Magenta 40%
        if (el.is_hovered) hit_color = {1.0f, 1.0f, 0.0f, 0.6f}; // Yellow 60%
        if (el.is_active) hit_color = {1.0f, 1.0f, 1.0f, 0.8f};  // White 80%

        dl.add_rect(
            {layout.x - viewport_tx, layout.y - viewport_ty},
            {layout.w, layout.h},
            {0,0,0,0}, // Transparent fill
            draw_elevation + 0.1f // Slightly above content
        );
        // We don't have a direct "outline-only" rect in dl yet, 
        // but we can use add_rect with a very small offset or just use lines.
        // Actually, drawlist.hpp might have line primitive.
        float lx = layout.x - viewport_tx;
        float ly = layout.y - viewport_ty;
        float lw = layout.w;
        float lh = layout.h;
        dl.add_line({lx, ly}, {lx + lw, ly}, 1.0f, hit_color);
        dl.add_line({lx + lw, ly}, {lx + lw, ly + lh}, 1.0f, hit_color);
        dl.add_line({lx + lw, ly + lh}, {lx, ly + lh}, 1.0f, hit_color);
        dl.add_line({lx, ly + lh}, {lx, ly}, 1.0f, hit_color);
}

static void apply_container_transform(
    const fanta::internal::ElementState& el, 
    const fanta::internal::ComputedLayout& layout, 
    fanta::internal::DrawList& dl, 
    fanta::internal::StateStore& store, 
    fanta::internal::RuntimeState& runtime, 
    fanta::internal::InputContext& input, 
    float viewport_tx, float viewport_ty, float scroll_tx, float scroll_ty, float world_scale, 
    float& next_vtx, float& next_vty, float& next_stx, float& next_sty, float& next_scale, 
    bool& pushed_transform, bool& pushed_clip
) {
    next_vtx = viewport_tx;
    next_vty = viewport_ty;
    next_stx = scroll_tx;
    next_sty = scroll_ty;
    next_scale = world_scale;
    pushed_transform = false;
    pushed_clip = false;

    if (el.is_scrollable) {
        // Hardware clip uses current screen-space bounds
        dl.push_clip({layout.x - viewport_tx, layout.y - viewport_ty}, {layout.w, layout.h});
        pushed_clip = true;

        auto& p_data = store.persistent_states[el.id];
        if (!p_data.scroll_y.initialized) p_data.scroll_y.init(0);
        if (!p_data.scroll_x.initialized) p_data.scroll_x.init(0);

        // Phase 2.2: Elastic Bounds calculation
        float max_scroll_y = 0;
        float max_scroll_x = 0;
        for (fanta::internal::ID child_id : el.children) {
            auto it = store.persistent_states.find(child_id); // This is wrong, should use layout_results
        }
        // Actually, we need to find the max Y of children from layout_results
        for (fanta::internal::ID child_id : el.children) {
            if (runtime.layout_results.count(child_id)) {
                const auto& cl = runtime.layout_results.at(child_id);
                max_scroll_y = std::max(max_scroll_y, cl.y + cl.h - layout.y);
                max_scroll_x = std::max(max_scroll_x, cl.x + cl.w - layout.x);
            }
        }
        
        float scroll_range_y = std::max(0.0f, max_scroll_y - layout.h);
        float scroll_range_x = std::max(0.0f, max_scroll_x - layout.w);
        
        p_data.scroll_y.set_bounds(0, scroll_range_y);
        p_data.scroll_x.set_bounds(0, scroll_range_x);

        if (el.is_hovered) {
            if (el.scroll_vertical) p_data.scroll_y.target -= input.mouse_wheel * 40.0f;
            if (el.scroll_horizontal) p_data.scroll_x.target -= input.mouse_wheel * 40.0f;
        }

        // Clamp target slightly to avoid runaway scroll, but allow elastic overshoot in AnimState
        p_data.scroll_y.target = std::clamp(p_data.scroll_y.target, -50.0f, scroll_range_y + 50.0f);
        p_data.scroll_x.target = std::clamp(p_data.scroll_x.target, -50.0f, scroll_range_x + 50.0f);

        p_data.scroll_y.update(runtime.dt, SpringConfig::Default());
        p_data.scroll_x.update(runtime.dt, SpringConfig::Default());

        float sx = p_data.scroll_x.value;
        float sy = p_data.scroll_y.value;

        // Visual scroll via hardware transform
        dl.push_transform({-sx, -sy}, 1.0f); 
        pushed_transform = true;

        next_stx = scroll_tx + sx;
        next_sty = scroll_ty + sy;
    } else if (el.is_canvas) {
        dl.push_clip({layout.x - viewport_tx, layout.y - viewport_ty}, {layout.w, layout.h});
        pushed_clip = true;
        
        float px = el.canvas_pan.x;
        float py = el.canvas_pan.y;
        float pz = el.canvas_zoom;
        
        dl.push_transform({px, py}, pz);
        pushed_transform = true;
        
        // Canvas is special: it's a camera.
        // Rendering: children at (x,y) are shifted by (px, py) and scaled by pz.
        // If we push transform {px, py} scale pz, then children drawn at (x-vtx) will be transformed.
        // Wait, if we use hardware transform for Pan/Zoom, we should NOT adjust vtx for rendering children.
        // But for hit-testing, children are moved.
        next_stx = scroll_tx - px / pz; // Mouse to content space
        next_sty = scroll_ty - py / pz;
        next_scale = world_scale * pz;
        
        // vtx for rendering? If hardware does it, vtx stays.
    }
}

void RenderTree(
    fanta::internal::ElementState& el,
    const std::map<fanta::internal::ID, fanta::internal::ComputedLayout>& layout_results,
    const std::vector<fanta::internal::ElementState>& all_elements,
    const std::map<fanta::internal::ID, size_t>& id_map,
    fanta::internal::DrawList& dl,
    fanta::internal::RuntimeState& runtime,
    fanta::internal::InputContext& input,
    fanta::internal::StateStore& store,
    float viewport_tx, float viewport_ty, float scroll_tx, float scroll_ty, float world_scale,
    bool inside_canvas
) {
    if (layout_results.find(el.id) == layout_results.end()) return; 
    fanta::internal::ComputedLayout calculated_layout = layout_results.at(el.id);

    if (el.is_popup && !inside_canvas) { 
         runtime.overlay_ids.push_back(el.id);
         return; 
    }
    
    resolve_interaction(el, calculated_layout, input, scroll_tx, scroll_ty, world_scale);

    ColorF draw_bg = GetThemeCtx().current.Resolve(el.bg_color.rgba, el.bg_color.token, el.bg_color.is_semantic);
    ColorF draw_text = GetThemeCtx().current.Resolve(el.text_color.rgba, el.text_color.token, el.text_color.is_semantic);
    float draw_elevation = 0;

    apply_animation(el, calculated_layout, store, runtime.dt, input, draw_bg, draw_text, draw_elevation);
    
    emit_primitives(el, calculated_layout, dl, draw_bg, draw_text, draw_elevation, viewport_tx, viewport_ty);

    float next_vtx, next_vty, next_stx, next_sty, next_scale;
    bool pushed_transform = false;
    bool pushed_clip = false;

    // Standard elements don't push transform (already absolute)
    // ONLY Scroll/Canvas push transforms.
    next_vtx = viewport_tx;
    next_vty = viewport_ty;
    next_stx = scroll_tx;
    next_sty = scroll_ty;
    next_scale = world_scale;

    apply_container_transform(el, calculated_layout, dl, store, runtime, input, viewport_tx, viewport_ty, scroll_tx, scroll_ty, world_scale, next_vtx, next_vty, next_stx, next_sty, next_scale, pushed_transform, pushed_clip);

    float scroll_y = 0;
    if (el.is_scrollable) {
        scroll_y = store.persistent_states[el.id].scroll_y.value;
    }

    for (fanta::internal::ID child_id : el.children) {
        auto it = id_map.find(child_id);
        if (it == id_map.end()) continue;
        fanta::internal::ElementState& child_el = const_cast<fanta::internal::ElementState&>(all_elements[it->second]);
        
        // Virtualization (using scroll_y for culling)
        if (el.is_scrollable && el.scroll_vertical) {
            if (layout_results.find(child_id) != layout_results.end()) {
                const auto& child_layout = layout_results.at(child_id);
                float child_top_rel = child_layout.y - calculated_layout.y - scroll_y;
                float child_bottom_rel = child_top_rel + child_layout.h;
                
                if (child_bottom_rel < 0 || child_top_rel > calculated_layout.h) {
                    continue; 
                }
            }
        }
        
        RenderTree(child_el, layout_results, all_elements, id_map, dl, runtime, input, store, next_vtx, next_vty, next_stx, next_sty, next_scale, inside_canvas || el.is_canvas);
    }

    if (pushed_transform) dl.pop_transform();
    if (pushed_clip) dl.pop_clip();
}

} // namespace internal
} // namespace fanta
