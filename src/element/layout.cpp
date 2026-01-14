#include "element/layout.hpp"
#include "text/text_layout.hpp"
#include <algorithm>
#include <cmath>

namespace fanta {
namespace internal {

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
                rx = el.node_pos.x;
                ry = el.node_pos.y;
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
    if (!el.label.empty()) {
        // Default Font ID = 0
        TextLayout::measure(el.label, 0, el.font_size, text_w, text_h);
    }

    // Resolve width
    if (el.w > 0) {
        out_w = el.w;
    } else {
        // Auto width: Content + Padding (clamped to available?)
        // Flexbox usually shrinks if needed, but for "Auto" we default to content size
        out_w = text_w + el.p * 2;
        // Optionally clamp to available_w if not infinite? 
        // For simplicity: content size wins unless strictly constrained?
        // Let's implement basic shrink later.
    }

    // Resolve height
    if (el.h > 0) {
        out_h = el.h;
    } else {
        // Auto height
        // Ensure at least some height if text exists, otherwise default
        if (text_h > 0) {
            out_h = text_h + el.p * 2;
        } else {
            // Auto-size container: Default to available space (Flex-like behavior)
            // instead of arbitrary 40px.
            out_h = available_h > 0 ? available_h : 0.0f;
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

    // Apply padding
    float padding = el.p;
    float content_x = parent_x + padding;
    float content_y = parent_y + padding;
    float content_w = my_w - padding * 2;
    float content_h = my_h - padding * 2;

    // Store computed layout
    ComputedLayout computed;
    computed.x = parent_x;
    computed.y = parent_y;
    computed.w = my_w;
    computed.h = my_h;
    results[el.id] = computed;

    // Layout children if any
    if (!el.children.empty()) {
        float cx = content_x;
        float cy = content_y;
        if (el.is_canvas) {
            cx = el.p;
            cy = el.p;
        }
        layout_children(el, el.children, elements, id_map, cx, cy, content_w, content_h, results);
    }

    return computed;
}

void LayoutEngine::layout_children(
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
    if (child_ids.empty()) return;

    bool is_row = (parent.layout_mode == LayoutMode::Row);
    float gap = parent.gap;

    // Collect child elements
    std::vector<const ElementState*> children;
    std::vector<const ElementState*> absolute_nodes;
    for (ID child_id : child_ids) {
        auto it = id_map.find(child_id);
        if (it != id_map.end()) {
            const auto* el = &elements[it->second];
            if (el->is_node) {
                absolute_nodes.push_back(el);
            } else {
                children.push_back(el);
            }
        }
    }

    // Process absolute nodes first or last? User says "skip layout engine", but we need solve_node.
    for (const auto* node : absolute_nodes) {
        float nw, nh;
        measure_element(*node, content_w, content_h, nw, nh);
        solve_node(*node, elements, id_map, content_x + node->node_pos.x, content_y + node->node_pos.y, nw, nh, results);
    }

    if (children.empty()) return;

    // Measure all children
    struct ChildMeasure {
        const ElementState* el;
        float w, h;
        bool is_flex_main; // Is main-axis size flexible?
    };
    std::vector<ChildMeasure> measures;
    
    float main_axis_size = is_row ? content_w : content_h;
    float cross_axis_size = is_row ? content_h : content_w;
    
    // Calculate total gap space
    float total_gap = gap * (children.size() - 1);
    float available_main = main_axis_size - total_gap;

    // Measure fixed-size children
    float total_fixed_main = 0;
    int flex_count = 0;
    
    for (const ElementState* child : children) {
        ChildMeasure m;
        m.el = child;
        
        // Check if main-axis is flexible
        m.is_flex_main = (is_row && child->w == 0) || (!is_row && child->h == 0);
        
        float available_w_for_child = is_row ? available_main : cross_axis_size;
        float available_h_for_child = is_row ? cross_axis_size : available_main;
        
        // For stretch align, use full cross-axis size
        if (parent.align == Align::Stretch) {
            if (is_row && child->h == 0) {
                available_h_for_child = cross_axis_size;
            } else if (!is_row && child->w == 0) {
                available_w_for_child = cross_axis_size;
            }
        }
        
        measure_element(*child, available_w_for_child, available_h_for_child, m.w, m.h);
        
        float child_main = is_row ? m.w : m.h;
        if (m.is_flex_main) {
            flex_count++;
        } else {
            total_fixed_main += child_main;
        }
        
        measures.push_back(m);
    }

    // Distribute remaining space to flexible children
    float remaining_main = available_main - total_fixed_main;
    float flex_size = (flex_count > 0 && remaining_main > 0) ? remaining_main / flex_count : 0;

    // Update flexible children sizes
    for (auto& m : measures) {
        if (m.is_flex_main) {
            if (is_row) {
                m.w = std::max(0.0f, flex_size);
            } else {
                m.h = std::max(0.0f, flex_size);
            }
        }
        
        // Apply stretch to cross-axis
        if (parent.align == Align::Stretch) {
            if (is_row && m.el->h == 0) {
                m.h = cross_axis_size;
            } else if (!is_row && m.el->w == 0) {
                m.w = cross_axis_size;
            }
        }
    }

    // Apply justify-content
    float main_start = content_x;
    float main_pos = main_start;
    
    if (is_row) {
        if (parent.justify == Justify::Center) {
            float total_children_main = total_fixed_main + flex_size * flex_count;
            main_pos = content_x + (content_w - total_children_main - total_gap) / 2;
        } else if (parent.justify == Justify::End) {
            float total_children_main = total_fixed_main + flex_size * flex_count;
            main_pos = content_x + content_w - total_children_main - total_gap;
        } else if (parent.justify == Justify::SpaceBetween && children.size() > 1) {
            float space = (content_w - total_fixed_main - flex_size * flex_count) / (children.size() - 1);
            gap = space;
            main_pos = content_x;
        } else if (parent.justify == Justify::SpaceAround && children.size() > 0) {
            float space = (content_w - total_fixed_main - flex_size * flex_count) / children.size();
            gap = space;
            main_pos = content_x + space / 2;
        }
    } else {
        if (parent.justify == Justify::Center) {
            float total_children_main = total_fixed_main + flex_size * flex_count;
            main_pos = content_y + (content_h - total_children_main - total_gap) / 2;
        } else if (parent.justify == Justify::End) {
            float total_children_main = total_fixed_main + flex_size * flex_count;
            main_pos = content_y + content_h - total_children_main - total_gap;
        } else if (parent.justify == Justify::SpaceBetween && children.size() > 1) {
            float space = (content_h - total_fixed_main - flex_size * flex_count) / (children.size() - 1);
            gap = space;
            main_pos = content_y;
        } else if (parent.justify == Justify::SpaceAround && children.size() > 0) {
            float space = (content_h - total_fixed_main - flex_size * flex_count) / children.size();
            gap = space;
            main_pos = content_y + space / 2;
        } else {
            main_pos = content_y;
        }
    }

    // Position children
    for (size_t i = 0; i < measures.size(); i++) {
        const ChildMeasure& m = measures[i];
        
        float child_x, child_y;
        
        if (is_row) {
            child_x = main_pos;
            // Apply align (cross-axis)
            if (parent.align == Align::Center) {
                child_y = content_y + (content_h - m.h) / 2;
            } else if (parent.align == Align::End) {
                child_y = content_y + content_h - m.h;
            } else if (parent.align == Align::Stretch) {
                child_y = content_y;
                // Height already set in measure phase
            } else {
                child_y = content_y; // Start
            }
            main_pos += m.w + gap;
        } else {
            child_y = main_pos;
            // Apply align (cross-axis)
            if (parent.align == Align::Center) {
                child_x = content_x + (content_w - m.w) / 2;
            } else if (parent.align == Align::End) {
                child_x = content_x + content_w - m.w;
            } else if (parent.align == Align::Stretch) {
                child_x = content_x;
                // Width already set in measure phase
            } else {
                child_x = content_x; // Start
            }
            main_pos += m.h + gap;
        }

        // Recursively solve child
        solve_node(*m.el, elements, id_map, child_x, child_y, m.w, m.h, results);
    }
}

} // namespace internal
} // namespace fanta
