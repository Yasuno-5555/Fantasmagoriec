#include "docking.hpp"
#include <fanta.h>
#include "../core/contexts_internal.hpp"
#include <algorithm>

namespace fanta {
namespace internal {

static void RenderDockNode(DockNode& node, float x, float y, float w, float h) {
    if (node.type == DockType::Split) {
        float ratio = node.split_ratio;
        bool horiz = (node.split_dir == SplitDir::Horizontal);
        
        float split_pos = horiz ? w * ratio : h * ratio;
        float splitter_size = 4.0f;
        
        // Render Child A
        if (node.child_a) {
            float aw = horiz ? split_pos : w;
            float ah = horiz ? h : split_pos;
            RenderDockNode(*node.child_a, x, y, aw, ah);
        }
        
        // Render Splitter
        float sx = horiz ? x + split_pos : x;
        float sy = horiz ? y : y + split_pos;
        float sw = horiz ? splitter_size : w;
        float sh = horiz ? h : splitter_size;
        
        PushID(node.id);
        Element sep(ID("splitter"));
        sep.absolute().top(sy).left(sx).size(sw, sh).bg(Color(40, 40, 40));
        
        // Splitter Logic
        if (sep.handle) {
            auto& el = GetStore().frame_elements[reinterpret_cast<size_t>(sep.handle) - 1];
            if (el.is_active) {
                float mx, my;
                GetMousePos(mx, my);
                if (horiz) node.split_ratio = std::clamp((mx - x) / w, 0.05f, 0.95f);
                else node.split_ratio = std::clamp((my - y) / h, 0.05f, 0.95f);
            }
        }
        PopID();

        // Render Child B
        if (node.child_b) {
            float bx = horiz ? x + split_pos + splitter_size : x;
            float by = horiz ? y : y + split_pos + splitter_size;
            float bw = horiz ? w - split_pos - splitter_size : w;
            float bh = horiz ? h : h - split_pos - splitter_size;
            RenderDockNode(*node.child_b, bx, by, bw, bh);
        }
    } else if (node.type == DockType::Tabs) {
        // Tab Bar
        float tab_h = 30.0f;
        PushID(node.id);
        Element bar(ID("tabbar"));
        bar.absolute().top(y).left(x).size(w, tab_h).row().bg(Color(30, 30, 30));
        
        for (int i = 0; i < (int)node.tabs.size(); ++i) {
            PushID(i);
            Element tab(ID("tab"));
            tab.label(node.tabs[i].label.c_str()).padding(8);
            if (i == node.active_tab) tab.bg(Color(60, 60, 60));
            if (IsClicked(tab.id)) node.active_tab = i;
            PopID();
        }
        PopID();

        // Active Content
        if (node.active_tab >= 0 && node.active_tab < (int)node.tabs.size()) {
            float content_y = y + tab_h;
            float content_h = h - tab_h;
            PushID(node.active_tab);
            Element content_box(ID("content"));
            content_box.absolute().top(content_y).left(x).size(w, content_h);
            if (node.tabs[node.active_tab].builder) {
                node.tabs[node.active_tab].builder();
            }
            PopID();
        }
    }
}

} // namespace internal

Element& Element::beginDockSpace(const char* name) {
    auto& s = internal::GetState(handle);
    s.is_dock_space = true;
    
    // Logic: fetch or create dock tree
    auto* space = internal::DockManager::Get().get_or_create_space(name);
    if (space && space->root) {
        // Trigger recursive rendering
        // Need current absolute layout of this element
        internal::RuntimeState& runtime = internal::GetRuntime();
        if (runtime.layout_results.count(s.id)) {
            auto const& layout = runtime.layout_results.at(s.id);
            internal::RenderDockNode(*space->root, layout.x, layout.y, layout.w, layout.h);
        }
    }
    return *this;
}

Element& Element::dockItem(const char* label, std::function<void()> content) {
    // This adds a potential dock item to the current tree if it doesn't exist
    // For now, very naive: just ensure it's in the root tabs if root is empty
    auto* space = internal::DockManager::Get().get_or_create_space("main"); // FIX: use current space
    if (space->root->type == internal::DockType::Content) {
        space->root->type = internal::DockType::Tabs;
    }
    
    bool found = false;
    for (auto& t : space->root->tabs) {
        if (t.label == label) {
            t.builder = content; // Update callback
            found = true;
            break;
        }
    }
    
    if (!found) {
        space->root->tabs.push_back({label, content});
    }
    return *this;
}

} // namespace fanta
