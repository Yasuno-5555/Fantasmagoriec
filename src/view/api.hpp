#include "view/definitions.hpp"
#include "view/api_common.hpp"
#include <functional>

namespace fanta {
    namespace internal {
        struct DrawList;
    }
}

namespace fanta {
    // --- Core Engine API ---
    void Init(int width, int height, const char* title);
    void Shutdown();
    void BeginFrame();
    void EndFrame();
    bool Run(std::function<void()> app_loop);
    bool RunWithUI(std::function<void(::fanta::internal::DrawList&)> ui_loop);
    int GetScreenWidth();
    int GetScreenHeight();

    namespace ui {

    // --- The Universal Masquerade Builders ---

    template<typename T, typename Derived>
    struct ViewBuilder : public LayoutConfig<Derived>, public StyleConfig<Derived> {
        T* view_;
        ViewBuilder(T* v) : view_(v) {}
        
        using LayoutConfig<Derived>::size;
        using StyleConfig<Derived>::size;
        
        operator T*() const { return view_; }
        operator ViewHeader*() const { return view_; }
    };

    struct BoxConfig : public ViewBuilder<BoxView, BoxConfig> {
        using ViewBuilder::ViewBuilder;
    };

    struct TextConfig : public ViewBuilder<TextView, TextConfig> {
        using ViewBuilder::ViewBuilder;
        // Text specific presets/conveniencess could go here
    };

    struct ButtonConfig : public ViewBuilder<ButtonView, ButtonConfig> {
        using ViewBuilder::ViewBuilder;
        
        // Button specific: hover/active colors (Unique state)
        ButtonConfig& hover(internal::ColorF c) { view_->hover_color = c; return *this; }
        ButtonConfig& active(internal::ColorF c) { view_->active_color = c; return *this; }
        
        // Immediate Result (The Lie)
        operator bool() const;
    };

    struct ScrollConfig : public ViewBuilder<ScrollView, ScrollConfig> {
        using ViewBuilder::ViewBuilder;
        ScrollConfig& scrollbar(bool show) { view_->show_scrollbar = show; return *this; }
    };

    struct TextInputConfig : public ViewBuilder<TextInputView, TextInputConfig> {
        using ViewBuilder::ViewBuilder;
        TextInputConfig& placeholder(const char* p) { view_->placeholder = p; return *this; }
    };

    struct TextAreaConfig : public ViewBuilder<TextAreaView, TextAreaConfig> {
        using ViewBuilder::ViewBuilder;
        TextAreaConfig& placeholder(const char* p) { view_->placeholder = p; return *this; }
        TextAreaConfig& line_height(float h) { view_->line_height = h; return *this; }
    };

    struct ToggleConfig : public ViewBuilder<ToggleView, ToggleConfig> {
        using ViewBuilder::ViewBuilder;
    };

    struct SliderConfig : public ViewBuilder<SliderView, SliderConfig> {
        using ViewBuilder::ViewBuilder;
        SliderConfig& range(float min, float max) { view_->min = min; view_->max = max; return *this; }
    };

    struct ContextMenuConfig : public ViewBuilder<ContextMenuView, ContextMenuConfig> {
        using ViewBuilder::ViewBuilder;
        ContextMenuConfig& anchor(float x, float y) { view_->anchor_x = x; view_->anchor_y = y; return *this; }
    };

    struct PlotConfig : public ViewBuilder<PlotView, PlotConfig> {
        using ViewBuilder::ViewBuilder;
        PlotConfig& type(PlotType t) { view_->plot_type = t; return *this; }
        PlotConfig& range(float min, float max) { view_->min_val = min; view_->max_val = max; return *this; }
    };

    struct BezierConfig : public ViewBuilder<BezierView, BezierConfig> {
        using ViewBuilder::ViewBuilder;
        BezierConfig& points(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3) {
            view_->p0[0]=x0; view_->p0[1]=y0;
            view_->p1[0]=x1; view_->p1[1]=y1;
            view_->p2[0]=x2; view_->p2[1]=y2;
            view_->p3[0]=x3; view_->p3[1]=y3;
            return *this;
        }
        BezierConfig& thickness(float t) { view_->thickness = t; return *this; }
    };

