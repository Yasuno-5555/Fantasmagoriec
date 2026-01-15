#include "ui_context.hpp"
#include <algorithm>

// Helper
static uint32_t to_u32(const ui::Color& c) {
    uint32_t r = (uint32_t)(c.r * 255);
    uint32_t g = (uint32_t)(c.g * 255);
    uint32_t b = (uint32_t)(c.b * 255);
    uint32_t a = (uint32_t)(c.a * 255);
    return (r << 24) | (g << 16) | (b << 8) | a;
}

void UIContext::open_popup(const char* id) {
    NodeID nid = hash_str(id);
    open_popups.push_back(nid);
}

void UIContext::begin_frame() {
    node_store.clear();
    overlay_list.clear();
    tooltip_text.clear();
    frame_scroll_delta = 0.0f;
    
    // Reset frame counters
    last_id = 0;
    last_id = 0;
    parent_stack_vec.clear();
    
    // Style Stack Reset
    style_stack.clear();
    Style default_style;
    style_stack.push_back(default_style);
    
    // Transform Stack Reset
    transform_stack.clear();
    current_transform = Transform::Identity();

    
    // Reset Next
    next_layout = LayoutConstraints();
    next_style = Style();

    root_id = get_id("root");
    begin_node(root_id); 
}

void UIContext::end_frame() {
    // Persist scroll state
    for (const auto& id : node_store.nodes) {
        if (node_store.scroll[id].scrollable) {
            persistent_scroll[id] = node_store.scroll[id].scroll_y;
        }
        if (node_store.layout.count(id)) {
            last_node_w = node_store.layout[id].computed_w;
            last_node_h = node_store.layout[id].computed_h;
        }
    }
    
    // Pop stacks
    // Snapshot for Next Frame Input
    prev_layout = node_store.layout;
    prev_input = node_store.input;
    prev_tree = node_store.tree; // Phase 19
    prev_scroll = node_store.scroll; // Phase 19
    prev_transform = node_store.transform; // Audit: Fix Hit Testing
    prev_nodes = node_store.nodes;
    
    // Clear Per-Frame Data
    node_store.clear(); // Pop root
}


void UIContext::begin_node(NodeID id) {
    node_store.add_node(id);
    node_store.transform[id] = current_transform; // Capture Transform
    
    if (!parent_stack_vec.empty()) {
        NodeID pid = parent_stack_vec.back();
        // Link Tree
        node_store.tree[pid].children.push_back(id);
        node_store.tree[id].parent = pid;
    }
    
    parent_stack_vec.push_back(id);
    node_stack.push(id); // For tracking current_id? 
    // Wait, ui_context.hpp declared node_stack too. 
    // But parent_stack_vec effectively tracks the hierarchy. 
    // I'll keep node_stack logic if needed for current_id.
    current_id = id;
    
    // Restore persistent state
    if (persistent_scroll.count(id)) {
        node_store.scroll[id].scroll_y = persistent_scroll[id];
    }
}

void UIContext::end_node() {
    // Audit: Update last_node size IMMEDIATELY so GetLastNodeSize works in current frame
    NodeID id = parent_stack_vec.back();
    if (node_store.layout.count(id)) {
        last_node_w = node_store.layout[id].computed_w;
        last_node_h = node_store.layout[id].computed_h;
    }

    if (!parent_stack_vec.empty()) {
        parent_stack_vec.pop_back();
    }
    // Update current_id
    if (!parent_stack_vec.empty()) current_id = parent_stack_vec.back();
    else current_id = 0;
}

