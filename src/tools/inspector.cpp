#include "inspector.hpp"

namespace fanta {
namespace internal {

static InspectorState g_inspector;

InspectorState& GetInspector() {
    return g_inspector;
}

void InspectorState::build_tree(const std::vector<ElementState>& elements) {
    nodes.clear();
    
    for (const auto& el : elements) {
        InspectorNode node;
        node.id = el.id;
        node.label = el.label.empty() ? "[unnamed]" : el.label;
        node.depth = 0;  // Would need parent traversal
        node.selected = (el.id == selected_id);
        
        // Type detection
        if (el.is_canvas) node.type_name = "Canvas";
        else if (el.is_node) node.type_name = "Node";
        else if (el.is_text_input) node.type_name = "TextInput";
        else if (el.is_tree_node) node.type_name = "TreeNode";
        else if (el.is_slider) node.type_name = "Slider";
        else if (el.is_toggle) node.type_name = "Toggle";
        else if (el.is_table) node.type_name = "Table";
        else if (el.is_scrollable) node.type_name = "ScrollArea";
        else if (el.is_popup) node.type_name = "Popup";
        else node.type_name = "Element";
        
        // Properties
        node.properties["id"] = std::to_string(el.id.value);
        node.properties["w"] = std::to_string(static_cast<int>(el.w));
        node.properties["h"] = std::to_string(static_cast<int>(el.h));
        node.properties["hovered"] = el.is_hovered ? "true" : "false";
        node.properties["focused"] = el.is_focused ? "true" : "false";
        
        node.children = el.children;
        nodes.push_back(node);
    }
}

void InspectorState::select(ID id) {
    selected_id = id;
    for (auto& n : nodes) {
        n.selected = (n.id == id);
    }
}

void InspectorState::toggle(ID id) {
    for (auto& n : nodes) {
        if (n.id == id) {
            n.expanded = !n.expanded;
            break;
        }
    }
}

InspectorNode* InspectorState::find_node(ID id) {
    for (auto& n : nodes) {
        if (n.id == id) return &n;
    }
    return nullptr;
}

void InspectorLogic::Update(InspectorState& state, InputContext& input, RuntimeState& runtime) {
    if (!state.visible) return;
    
    // Rebuild tree each frame (could optimize)
    state.build_tree(GetStore().frame_elements);
    
    (void)input;
    (void)runtime;
}

void InspectorLogic::Render(InspectorState& state, DrawList& dl, float screen_width, float screen_height) {
    if (!state.visible) return;
    
    float total_width = state.panel_width + state.property_panel_width;
    float x = state.dock_left ? 0 : screen_width - total_width;
    float y = 0;
    float h = screen_height;
    
    // Background
    ColorF bg = {0.12f, 0.12f, 0.12f, 0.95f};
    dl.add_rect({x, y}, {total_width, h}, bg, 0);
    
    // Tree panel
    RenderTreePanel(state, dl, x, y, state.panel_width, h);
    
    // Property panel
    RenderPropertyPanel(state, dl, x + state.panel_width, y, state.property_panel_width, h);
}

void InspectorLogic::RenderTreePanel(InspectorState& state, DrawList& dl, float x, float y, float w, float h) {
    // Header
    ColorF header_bg = {0.2f, 0.2f, 0.2f, 1.0f};
    dl.add_rect({x, y}, {w, 28.0f}, header_bg, 0);
    
    // TODO: Render "Elements" header text
    
    // Tree items
    float item_height = 22.0f;
    float item_y = y + 32.0f;
    
    for (const auto& node : state.nodes) {
        if (item_y > y + h) break;  // Clip
        
        // Selection highlight
        if (node.selected) {
            ColorF sel = {0.3f, 0.3f, 0.5f, 1.0f};
            dl.add_rect({x, item_y}, {w, item_height}, sel, 0);
        }
        
        // Indent
        float indent = node.depth * 16.0f;
        
        // Expand arrow (if has children)
        if (!node.children.empty()) {
            // TODO: Draw triangle
        }
        
        // TODO: Render node.type_name and node.label
        
        item_y += item_height;
        (void)indent;
    }
    
    (void)h;
}

void InspectorLogic::RenderPropertyPanel(InspectorState& state, DrawList& dl, float x, float y, float w, float h) {
    // Header
    ColorF header_bg = {0.2f, 0.2f, 0.2f, 1.0f};
    dl.add_rect({x, y}, {w, 28.0f}, header_bg, 0);
    
    // TODO: Render "Properties" header text
    
    // Properties
    if (auto* node = state.find_node(state.selected_id)) {
        float prop_y = y + 36.0f;
        float row_height = 20.0f;
        
        for (const auto& [key, value] : node->properties) {
            // TODO: Render property key-value
            prop_y += row_height;
            
            if (prop_y > y + h) break;
        }
    }
}

} // namespace internal

// Public API
void ShowInspector(bool show) {
    internal::GetInspector().visible = show;
}

void ToggleInspector() {
    internal::GetInspector().visible = !internal::GetInspector().visible;
}

bool IsInspectorVisible() {
    return internal::GetInspector().visible;
}

} // namespace fanta
