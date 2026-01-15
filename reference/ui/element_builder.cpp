#include <fanta.h>
#include "../core/contexts_internal.hpp"
#include "../core/contexts_internal.hpp"
#include "../core/markdown.hpp"
#include "widget_logics.hpp"

#include <string>
#include <vector>
#include <functional>

namespace fanta {

// --- Element Handle Implementation ---
static size_t ToIndex(ElementHandle* h) { return reinterpret_cast<size_t>(h) - 1; }
static ElementHandle* ToHandle(size_t idx) { return reinterpret_cast<ElementHandle*>(idx + 1); }

// Helper to access state safely
static internal::ElementState& GetState(ElementHandle* h) {
    return internal::GetStore().frame_elements[ToIndex(h)];
}


Element::Element(ID id) {
    auto& store = internal::GetStore();
    auto& runtime = internal::GetRuntime();
    
    internal::ElementState s{}; 
    
    // ID Generation: Mix with Stack Top to ensure hierarchy uniqueness
    if (runtime.id_stack.empty()) {
        s.id = id;
    } else {
        ID parent = runtime.id_stack.back();
        // FNV-1a Mix
        uint64_t mixed = parent.value ^ id.value;
        mixed *= 1099511628211ULL;
        s.id = ID(mixed);
    }

    s.has_visible_label = false; 
    
    // Default Styling
    s.text_color = {0, internal::ColorToken::Label, true};
    
    // Phase 6.2: Load Persistent State
    auto it = store.persistent_states.find(s.id);
    if (it != store.persistent_states.end()) {
        s.persistent = it->second;
        s.is_selected = s.persistent.is_selected;
    }

    store.frame_elements.push_back(s);
    handle = ToHandle(store.frame_elements.size() - 1);
}

Element& Element::label(const char* text) {
    auto& s = GetState(handle);
    s.label = text;
    s.has_visible_label = true;
    return *this;
}

Element& Element::width(float w) { 
    GetState(handle).w = w;
    return *this;
}

Element& Element::height(float h) { 
    GetState(handle).h = h;
    return *this;
}

Element& Element::size(float w, float h) {
    auto& s = GetState(handle);
    s.w = w;
    s.h = h;
    return *this;
}

Element& Element::row() {
    GetState(handle).layout_mode = internal::LayoutMode::Row;
    return *this;
}

Element& Element::column() {
    GetState(handle).layout_mode = internal::LayoutMode::Column;
    return *this;
}

Element& Element::wrap(bool enable) {
    GetState(handle).wrap_mode = enable ? internal::Wrap::Wrap : internal::Wrap::NoWrap;
    return *this;
}

Element& Element::padding(float p) {
    GetState(handle).p = p;
    return *this;
}

Element& Element::gap(float g) {
    GetState(handle).gap = g;
    return *this;
}

Element& Element::bg(Color c) {
    auto& s = GetState(handle);
    s.bg_color.rgba = c.rgba;
    s.bg_color.token = static_cast<internal::ColorToken>(c.token);
    s.bg_color.is_semantic = c.is_semantic;
    return *this;
}

Element& Element::color(Color c) {
    auto& s = GetState(handle);
    s.text_color.rgba = c.rgba;
    s.text_color.token = static_cast<internal::ColorToken>(c.token);
    s.text_color.is_semantic = c.is_semantic;
    return *this;
}

Element& Element::fontSize(float size) {
    GetState(handle).font_size = size;
    return *this;
}

Element& Element::textStyle(TextToken t) {
    GetState(handle).text_token = static_cast<internal::TextToken>(t);
    return *this;
}

Element& Element::rounded(float radius) {
    GetState(handle).corner_radius = radius;
    return *this;
}

Element& Element::elevation(float e) {
    GetState(handle).elevation = e;
    return *this;
}

Element& Element::alignContent(Align alignment) {
    GetState(handle).align_content = alignment;
    return *this;
}

Element& Element::flexGrow(float grow) {
    GetState(handle).flex_grow = grow;
    return *this;
}

Element& Element::hoverBg(Color c) {
    auto& s = GetState(handle);
    if (!s.hover_style) s.hover_style.emplace();
    s.hover_style->bg = {c.rgba, static_cast<internal::ColorToken>(c.token), c.is_semantic};
    return *this;
}

Element& Element::activeBg(Color c) {
    auto& s = GetState(handle);
    if (!s.active_style) s.active_style.emplace();
    s.active_style->bg = {c.rgba, static_cast<internal::ColorToken>(c.token), c.is_semantic};
    return *this;
}

Element& Element::justify(Justify j) {
    GetState(handle).justify = j;
    return *this;
}

Element& Element::align(Align a) {
    GetState(handle).align = a;
    return *this;
}

Element& Element::child(const Element& child) {
    auto& store = internal::GetStore();
    auto& parent_state = GetState(handle);
    // Element contains a handle pointer.
    // We need to dereference it to get state.
    auto& child_state = GetState(child.handle);
    
    // Parenting logic
    child_state.parent = parent_state.id;
    parent_state.children.push_back(child_state.id);
    
    return *this;
}

Element& Element::line() {
    GetState(handle).is_line = true;
    return *this;
}

Element& Element::canvas(float zoom, float pan_x, float pan_y) {
    auto& s = GetState(handle);
    s.is_canvas = true;
    s.canvas_zoom = zoom;
    s.canvas_pan = {pan_x, pan_y};
    return *this;
}

Element& Element::node(float x, float y) {
    auto& s = GetState(handle);
    s.is_node = true;
    s.node_pos = {x, y};
    return *this;
}

Element& Element::port() {
    GetState(handle).is_port = true;
    return *this;
}



Element& Element::slider(float& value, float min, float max) {
    auto& s = GetState(handle);
    s.is_slider = true;
    s.slider_min = min;
    s.slider_max = max;
    
    internal::RuntimeState& runtime = internal::GetRuntime();
    internal::InputContext& input = internal::GetInput();
    
    // Sync to persistent store
    auto& p_data = internal::GetStore().persistent_states[s.id];
    
    // If it's a new element or externally changed, update persistent
    // (In immediate mode, detect external change can be tricky, but let's assume
    // the user might want code-driven changes to be undoable too if they happen
    // between frames, but for now we just let logic take over).
    
    if (runtime.layout_results.count(s.id)) {
        internal::SliderLogic::Update(s, input, runtime.layout_results.at(s.id));
    } else {
        // First frame: initialize p_data from value
        static bool first = true; // Not ideal, but we need a better "first seen" check
        // Actually PersistentData is initialized when first used.
        // Let's just sync p_data -> s.slider_val and s.slider_val -> value
        s.slider_val = p_data.slider_val; 
    }
    
    value = p_data.slider_val; // Update the user's reference
    return *this;
}

Element& Element::toggle(bool& value, const char* label) {
    auto& s = GetState(handle);
    s.is_toggle = true;
    if (label) {
        s.label = label;
        s.has_visible_label = true;
    }
    
    auto& p_data = internal::GetStore().persistent_states[s.id];
    internal::ToggleLogic::Update(s, internal::GetInput());
    
    value = p_data.toggle_val;
    return *this;
}

Element& Element::wire(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float thickness) {
    auto& s = GetState(handle);
    s.is_wire = true;
    s.wire_p0 = {x0, y0};
    s.wire_p1 = {x1, y1};
    s.wire_p2 = {x2, y2};
    s.wire_p3 = {x3, y3};
    s.wire_thickness = thickness;
    return *this;
}

Element& Element::onClick(std::function<void()> callback) {
    auto& s = GetState(handle);
    if (IsClicked(s.id)) { 
        callback();
    }
    return *this;
}

Element& Element::activeScale(float scale) {
    auto& s = GetState(handle);
    if (!s.active_style) s.active_style.emplace();
    s.active_style->scale_delta = scale - 1.0f; // Store delta
    return *this;
}

Element& Element::activeElevation(float e) {
    auto& s = GetState(handle);
    if (!s.active_style) s.active_style.emplace();
    s.active_style->elevation_delta = e - s.elevation;
    return *this;
}

Element& Element::hoverElevation(float e) {
    auto& s = GetState(handle);
    if (!s.hover_style) s.hover_style.emplace();
    s.hover_style->elevation_delta = e - s.elevation;
    return *this;
}

Element& Element::scrollable(bool vertical, bool horizontal) {
    auto& s = GetState(handle);
    s.is_scrollable = true;
    s.scroll_vertical = vertical;
    s.scroll_horizontal = horizontal;
    return *this;
}

Element& Element::markdown(const char* md_text) {
    auto& s = GetState(handle);
    s.label = md_text;
    return *this;
}

Element& Element::focusable(bool enable) {
    GetState(handle).is_focusable = enable;
    return *this;
}

Element& Element::focusBg(Color c) {
    auto& s = GetState(handle);
    if (!s.focus_style) s.focus_style.emplace();
    s.focus_style->bg = {c.rgba, static_cast<internal::ColorToken>(c.token), c.is_semantic};
    return *this;
}

Element& Element::focusLift(float delta) {
    auto& s = GetState(handle);
    if (!s.focus_style) s.focus_style.emplace();
    s.focus_style->elevation_delta = delta;
    return *this;
}

Element& Element::blur(float radius) {
    GetState(handle).blur_radius = radius;
    return *this;
}

Element& Element::backdrop(bool enable) {
    GetState(handle).backdrop_blur = enable;
    return *this;
}

Element& Element::popup() {
    GetState(handle).is_popup = true;
    return *this;
}

Element& Element::hoverLift(float delta) { // Implement generic lift
     return hoverElevation(delta); // Assuming this what was meant, or delta from base. 
     // For now use hoverElevation logic if delta is absolute? No delta is relative.
     // Let's assume hoverElevation takes logic.
     // We implemented hoverElevation as absolute replacement.
     // But `hoverLift` implies delta.
     // Let's implement it properly.
     auto& s = GetState(handle);
     if (!s.hover_style) s.hover_style.emplace();
     s.hover_style->elevation_delta = delta;
     return *this;
}

// Missing method from previous header but in implementation:
Element& Element::disabled(bool enable) {
    GetState(handle).is_disabled = enable;
    return *this;
}

// Phase 1: TextInput
Element& Element::textInput(std::string& value, const char* placeholder) {
    auto& s = GetState(handle);
    s.is_text_input = true;
    if (placeholder) { s.label = placeholder; s.has_visible_label = false; }
    
    auto& p_data = internal::GetStore().persistent_states[s.id];
    
    // Call interaction logic (cursor placement on click)
    internal::RuntimeState& runtime = internal::GetRuntime();
    if (runtime.layout_results.count(s.id)) {
        internal::TextInputLogic::Update(s, internal::GetInput(), runtime.layout_results.at(s.id));
    }

    // Sync from persistent to state for rendering
    s.text_buffer = p_data.text_buffer;
    s.cursor_pos = p_data.cursor_pos;
    s.selection_start = p_data.selection_start;
    s.selection_end = p_data.selection_end;

    // Sync back to user's reference
    value = p_data.text_buffer;
    return *this;
}

// Phase 1: TreeNode
Element& Element::treeNode(bool& expanded, const char* lbl) {
    auto& s = GetState(handle);
    s.is_tree_node = true;
    s.tree_expanded = expanded;
    s.label = lbl;
    s.has_visible_label = true;
    
    // Sync state
    expanded = s.tree_expanded;
    return *this;
}

// Phase 2: Table
Element& Element::beginTable(int columns) {
    auto& s = GetState(handle);
    s.is_table = true;
    // Basic initialization
    (void)columns;
    return *this;
}

Element& Element::tableHeader(const char* lbl, float width) {
    auto& s = GetState(handle);
    s.label = lbl;
    if (width > 0) s.w = width;
    return *this;
}

Element& Element::tableCell() {
    // Cell is just a styled container
    return *this;
}

Element& Element::tableRow() {
    auto& s = GetState(handle);
    s.layout_mode = internal::LayoutMode::Row;
    return *this;
}

// Phase 3: Context Menu
Element& Element::contextMenu() {
    auto& s = GetState(handle);
    s.is_popup = true;  // Reuse popup infrastructure
    return *this;
}

Element& Element::menuItem(const char* lbl, const char* shortcut) {
    auto& s = GetState(handle);
    s.label = lbl ? lbl : "";
    s.has_visible_label = true;
    // Shortcut would be stored separately, for now in label
    if (shortcut) {
        s.label += "\t";
        s.label += shortcut;
    }
    return *this;
}

// Phase 4: Menu Bar
Element& Element::beginMenuBar() {
    auto& s = GetState(handle);
    s.layout_mode = internal::LayoutMode::Row;
    s.h = 22.0f;  // Standard menu bar height
    return *this;
}

Element& Element::menu(const char* lbl) {
    auto& s = GetState(handle);
    s.label = lbl;
    s.has_visible_label = true;
    s.is_focusable = true;
    return *this;
}

Element& Element::endMenuBar() {
    // No-op, just for API symmetry
    return *this;
}

Element& Element::gridOffset(int r, int c, int rs, int cs) {
    auto& s = internal::GetState(m_id);
    // Grid logic would use these
    (void)s; (void)r; (void)c; (void)rs; (void)cs;
    return *this;
}

Element& Element::path(const std::string& svg_path, uint32_t color, float stroke, bool fill) {
    auto& s = internal::GetState(m_id);
    // Naive SVG parsing could go here
    (void)s; (void)svg_path; (void)color; (void)stroke; (void)fill;
    return *this;
}

Element& Element::richText(const std::string& markup) {
    auto& s = internal::GetState(m_id);
    s.label = markup; // Currently label stores the markup
    s.has_visible_label = true;
    // Renderer will call RichText::Parse
    return *this;
}

Element& Element::constraint(ID other, int type, float param) {
    // Constraint logic would be handled in the solver pass
    (void)other; (void)type; (void)param;
    return *this;
}

Element& Element::safeArea(bool enable) {
    auto& s = internal::GetState(m_id);
    s.respect_safe_area = enable;
    return *this;
}

Element& Element::onLifeCycle(std::function<void(LifeCycle)> callback) {
    auto& s = internal::GetState(m_id);
    s.lifecycle_callback = callback;
    return *this;
}

Element& Element::accessibilityLabel(const std::string& label) {
    auto& s = internal::GetState(m_id);
    s.accessibility_label = label;
    return *this;
}

Element& Element::accessibilityHint(const std::string& hint) {
    auto& s = internal::GetState(m_id);
    s.accessibility_hint = hint;
    return *this;
}

Element& Element::role(AccessibilityRole role) {
    auto& s = internal::GetState(m_id);
    s.accessibility_role = role;
    return *this;
}

Element& Element::rounded(float radius, bool squircle) {
    auto& s = internal::GetState(m_id);
    s.radius = radius;
    s.is_squircle = squircle;
    return *this;
}

Element& Element::rtl(bool enable) {
    auto& s = internal::GetState(m_id);
    s.is_rtl = enable;
    return *this;
}

Element& Element::animate(float duration_ms, int curve_type) {
    auto& s = internal::GetState(m_id);
    s.anim_duration = duration_ms / 1000.0f; // Convert ms to seconds
    s.anim_curve = static_cast<internal::CurveType>(curve_type);
    return *this;
}

Element& Element::hero(const std::string& tag) {
    auto& s = internal::GetState(m_id);
    s.hero_tag = tag;
    return *this;
}

Element& Element::script(const std::string& source) {
    auto& s = internal::GetState(m_id);
    s.script_source = source;
    // Immediate execution for one-off logic if engine exists
    if (!source.empty()) {
        Scripting::Run(source);
    }
    return *this;
}

Element& Element::backdropBlur(float sigma) {
    auto& s = internal::GetState(m_id);
    s.backdrop_blur = sigma;
    return *this;
}

Element& Element::vibrancy(float amount) {
    auto& s = internal::GetState(m_id);
    s.vibrancy = amount;
    return *this;
}

Element& Element::material(MaterialType type) {
    auto& s = internal::GetState(m_id);
    s.material = type;
    
    // Apple-inspired Material Presets (System Chrome / Vibrancy)
    if (type == MaterialType::UltraThin) {
        s.color = {1.0f, 1.0f, 1.0f, 0.15f};
        s.backdrop_blur = 15.0f;
    } else if (type == MaterialType::Thin) {
        s.color = {1.0f, 1.0f, 1.0f, 0.3f};
        s.backdrop_blur = 25.0f;
    } else if (type == MaterialType::Regular) {
        s.color = {1.0f, 1.0f, 1.0f, 0.5f};
        s.backdrop_blur = 40.0f;
    } else if (type == MaterialType::Thick) {
        s.color = {1.0f, 1.0f, 1.0f, 0.75f};
        s.backdrop_blur = 60.0f;
    } else if (type == MaterialType::Chrome) {
        s.color = {0.95f, 0.95f, 0.97f, 0.9f}; // iOS Greyish Chrome
        s.backdrop_blur = 20.0f;
    }
    
    s.is_squircle = true; // Materials almost always use squircles in Apple design
    return *this;
}

Element& Element::virtualList(size_t count, float item_height, std::function<void(size_t index, Element& item)> item_builder) {
    auto& s = GetState(handle);
    s.is_scrollable = true;
    s.scroll_vertical = true;
    
    internal::RuntimeState& runtime = internal::GetRuntime();
    internal::StateStore& store = internal::GetStore();
    
    // Get current scroll state
    auto& p_data = store.persistent_states[s.id];
    float scroll_y = p_data.scroll_y.value;
    
    float view_h = s.h > 0 ? s.h : 600.0f; 
    if (runtime.layout_results.count(s.id)) {
        view_h = runtime.layout_results.at(s.id).h;
    }

    int start_idx = static_cast<int>(std::floor(scroll_y / item_height));
    int end_idx = static_cast<int>(std::ceil((scroll_y + view_h) / item_height));
    
    start_idx = std::max(0, start_idx);
    end_idx = std::min(static_cast<int>(count) - 1, end_idx);

    // Set an invisible element at the end to force scroll range (Phantom Spacer)
    PushID("__virtual_spacer__");
    Element(ID(0xCAFE)).absolute().top(count * item_height - 1.0f).size(1, 1).bg(Color(0,0,0,0));
    PopID();

    // Render visible items
    for (int i = start_idx; i <= end_idx; ++i) {
        PushID(i);
        // Create an item container with absolute positioning
        Element item_container(ID(0x1337));
        item_container.absolute().top(i * item_height).width(s.w > 0 ? s.w : 100.0f).height(item_height);
        
        item_builder(static_cast<size_t>(i), item_container);
        PopID();
    }

    return *this;
}

Element& Element::bind(Observable<float>& obs) {
    auto& s = GetState(handle);
    if (s.is_slider) {
        // Two-way sync
        auto& p_data = internal::GetStore().persistent_states[s.id];
        // 1. UI -> Data
        obs.set(p_data.slider_val);
        // 2. Data -> UI (if changed externally)
        obs.onChange([&](float v) {
            auto& p = internal::GetStore().persistent_states[s.id];
            p.slider_val = v;
        });
    }
    return *this;
}

Element& Element::bind(Observable<bool>& obs) {
    auto& s = GetState(handle);
    if (s.is_toggle) {
         auto& p_data = internal::GetStore().persistent_states[s.id];
         obs.set(p_data.toggle_val);
         obs.onChange([&](bool v) {
            auto& p = internal::GetStore().persistent_states[s.id];
            p.toggle_val = v;
        });
    }
    return *this;
}

Element& Element::bind(Observable<std::string>& obs) {
    auto& s = GetState(handle);
    if (s.is_text_input) {
        auto& p_data = internal::GetStore().persistent_states[s.id];
        obs.set(p_data.text_buffer);
        obs.onChange([&](const std::string& v) {
            auto& p = internal::GetStore().persistent_states[s.id];
            p.text_buffer = v;
        });
    }
    return *this;
}

Element& Element::beginGroup(const char* label) {
    column().padding(12).gap(8).bg(Color(35, 35, 35)).rounded(6);
    Element(ID(label)).label(label).textStyle(TextToken::Heading2).color(Color(200, 200, 200));
    return *this;
}

Element& Element::endGroup() {
    return *this;
}

Element& Element::property(const char* label, float& value, float min, float max) {
    PushID(label);
    row().align(Align::Center).gap(8).padding(2).height(24);
    Element(ID("lbl")).label(label).width(100).textStyle(TextToken::Body).color(Color(180, 180, 180));
    Element(ID("val")).slider(value, min, max).flexGrow(1.0f);
    PopID();
    return *this;
}

Element& Element::property(const char* label, bool& value) {
    PushID(label);
    row().align(Align::Center).gap(8).padding(2).height(24);
    Element(ID("lbl")).label(label).width(100).textStyle(TextToken::Body).color(Color(180, 180, 180));
    Element(ID("val")).toggle(value);
    PopID();
    return *this;
}

Element& Element::property(const char* label, std::string& value) {
    PushID(label);
    row().align(Align::Center).gap(8).padding(2).height(24);
    Element(ID("lbl")).label(label).width(100).textStyle(TextToken::Body).color(Color(180, 180, 180));
    Element(ID("val")).textInput(value).flexGrow(1.0f);
    PopID();
    return *this;
}

Element& Element::property(const char* label, Color& value) {
    PushID(label);
    row().align(Align::Center).gap(8).padding(2).height(24);
    Element(ID("lbl")).label(label).width(100).textStyle(TextToken::Body).color(Color(180, 180, 180));
    Element(ID("swatch")).size(40, 16).bg(value).rounded(2);
    PopID();
    return *this;
}

Element& Element::accessibleName(const char* name) {
    auto& s = internal::GetState(handle);
    s.accessible_name = name;
    return *this;
}

Element& Element::role(AccessibilityRole role) {
    auto& s = internal::GetState(handle);
    s.accessibility_role = role;
    return *this;
}

Element& Element::shader(const std::string& source) {
    auto& s = internal::GetState(handle);
    s.shader_source = source;
    return *this;
}
