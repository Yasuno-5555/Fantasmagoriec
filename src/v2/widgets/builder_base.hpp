// Fantasmagorie v2 - Builder Base
// Common infrastructure for all widget builders
#pragma once

#include "widget_base.hpp"
#include "../core/undo.hpp"
#include "../script/engine.hpp"
#include "../a11y/accessibility.hpp"

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
    
    Derived& self() { return static_cast<Derived&>(*this); }
    
    void apply_constraints(NodeHandle& n) {
        if (width_ > 0) n.constraint().width = width_;
        if (height_ > 0) n.constraint().height = height_;
        if (grow_ > 0) n.constraint().grow = grow_;
        if (padding_ > 0) n.constraint().padding = padding_;
    }
    
    void apply_script(NodeID id) {
        if (script_code_) {
            script::global_context().exec(script_code_);
        }
    }
    
    void apply_accessibility(NodeID id, const char* role, const char* name) {
        if (a11y_desc_ || tab_index_ >= 0) {
            a11y::AccessibleInfo info;
            info.name = name ? name : "";
            info.description = a11y_desc_ ? a11y_desc_ : "";
            info.role = role ? role : "widget";
            a11y::AccessibleStore::instance().set(id, info);
            
            if (tab_index_ >= 0) {
                a11y::FocusManager::instance().register_focusable(id, tab_index_);
            }
        }
    }
};

// ============================================================================
// Scope Guard for Panel/Window
// ============================================================================

class PanelScope {
public:
    PanelScope(const char* id) {
        if (g_ctx) {
            id_ = g_ctx->begin_node(id);
            NodeHandle n = g_ctx->get(id_);
            n.style().bg = Color::Hex(0x1A1A1AFF);
            n.style().corner_radius = 8.0f;
            n.constraint().padding = 8.0f;
        }
    }
    
    ~PanelScope() {
        if (g_ctx) g_ctx->end_node();
    }
    
    PanelScope(const PanelScope&) = delete;
    PanelScope& operator=(const PanelScope&) = delete;
    
private:
    NodeID id_ = INVALID_NODE;
};

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
