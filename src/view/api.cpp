#include "view/api.hpp"
#include "view/interaction.hpp"
#include "core/contexts_internal.hpp"
#include <cstring>

namespace fanta {
namespace ui {

    // --- Internal Helpers ---
    namespace {
        internal::EngineContext& GetCtx() {
            return internal::GetEngineContext();
        }
        
        void PushParent(void* p) { 
            if (p) GetCtx().frame.parent_stack.push_back(internal::ID((uint64_t)p)); 
        }
        void PopParent() { 
            if (!GetCtx().frame.parent_stack.empty()) GetCtx().frame.parent_stack.pop_back(); 
        }
        BoxView* TopParent() { 
            auto& stack = GetCtx().frame.parent_stack;
            return stack.empty() ? nullptr : (BoxView*)(stack.back().value); 
        }
        
        template<typename T>
        T* AllocView(ViewType type) {
            auto* view = GetCtx().frame.arena.alloc<T>();
            if (!view) return nullptr;
            view->type = type;
            view->id = internal::ID(internal::GetGlobalIDCounter()++);
            
            // Auto-link to current parent
            if (auto* p = TopParent()) p->add(view);
            
            return view;
        }
    }

    // --- Builder Implementations ---

    ButtonConfig::operator bool() const {
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.get_rect(view_->id);
        interaction::UpdateHitTest(view_->id, last_rect.x, last_rect.y, last_rect.w, last_rect.h);
        return interaction::IsClicked(view_->id);
    }

    TreeNodeConfig::operator bool() const {
        return view_ && view_->expanded && *view_->expanded;
    }

    // --- Memory Utilities ---
    const char* AllocString(const char* s) {
        return GetCtx().frame.arena.alloc_string(s);
    }

    // --- Public Builder API ---

    BoxConfig Box() {
        return BoxConfig(AllocView<BoxView>(ViewType::Box));
    }
    
    BoxConfig Row() {
        auto* view = AllocView<BoxView>(ViewType::Box);
        view->is_row = true;
        PushParent(view);
        return BoxConfig(view);
    }
    
    BoxConfig Column() {
        auto* view = AllocView<BoxView>(ViewType::Box);
        view->is_row = false;
        PushParent(view);
        return BoxConfig(view);
    }

    void End() {
        PopParent();
    }

    TextConfig Text(const char* str) {
        auto* view = AllocView<TextView>(ViewType::Text);
        view->text = AllocString(str);
        return TextConfig(view);
    }

    ButtonConfig Button(const char* label) {
        auto* view = AllocView<ButtonView>(ViewType::Button);
        view->label = AllocString(label);
        return ButtonConfig(view);
    }

    ScrollConfig Scroll(bool vertical) {
        auto* view = AllocView<ScrollView>(ViewType::Scroll);
        view->is_row = !vertical;
        interaction::HandleScrollInteraction(view);
        PushParent(view);
        return ScrollConfig(view);
    }

    TextInputConfig TextInput(char* buffer, size_t size) {
        auto* view = AllocView<TextInputView>(ViewType::TextInput);
        view->buffer = buffer;
        view->buffer_size = size;
        
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.get_rect(view->id);
        interaction::UpdateHitTest(view->id, last_rect.x, last_rect.y, last_rect.w, last_rect.h);
        
        if (interaction::IsFocused(view->id)) {
            for (uint32_t c : ctx.input.chars) {
                if (c >= 32) {
                    size_t len = strlen(buffer);
                    if (len + 1 < size) {
                        buffer[len] = (char)c;
                        buffer[len+1] = '\0';
                    }
                }
            }
            if (!ctx.input.ime_result.empty()) {
                size_t len = strlen(buffer);
                size_t ime_len = ctx.input.ime_result.size();
                if (len + ime_len < size) {
                    strcat(buffer, ctx.input.ime_result.c_str());
                }
            }
            for (int key : ctx.input.keys) {
                if (key == 259) { // Backspace
                    size_t len = strlen(buffer);
                    if (len > 0) {
                        len--;
                        while (len > 0 && (buffer[len] & 0xC0) == 0x80) len--;
                        buffer[len] = '\0';
                    }
                }
            }
        }
        return TextInputConfig(view);
    }

