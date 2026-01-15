// Fantasmagorie v2 - Accessibility Support
// Screen reader, keyboard navigation, high contrast
#pragma once

#include "core/types.hpp"
#include "core/node.hpp"
#include <string>
#include <vector>
#include <algorithm>

#ifdef _WIN32
#include <windows.h>
#include <oleacc.h>
#pragma comment(lib, "oleacc.lib")
#endif

namespace fanta {
namespace a11y {

// ============================================================================
// Accessibility Info
// ============================================================================

struct AccessibleInfo {
    std::string name;           // Accessible name (for screen readers)
    std::string description;    // Longer description
    std::string role;           // "button", "slider", "checkbox", etc.
    std::string value;          // Current value (for sliders, inputs)
    bool focusable = true;
    bool disabled = false;
};

// ============================================================================
// Accessibility Store (SoA style)
// ============================================================================

class AccessibleStore {
public:
    static AccessibleStore& instance() {
        static AccessibleStore s;
        return s;
    }
    
    void set(NodeID id, const AccessibleInfo& info) {
        infos_[id] = info;
    }
    
    AccessibleInfo* get(NodeID id) {
        auto it = infos_.find(id);
        return (it != infos_.end()) ? &it->second : nullptr;
    }
    
    void clear() { infos_.clear(); }
    
private:
    std::unordered_map<NodeID, AccessibleInfo> infos_;
};

// ============================================================================
// Screen Reader Announcement
// ============================================================================

inline void announce(const char* text) {
#ifdef _WIN32
    // Use Windows UI Automation or SAPI for screen reader
    // Simplified: use message box as fallback
    // Real implementation would use NotifyWinEvent or IAccessible
    (void)text;
#endif
}

// ============================================================================
// High Contrast Mode Detection
// ============================================================================

inline bool is_high_contrast_mode() {
#ifdef _WIN32
    HIGHCONTRASTW hc = { sizeof(hc) };
    if (SystemParametersInfoW(SPI_GETHIGHCONTRAST, sizeof(hc), &hc, 0)) {
        return (hc.dwFlags & HCF_HIGHCONTRASTON) != 0;
    }
#endif
    return false;
}

// ============================================================================
// Focus Navigation
// ============================================================================

class FocusManager {
public:
    static FocusManager& instance() {
        static FocusManager m;
        return m;
    }
    
    void register_focusable(NodeID id, int tab_index = -1) {
        FocusableNode node = { id, tab_index >= 0 ? tab_index : next_index_++ };
        focusables_.push_back(node);
    }
    
    void clear() { 
        focusables_.clear(); 
        next_index_ = 0;
    }
    
    NodeID current() const { return current_focus_; }
    
    void focus(NodeID id) {
        current_focus_ = id;
        auto* info = AccessibleStore::instance().get(id);
        if (info) {
            announce(info->name.c_str());
        }
    }
    
    void focus_next() {
        if (focusables_.empty()) return;
        
        // Sort by tab index
        std::sort(focusables_.begin(), focusables_.end(),
            [](const FocusableNode& a, const FocusableNode& b) {
                return a.tab_index < b.tab_index;
            });
        
        // Find current and move to next
        for (size_t i = 0; i < focusables_.size(); i++) {
            if (focusables_[i].id == current_focus_) {
                size_t next = (i + 1) % focusables_.size();
                focus(focusables_[next].id);
                return;
            }
        }
        
        // No current focus, focus first
        if (!focusables_.empty()) {
            focus(focusables_[0].id);
        }
    }
    
    void focus_prev() {
        if (focusables_.empty()) return;
        
        std::sort(focusables_.begin(), focusables_.end(),
            [](const FocusableNode& a, const FocusableNode& b) {
                return a.tab_index < b.tab_index;
            });
        
        for (size_t i = 0; i < focusables_.size(); i++) {
            if (focusables_[i].id == current_focus_) {
                size_t prev = (i == 0) ? focusables_.size() - 1 : i - 1;
                focus(focusables_[prev].id);
                return;
            }
        }
        
        if (!focusables_.empty()) {
            focus(focusables_.back().id);
        }
    }
    
private:
    struct FocusableNode {
        NodeID id;
        int tab_index;
    };
    
    std::vector<FocusableNode> focusables_;
    NodeID current_focus_ = 0;
    int next_index_ = 0;
};

// ============================================================================
// Convenience Functions
// ============================================================================

inline void focus_next() { FocusManager::instance().focus_next(); }
inline void focus_prev() { FocusManager::instance().focus_prev(); }
inline NodeID focused() { return FocusManager::instance().current(); }

} // namespace a11y
} // namespace fanta

