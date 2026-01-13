// Fantasmagorie v2 - Builder Base
// Common infrastructure for all widget builders
#pragma once

#include "widget_base.hpp"
// #include "../core/undo.hpp"
// #include "../script/engine.hpp"
// #include "../a11y/accessibility.hpp"
#include <unordered_map>
#include <functional>

namespace fanta {

// ============================================================================
// Builder Traits
// ============================================================================

template<typename Derived>
class BuilderBase {
public:
    Derived& width(float w) { width_ = w; return self(); }
    Derived& height(float h) { height_ = h; return self(); }
    Derived& grow(float g) { grow_ = g; return self(); }
    Derived& padding(float p) { padding_ = p; return self(); }
    Derived& id(const char* id) { custom_id_ = id; return self(); }
    
    // Undoable support
    Derived& undoable() { undoable_ = true; return self(); }
    
    // Script support (Phase 8)
    Derived& script(const char* code) { script_code_ = code; return self(); }
    Derived& on_script(const char* event, const char* code) { 
        script_events_[event] = code; 
        return self(); 
    }
    
    // Accessibility support (Phase 9)
    Derived& accessible(const char* description) { 
        a11y_desc_ = description; 
        return self(); 
    }
    Derived& tab_index(int index) { tab_index_ = index; return self(); }
    
    // Implicit Animation (Phase 1)
    Derived& animate_on(InteractionState state, std::function<void(ResolvedStyle&)> fn) {
        visual_modifiers_[state] = fn;
        return self();
    }
    
protected:
    float width_ = 0;
    float height_ = 0;
    float grow_ = 0;
    float padding_ = 0;
    const char* custom_id_ = nullptr;
    bool undoable_ = false;
    const char* script_code_ = nullptr;
    std::unordered_map<std::string, std::string> script_events_;
    const char* a11y_desc_ = nullptr;
    int tab_index_ = -1;
    bool built_ = false;
    
    // Visual modifiers for implicit animation
    std::map<InteractionState, std::function<void(ResolvedStyle&)>> visual_modifiers_;
    
    Derived& self() { return static_cast<Derived&>(*this); }
    
    void apply_constraints(NodeHandle& n) {
        FANTA_ASSERT_CONTEXT();
        if (width_ > 0) n.constraint().width = width_;
        if (height_ > 0) n.constraint().height = height_;
        if (grow_ > 0) n.constraint().grow = grow_;
        if (padding_ > 0) n.constraint().padding = padding_;
    }
    
    // Call this at the end of build() to set up implicit animations
    void apply_visuals(NodeHandle& n) {
        if (visual_modifiers_.empty()) return;
        
        if (!g_ctx) return;
        auto& store = g_ctx->store;
        NodeID id = n.id();
        
        // Ensure VisualState exists
        // Note: NodeHandle doesn't expose visual_state directly in current API, 
        // we might need to add it or access store via context global?
        // NodeHandle has store_ member but it's private.
        // But we are in fanta namespace.
        // Actually NodeHandle::visual_state() was not added in my previous edit to node.hpp!
        // I need to add that accessor or just use g_ctx->store.
        
        if (!g_ctx) return;
        auto& vs = g_ctx->store.visual_state[id];
        
        // Base style (already set by builder)
        const auto& base = g_ctx->store.style[id];
        vs.targets[0] = base; // Default state
        
        // Generate targets for modifiers
        for (auto& [state, func] : visual_modifiers_) {
            ResolvedStyle target = base;
            func(target);
            vs.targets[(int)state] = target;
        }
        
        if (!vs.initialized) {
            vs.current = base;
            vs.initialized = true;
        }
    }
    
    void apply_script(NodeID id) {
        FANTA_ASSERT_CONTEXT();
        // if (script_code_) {
        //     script::global_context().exec(script_code_);
        // }
    }
    
    void apply_accessibility(NodeID id, const char* role, const char* name) {
        // if (a11y_desc_ || tab_index_ >= 0) {
        //     a11y::AccessibleInfo info;
        //     info.name = name ? name : "";
        //     info.description = a11y_desc_ ? a11y_desc_ : "";
        //     info.role = role ? role : "widget";
        //     a11y::AccessibleStore::instance().set(id, info);
        //     
        //     if (tab_index_ >= 0) {
        //         a11y::FocusManager::instance().register_focusable(id, tab_index_);
        //     }
        // }
    }
};

// PanelScope moved to widget_base.hpp

// ============================================================================
// Children Lambda Helper
// ============================================================================

template<typename Derived>
class ContainerBuilder : public BuilderBase<Derived> {
public:
    Derived& children(std::function<void()> fn) {
        children_fn_ = fn;
        return this->self();
    }
    
protected:
    std::function<void()> children_fn_;
    
    void build_children() {
        if (children_fn_) children_fn_();
    }
};

} // namespace fanta