NodeID UIContext::begin_node(const char* name) {
    NodeID id = get_id(name);
    begin_node(id);
    
    // Apply Next State
    NodeHandle n = get(id);
    n.constraint() = next_layout;
    
    // Manual Style Copy from ui::Style to ResolvedStyle
    auto& s = n.style();
    if(next_style.bg.a != 0 || next_style.bg.r!=0) s.bg_color = to_u32(next_style.bg);
    if(next_style.bg_hover.a != 0) s.bg_hover = to_u32(next_style.bg_hover);
    if(next_style.bg_active.a != 0) s.bg_active = to_u32(next_style.bg_active);
    s.text_color = to_u32(next_style.text_color);
    s.corner_radius = next_style.radius;
    s.border_width = next_style.border_width;
    s.border_color = to_u32(next_style.border_color);
    s.show_focus_ring = next_style.show_focus_ring;
    s.focus_ring_width = next_style.focus_ring_width;
    s.focus_ring_color = to_u32(next_style.focus_ring_color);
    // Padding maps to Constraint padding
    if (next_style.padding > 0) n.constraint().padding = next_style.padding;

    // Handle inherit style (Simplified: Stack)
    if (!style_stack.empty()) {
        const Style& p = style_stack.back();
        if (s.bg_color == 0 && p.bg.a > 0) s.bg_color = to_u32(p.bg);
        // ... more inheritance logic ...
    }
    
    next_layout = LayoutConstraints();
    next_style = Style(); 
    if (next_align != -1) {
        n.layout().align = next_align;
        next_align = -1;
    }
    
    return id;
}

NodeID UIContext::get_id(const std::string& str_id) {
    uint32_t hash = hash_str(str_id.c_str());
    if (!parent_stack_vec.empty()) {
        hash ^= (uint32_t)(parent_stack_vec.back() * 16777619); 
    }
    return hash;
}

// ... Animation impl ...
float UIContext::animate(NodeID id, int prop_id, float target, float speed, float dt) {
    AnimState& s = anims[id][prop_id];
    if (!s.initialized) {
        s.f_val = target;
        s.initialized = true;
        return target;
    }
    float diff = target - s.f_val;
    s.f_val += diff * speed * dt;
    if (std::abs(target - s.f_val) < 0.001f) s.f_val = target;
    return s.f_val;
}

uint32_t UIContext::animate_color(NodeID id, int prop_id, uint32_t target, float speed, float dt) {
    AnimState& s = anims[id][prop_id];
    if (!s.initialized) {
        s.c_val = target;
        s.initialized = true;
        return target;
    }
    
    ui::Color c = ui::Color::Hex(s.c_val);
    ui::Color t = ui::Color::Hex(target);
    
    float k = speed * dt;
    if(k > 1.0f) k = 1.0f;
    
    // Lerp
    c.r += (t.r - c.r) * k;
    c.g += (t.g - c.g) * k;
    c.b += (t.b - c.b) * k;
    c.a += (t.a - c.a) * k;
    
    // Check threshold to snap
    // if(diff < ...)
    
    s.c_val = to_u32(c); // Use helper defined in this file (Line 5) or c.u32()?
    // c.u32() is valid struct method. `to_u32` is static helper at top of file.
    // I'll use `c.u32()` if `style.hpp` defines it. Step 1303 confirmed it.
    
    return s.c_val;
}

void UIContext::push_style(const Style& s) {
    style_stack.push_back(s);
}
void UIContext::pop_style() {
    if (!style_stack.empty()) style_stack.pop_back();
}

void UIContext::push_transform(const Transform& t) {
    transform_stack.push_back(current_transform);
    current_transform = current_transform * t;
}

void UIContext::pop_transform() {
    if (!transform_stack.empty()) {
        current_transform = transform_stack.back();
        transform_stack.pop_back();
    } else {
        current_transform = Transform::Identity();
    }
}

// Phase 5: Z-Order Persistence
static std::vector<NodeID> s_window_z_order;

void UIContext::bring_to_front(NodeID id) {
    auto it = std::find(s_window_z_order.begin(), s_window_z_order.end(), id);
    if (it != s_window_z_order.end()) {
        s_window_z_order.erase(it);
    }
    s_window_z_order.push_back(id);
    
    // Apply for current frame immediate effect
    for(auto& p : node_store.tree) {
        auto& c = p.second.children;
        auto cit = std::find(c.begin(), c.end(), id);
        if(cit != c.end()) {
            c.erase(cit);
            c.push_back(id);
            break;
        }
    }
}
