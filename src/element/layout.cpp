#include "element/layout.hpp"
#include "text/text_layout.hpp"
#include "core/contexts_internal.hpp"
#include <algorithm>
#include <cmath>

namespace fanta {
namespace internal {
    using ::fanta::LayoutMode;
    using ::fanta::Align;
    using ::fanta::Justify;
    using ::fanta::Wrap;

std::map<ID, ComputedLayout> LayoutEngine::solve(
    const std::vector<ElementState>& elements,
    const std::map<ID, size_t>& id_map,
    float root_width,
    float root_height
) {
    std::map<ID, ComputedLayout> results;

    // Find root elements (parent == 0)
    for (const auto& el : elements) {
        if (el.parent == 0) {
            float rx = 0, ry = 0;
            if (el.is_node || el.is_popup) { // Popups or Nodes can have absolute pos
                rx = el.node_pos.x + el.persistent.drag_offset.x;
                ry = el.node_pos.y + el.persistent.drag_offset.y;
            }
            solve_node(el, elements, id_map, rx, ry, root_width, root_height, results);
        }
    }

    return results;
}

void LayoutEngine::measure_element(
    const ElementState& el,
    float available_w,
    float available_h,
    float& out_w,
    float& out_h
) {
    // Measure content (Text)
    float text_w = 0;
    float text_h = 0;
    
    // Phase 12.7: Resolve Typography
    TypographyRule rule = GetThemeCtx().current.ResolveFont(el.font_size, el.text_token);

    if (!el.label.empty() && el.has_visible_label) {
        // Default Font ID = 0
        float max_w = el.text_wrap ? available_w : 0;
        Vec2 size = TextLayout::measure(el.label, 0, rule, max_w);
        text_w = size.x;
        text_h = size.y;
    }

    // Resolve width
    if (el.w > 0) {
        out_w = el.w;
    } else {
        // Auto width: Content + Padding
        if (text_w > 0) {
            out_w = text_w + el.p * 2;
        } else {
            // No text: Auto-size container. Use available width if possible.
            out_w = (available_w > 0) ? available_w : (el.p * 2);
        }
    }

    // Resolve height
    if (el.h > 0) {
        out_h = el.h;
    } else {
        // Auto height
        if (text_h > 0) {
            out_h = text_h + el.p * 2;
        } else {
            // No text: Auto-size container.
            // Note: Containers will be shrunk to fit children later in solve_node.
            // For measurement pass, we report available height.
            out_h = (available_h > 0) ? available_h : (el.p * 2);
        }
    }
}

ComputedLayout LayoutEngine::solve_node(
    const ElementState& el,
    const std::vector<ElementState>& elements,
    const std::map<ID, size_t>& id_map,
    float parent_x,
    float parent_y,
    float available_w,
    float available_h,
    std::map<ID, ComputedLayout>& results
) {
    // Measure self
    float my_w, my_h;
    measure_element(el, available_w, available_h, my_w, my_h);

    // Store computed layout
    ComputedLayout computed;
    computed.x = parent_x;
    computed.y = parent_y;
    computed.w = my_w;
    computed.h = my_h;
    results[el.id] = computed;

    // Apply padding for children
    float padding = el.p;
    float content_x = parent_x + padding;
    float content_y = parent_y + padding;
    float content_w = (el.w > 0) ? (el.w - padding * 2) : (available_w - padding * 2);
    float content_h = (el.h > 0) ? (el.h - padding * 2) : (available_h - padding * 2);

    // Layout children if any
    if (!el.children.empty()) {
        float cx = content_x;
        float cy = content_y;
        if (el.is_canvas) {
            cx = el.p;
            cy = el.p;
        }
        Vec2 total_children_size = layout_children(el, el.children, elements, id_map, cx, cy, content_w, content_h, results);
        
        // Update my size if auto
        bool auto_w = (el.w == 0);
        bool auto_h = (el.h == 0);
        
        if (auto_w && !el.is_canvas) {
            results[el.id].w = total_children_size.x + padding * 2;
        }
        if (auto_h && !el.is_canvas) {
            results[el.id].h = total_children_size.y + padding * 2;
        }
    }

    return results[el.id];
}

Vec2 LayoutEngine::layout_children(
    const ElementState& parent,
    const std::vector<ID>& child_ids,
    const std::vector<ElementState>& elements,
    const std::map<ID, size_t>& id_map,
    float content_x,
    float content_y,
    float content_w,
    float content_h,
    std::map<ID, ComputedLayout>& results
) {
    if (child_ids.empty()) return {0,0};

    bool is_row = (parent.layout_mode == LayoutMode::Row);
    float gap = parent.gap;

    // Collect child elements
    std::vector<const ElementState*> children;
    for (ID child_id : child_ids) {
        auto it = id_map.find(child_id);
        if (it != id_map.end()) {
            const auto* el = &elements[it->second];
            if (!el->is_node) children.push_back(el);
        }
    }

    if (children.empty()) return {0,0};

    // Pass 1: Pre-solve children to get true intrinsic sizes
    struct ChildMeasure {
        const ElementState* el;
        float w, h;
    };
    std::vector<ChildMeasure> all_measures;
    
    for (const ElementState* child : children) {
        float avail_w = is_row ? content_w : content_w; // simplified
        float avail_h = is_row ? content_h : content_h;
        
        float mw, mh;
        measure_element(*child, avail_w, avail_h, mw, mh);
        
        // Solve with placeholder to get true height
        auto solved = solve_node(*child, elements, id_map, 0, 0, mw, mh, results);
        
        ChildMeasure m;
        m.el = child;
        m.w = solved.w;
        m.h = solved.h;
        all_measures.push_back(m);
    }

    // Pass 2: Line Splitting using TRUE sizes
    struct Line {
        std::vector<ChildMeasure> measures;
        float main_size = 0;
        float cross_size = 0;
        float total_grow = 0;
    };
    std::vector<Line> lines;
    float main_axis_size = is_row ? content_w : content_h;

    if (parent.wrap_mode != Wrap::NoWrap) {
        Line current_line;
        for (const auto& m : all_measures) {
            float child_main = is_row ? m.w : m.h;
            float child_cross = is_row ? m.h : m.w;
            
            bool wrap_needed = !current_line.measures.empty() && 
                               (current_line.main_size + child_main + gap > main_axis_size);
            
            if (wrap_needed) {
                lines.push_back(current_line);
                current_line = Line();
            }
            
            current_line.measures.push_back(m);
            current_line.main_size += child_main + (current_line.measures.size() > 1 ? gap : 0);
            current_line.cross_size = std::max(current_line.cross_size, child_cross);
            current_line.total_grow += m.el->flex_grow;
        }
        if (!current_line.measures.empty()) lines.push_back(current_line);
    } else {
        Line line;
        for (const auto& m : all_measures) {
            line.measures.push_back(m);
            line.main_size += (is_row ? m.w : m.h) + (line.measures.size() > 1 ? gap : 0);
            line.cross_size = std::max(line.cross_size, is_row ? m.h : m.w);
            line.total_grow += m.el->flex_grow;
        }
        lines.push_back(line);
    }

    if (parent.wrap_mode == Wrap::WrapReverse) std::reverse(lines.begin(), lines.end());

    // Pass 3: Final Position & Grow children
    float total_cross = 0;
    for (size_t i=0; i<lines.size(); ++i) {
        total_cross += lines[i].cross_size;
        if (i > 0) total_cross += gap;
    }

    float cross_axis_size = is_row ? content_h : content_w;
    float current_cross_base = is_row ? content_y : content_x;
    if (parent.align_content == Align::Center) current_cross_base += (cross_axis_size - total_cross) / 2;
    else if (parent.align_content == Align::End) current_cross_base += (cross_axis_size - total_cross);

    float current_cross = current_cross_base;
    float max_main_used = 0;

    for (auto& line : lines) {
        float remaining = main_axis_size - line.main_size;
        float extra_per_grow = (remaining > 0 && line.total_grow > 0) ? remaining / line.total_grow : 0;
        
        float current_main = is_row ? content_x : content_y;
        float actual_gap = gap;
        
        if (parent.justify == Justify::Center) current_main += (main_axis_size - line.main_size) / 2;
        else if (parent.justify == Justify::End) current_main += (main_axis_size - line.main_size);
        else if (parent.justify == Justify::SpaceBetween && line.measures.size() > 1) {
            actual_gap = (main_axis_size - (line.main_size - (line.measures.size()-1)*gap)) / (line.measures.size()-1);
        } else if (parent.justify == Justify::SpaceAround) {
            float space = (main_axis_size - (line.main_size - (line.measures.size()-1)*gap)) / line.measures.size();
            current_main += space / 2;
            actual_gap = space;
        }

        // Phase 18: RTL Flip for Row
        bool do_rtl = is_row && parent.is_rtl;

        for (auto& m : line.measures) {
            float extra = m.el->flex_grow * extra_per_grow;
            if (is_row) m.w += extra; else m.h += extra;

            float cx, cy;
            float m_cross = is_row ? m.h : m.w;
            float child_cross_pos = current_cross;
            if (parent.align == Align::Center) child_cross_pos += (line.cross_size - m_cross) / 2;
            else if (parent.align == Align::End) child_cross_pos += (line.cross_size - m_cross);

            if (is_row) { 
                cx = current_main; 
                if (do_rtl) {
                    // Flip X relative to parent content area
                    cx = (content_x + content_w) - (current_main - content_x) - m.w;
                }
                cy = child_cross_pos; 
            }
            else { cx = child_cross_pos; cy = current_main; }

            // Final real solve with correct positions
            solve_node(*m.el, elements, id_map, cx, cy, m.w, m.h, results);

            current_main += (is_row ? m.w : m.h) + actual_gap;
        }
        max_main_used = std::max(max_main_used, current_main - (is_row ? content_x : content_y));
        current_cross += line.cross_size + gap;
    }

    float final_cross_used = current_cross - current_cross_base - (lines.empty() ? 0 : gap);
    if (is_row) return {max_main_used, final_cross_used};
    else return {final_cross_used, max_main_used};
}

} // namespace internal
} // namespace fanta
