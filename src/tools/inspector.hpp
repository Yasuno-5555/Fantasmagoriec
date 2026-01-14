#pragma once

#include "../core/types_internal.hpp"
#include <vector>
#include <string>
#include <map>
#include <functional>

namespace fanta {
namespace internal {

// Inspector Node (UI element representation)
struct InspectorNode {
    ID id;
    std::string type_name;
    std::string label;
    bool expanded = false;
    bool selected = false;
    int depth = 0;
    std::vector<ID> children;
    
    // Properties
    std::map<std::string, std::string> properties;  // Key-value for display
};

// Inspector State
struct InspectorState {
    bool visible = false;
    std::vector<InspectorNode> nodes;
    ID selected_id{};
    ID hovered_id{};
    
    // Layout
    float panel_width = 300.0f;
    float property_panel_width = 250.0f;
    bool dock_left = false;  // false = right
    
    // Filter
    std::string search_filter;
    bool show_hidden = false;
    
    void build_tree(const std::vector<ElementState>& elements);
    void select(ID id);
    void toggle(ID id);
    InspectorNode* find_node(ID id);
};

// Inspector Logic
struct InspectorLogic {
    static void Update(InspectorState& state, InputContext& input, RuntimeState& runtime);
    static void Render(InspectorState& state, DrawList& dl, float screen_width, float screen_height);
    static void RenderTreePanel(InspectorState& state, DrawList& dl, float x, float y, float w, float h);
    static void RenderPropertyPanel(InspectorState& state, DrawList& dl, float x, float y, float w, float h);
};

// Global accessor
InspectorState& GetInspector();

} // namespace internal

// Public API
void ShowInspector(bool show = true);
void ToggleInspector();
bool IsInspectorVisible();

} // namespace fanta