    struct TableConfig : public ViewBuilder<TableView, TableConfig> {
        using ViewBuilder::ViewBuilder;
        TableConfig& rows(int count) { view_->row_count = count; return *this; }
        TableConfig& row_height(float h) { view_->row_height = h; return *this; }
    };

    struct SplitterConfig : public ViewBuilder<SplitterView, SplitterConfig> {
        using ViewBuilder::ViewBuilder;
        SplitterConfig& vertical(bool v = true) { view_->is_vertical = v; return *this; }
    };
    
    struct TreeNodeConfig : public ViewBuilder<TreeNodeView, TreeNodeConfig> {
        using ViewBuilder::ViewBuilder;
        TreeNodeConfig& depth(int d) { view_->depth = d; return *this; }
        TreeNodeConfig& indent(float i) { view_->indent_per_level = i; return *this; }
        operator bool() const;  // Returns true if expanded (for block-style API)
    };
    
    struct ColorPickerConfig : public ViewBuilder<ColorPickerView, ColorPickerConfig> {
        using ViewBuilder::ViewBuilder;
        ColorPickerConfig& sv_size(float s) { view_->sv_size = s; return *this; }
        ColorPickerConfig& hue_width(float w) { view_->hue_bar_width = w; return *this; }
    };

    struct MenuBarConfig : public ViewBuilder<MenuBarView, MenuBarConfig> {
        using ViewBuilder::ViewBuilder;
        MenuBarConfig& bar_height(float h) { view_->bar_height = h; return *this; }
    };
    
    struct DragSourceConfig : public ViewBuilder<DragSourceView, DragSourceConfig> {
        using ViewBuilder::ViewBuilder;
    };
    
    struct DropTargetConfig : public ViewBuilder<DropTargetView, DropTargetConfig> {
        using ViewBuilder::ViewBuilder;
        DropTargetConfig& accept(const char* type) { view_->accept_type = type; return *this; }
    };

    // --- Builder Functions ---

    BoxConfig Box();
    BoxConfig Row();
    BoxConfig Column();
    void End();

    TextConfig Text(const char* str);
    ButtonConfig Button(const char* label);
    ScrollConfig Scroll(bool vertical = true);

    TextInputConfig TextInput(char* buffer, size_t size);
    TextAreaConfig TextArea(char* buffer, size_t size);
    
    ToggleConfig Toggle(const char* label, bool& value);
    inline ToggleConfig Toggle(const char* label, bool* value) { 
        static bool dummy = false;
        return Toggle(label, value ? *value : dummy); 
    }

    SliderConfig Slider(const char* label, float& value, float min=0, float max=100);
    inline SliderConfig Slider(const char* label, float* value, float min=0, float max=100) {
        static float dummy = 0;
        return Slider(label, value ? *value : dummy, min, max);
    }

    ContextMenuConfig ContextMenu(bool& is_open, ContextMenuItem* items, size_t count);
    PlotConfig Plot(const float* data, size_t count);
    BezierConfig Bezier();
    TableConfig Table(int row_count, float row_height, RowBuilderFn builder, void* user_data = nullptr);
    SplitterConfig Splitter(float& ratio, bool vertical = false);
    TreeNodeConfig TreeNode(const char* label, bool& expanded);
    inline TreeNodeConfig TreeNode(const char* label, bool* expanded) {
        static bool dummy_expanded = false; 
        return TreeNode(label, expanded ? *expanded : dummy_expanded);
    }

    void TreePop();
    ColorPickerConfig ColorPicker(internal::ColorF& color);
    inline ColorPickerConfig ColorPicker(internal::ColorF* color) {
        static internal::ColorF dummy = {1,1,1,1};
        return ColorPicker(color ? *color : dummy);
    }

    MenuBarConfig BeginMenuBar();
    void EndMenuBar();
    bool BeginMenu(const char* label);
    void EndMenu();
    bool MenuItem(const char* label, const char* shortcut = nullptr);
    void MenuSeparator();

    DragSourceConfig BeginDragSource();
    void EndDragSource();
    DropTargetConfig BeginDropTarget(const char* accept_type);
    bool EndDropTarget(void** out_data = nullptr);

    void SetKeyboardNav(bool enable);
    bool IsKeyboardNavActive();
    const char* AllocString(const char* s);
    void RenderUI(ViewHeader* root, float screen_w, float screen_h, ::fanta::internal::DrawList& dl);

}} // namespace fanta::ui