    TextAreaConfig TextArea(char* buffer, size_t size) {
        auto* view = AllocView<TextAreaView>(ViewType::TextArea);
        view->buffer = buffer;
        view->buffer_size = size;
        
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.get_rect(view->id);
        interaction::UpdateHitTest(view->id, last_rect.x, last_rect.y, last_rect.w, last_rect.h);
        
        if (interaction::IsFocused(view->id)) {
            for (uint32_t c : ctx.input.chars) {
                if (c >= 32) {
                    size_t len = strlen(buffer);
                    if (len + 1 < size) {
                        buffer[len] = (char)c;
                        buffer[len+1] = '\0';
                    }
                }
            }
            if (!ctx.input.ime_result.empty()) {
                size_t len = strlen(buffer);
                size_t ime_len = ctx.input.ime_result.size();
                if (len + ime_len < size) {
                    strcat(buffer, ctx.input.ime_result.c_str());
                }
            }
            for (int key : ctx.input.keys) {
                if (key == 259) { // Backspace
                    size_t len = strlen(buffer);
                    if (len > 0) {
                        len--;
                        while (len > 0 && (buffer[len] & 0xC0) == 0x80) len--;
                        buffer[len] = '\0';
                    }
                } else if (key == 257 || key == 335) { // Enter
                    size_t len = strlen(buffer);
                    if (len + 1 < size) {
                        buffer[len] = '\n';
                        buffer[len+1] = '\0';
                    }
                }
            }
        }
        return TextAreaConfig(view);
    }

    ToggleConfig Toggle(const char* label, bool& value) {
        auto* view = AllocView<ToggleView>(ViewType::Toggle);
        view->label = AllocString(label);
        view->value = &value;
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.get_rect(view->id);
        interaction::UpdateHitTest(view->id, last_rect.x, last_rect.y, last_rect.w, last_rect.h);
        if (interaction::IsClicked(view->id)) value = !value;
        return ToggleConfig(view);
    }

    SliderConfig Slider(const char* label, float& value, float min, float max) {
        auto* view = AllocView<SliderView>(ViewType::Slider);
        view->label = AllocString(label);
        view->value = &value;
        view->min = min;
        view->max = max;
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.get_rect(view->id);
        interaction::UpdateHitTest(view->id, last_rect.x, last_rect.y, last_rect.w, last_rect.h);
        if (interaction::IsActive(view->id)) {
            float local_x = ctx.input.mouse_x - last_rect.x;
            float t = local_x / last_rect.w;
            if (t < 0) t = 0; if (t > 1) t = 1;
            value = min + (max - min) * t;
        }
        return SliderConfig(view);
    }

    ContextMenuConfig ContextMenu(bool& is_open, ContextMenuItem* items, size_t count) {
        auto* view = AllocView<ContextMenuView>(ViewType::ContextMenu);
        view->is_open = &is_open;
        view->items = items;
        view->item_count = count;
        auto& ctx = internal::GetEngineContext();
        view->anchor_x = ctx.input.mouse_x;
        view->anchor_y = ctx.input.mouse_y;
        return ContextMenuConfig(view);
    }

    PlotConfig Plot(const float* data, size_t count) {
        auto* view = AllocView<PlotView>(ViewType::Plot);
        view->data = data;
        view->data_count = count;
        if (data && count > 0) {
            view->min_val = data[0]; view->max_val = data[0];
            for (size_t i = 1; i < count; ++i) {
                if (data[i] < view->min_val) view->min_val = data[i];
                if (data[i] > view->max_val) view->max_val = data[i];
            }
        }
        return PlotConfig(view);
    }

    BezierConfig Bezier() {
        auto* view = AllocView<BezierView>(ViewType::Bezier);
        view->fg_color = {1,1,1,1};
        view->thickness = 2.0f;
        for(int i=0; i<2; ++i) { view->p0[i]=0; view->p1[i]=0; view->p2[i]=0; view->p3[i]=0; }
        return BezierConfig(view);
    }

    TableConfig Table(int row_count, float row_height, RowBuilderFn builder, void* user_data) {
        auto* view = AllocView<TableView>(ViewType::Table);
        view->row_count = row_count;
        view->row_height = row_height;
        view->row_builder = builder;
        view->user_data = user_data;
        return TableConfig(view);
    }

    SplitterConfig Splitter(float& ratio, bool vertical) {
        auto* view = AllocView<SplitterView>(ViewType::Splitter);
        view->split_ratio = &ratio;
        view->is_vertical = vertical;
        view->is_row = !vertical; 
        PushParent(view);
        auto& ctx = internal::GetEngineContext();
        auto rect = ctx.persistent.get_rect(view->id);
        if (rect.w > 0) {
            float handle_pos = view->is_vertical ? (rect.h * ratio) : (rect.w * ratio);
            float h_x = view->is_vertical ? rect.x : rect.x + handle_pos - 4;
            float h_y = view->is_vertical ? rect.y + handle_pos - 4 : rect.y;
            float h_w = view->is_vertical ? rect.w : 8;
            float h_h = view->is_vertical ? 8 : rect.h;
            interaction::UpdateHitTest(view->id, h_x, h_y, h_w, h_h);
            if (interaction::IsActive(view->id)) {
                if (view->is_vertical) ratio = (ctx.input.mouse_y - rect.y) / rect.h;
                else ratio = (ctx.input.mouse_x - rect.x) / rect.w;
                if (ratio < 0.1f) ratio = 0.1f; if (ratio > 0.9f) ratio = 0.9f;
            }
        }
        return SplitterConfig(view);
    }

    TreeNodeConfig TreeNode(const char* label, bool& expanded) {
        auto* view = AllocView<TreeNodeView>(ViewType::TreeNode);
        view->label = label;
        view->expanded = &expanded;
        view->is_row = true;  
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.last_frame_rects[view->id.value];
        if (last_rect.w > 0 && interaction::IsClicked(view->id)) expanded = !expanded;
        if (expanded) {
            auto* content = AllocView<BoxView>(ViewType::Box);
            content->is_row = false;
            content->padding = 0;
            content->margin = 0;
            PushParent(content);
        }
        return TreeNodeConfig(view);
    }
    
    void TreePop() { PopParent(); }

    ColorPickerConfig ColorPicker(internal::ColorF& color) {
        auto* view = AllocView<ColorPickerView>(ViewType::ColorPicker);
        view->color = &color;
        view->hsv = HSV::FromRGB(color.r, color.g, color.b, color.a);
        auto& ctx = internal::GetEngineContext();
        auto last_rect = ctx.persistent.last_frame_rects[view->id.value];
        if (last_rect.w > 0) {
            float sv_size = view->sv_size;
            float hue_x = last_rect.x + sv_size + 8;
            float hue_w = view->hue_bar_width;
            if (interaction::MouseDown()) {
                float mx = ctx.input.mouse_x, my = ctx.input.mouse_y;
                if (mx >= last_rect.x && mx < last_rect.x + sv_size && my >= last_rect.y && my < last_rect.y + sv_size) {
                    view->hsv.s = (mx - last_rect.x) / sv_size;
                    view->hsv.v = 1.0f - (my - last_rect.y) / sv_size;
                    color = view->hsv.ToRGB();
                } else if (mx >= hue_x && mx < hue_x + hue_w && my >= last_rect.y && my < last_rect.y + sv_size) {
                    view->hsv.h = 360.0f * (my - last_rect.y) / sv_size;
                    color = view->hsv.ToRGB();
                }
            }
        }
        return ColorPickerConfig(view);
    }

    MenuBarConfig BeginMenuBar() {
        auto* view = AllocView<MenuBarView>(ViewType::MenuBar);
        view->is_row = true;
        GetCtx().frame.current_menu_bar = view->id.value;
        PushParent(view);
        return MenuBarConfig(view);
    }
    
    void EndMenuBar() { PopParent(); GetCtx().frame.current_menu_bar = 0; }
    
    bool BeginMenu(const char* label) {
        auto* btn = AllocView<ButtonView>(ViewType::Button);
        btn->label = label;
        auto& ctx = GetCtx();
        auto last_rect = ctx.persistent.last_frame_rects[btn->id.value];
        bool is_open = (last_rect.w > 0 && interaction::IsClicked(btn->id));
        if (is_open) {
            auto* menu = AllocView<BoxView>(ViewType::Box);
            menu->is_row = false;
            ctx.frame.current_menu = menu->id.value;
            PushParent(menu);
        }
        return is_open;
    }
    
    void EndMenu() {
        auto& ctx = GetCtx();
        if (ctx.frame.current_menu != 0) { PopParent(); ctx.frame.current_menu = 0; }
    }
    
    bool MenuItem(const char* label, const char* shortcut) {
        auto* item = AllocView<MenuItemView>(ViewType::MenuItem);
        item->label = label; item->shortcut = shortcut;
        auto& ctx = GetCtx();
        auto last_rect = ctx.persistent.last_frame_rects[item->id.value];
        return (last_rect.w > 0 && interaction::IsClicked(item->id));
    }
    
    void MenuSeparator() {
        auto* item = AllocView<MenuItemView>(ViewType::MenuItem);
        item->is_separator = true;
    }

    DragSourceConfig BeginDragSource() {
        auto* view = AllocView<DragSourceView>(ViewType::DragSource);
        auto& ctx = GetCtx();
        ctx.frame.drag_source = view->id.value;
        ctx.frame.drag_payload = view->payload_data;
        return DragSourceConfig(view);
    }
    
    void EndDragSource() { GetCtx().frame.drag_source = 0; }
    
    DropTargetConfig BeginDropTarget(const char* accept_type) {
        auto* view = AllocView<DropTargetView>(ViewType::DropTarget);
        view->accept_type = accept_type;
        return DropTargetConfig(view);
    }
    
    bool EndDropTarget(void** out_data) {
        auto& ctx = GetCtx();
        if (ctx.frame.drag_source != 0 && !ctx.input.mouse_down && out_data) {
            *out_data = ctx.frame.drag_payload;
            return true;
        }
        return false;
    }

    void SetKeyboardNav(bool enable) { GetCtx().runtime.keyboard_nav_enabled = enable; }
    bool IsKeyboardNavActive() { return GetCtx().runtime.keyboard_nav_enabled; }

} // namespace ui
} // namespace fanta
