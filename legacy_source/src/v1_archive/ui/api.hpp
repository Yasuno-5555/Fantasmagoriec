#pragma once
#include <string>
#include <cstdint>
#include <vector>

struct UIContext;
struct FocusManager;
struct InputDispatcher;

namespace ui {
    // Forward Decls
    void SetContext(UIContext* ctx);
    void SetInteraction(FocusManager* f, InputDispatcher* i);
    
    // Core Loop
    void UpdateInputState(void* window_handle, float mouse_x, float mouse_y, bool mouse_down, float scroll_y = 0.0f);
    
    // Phase 18-C
    void BeginChild(const char* id, float w, float h);
    void EndChild(); 

    // Phase 1: Transform API
    // (Wrappers for UIContext transform stack)
    void PushTransform(float* m33); // generic 3x3 array
    void PushTranslate(float x, float y);
    void PushScale(float s);
    void PopTransform();

    // Phase 1: Inline Draw API
    // Coordinates are transformed by the current Transform Stack
    void DrawLine(float x1, float y1, float x2, float y2, uint32_t color, float thickness=1.0f);
    void DrawRect(float x, float y, float w, float h, uint32_t color, float border_radius=0.0f);
    void DrawRectFilled(float x, float y, float w, float h, uint32_t color, float border_radius=0.0f);
    void DrawBezier(float x1, float y1, float x2, float y2, float cx1, float cy1, float cx2, float cy2, uint32_t color, float thickness=1.0f);
}


#include "../core/style.hpp"

namespace ui {
    void Begin(const char* id); 
    void End();

    void Column(); 
    void Row();
    
    void BeginGroup(const char* id);
    void EndGroup();

    // Constraint API
    enum class Align { Start, Center, End, Stretch };

    void SetNextWidth(float w);
    void SetNextHeight(float h); // 0 = Auto
    void SetNextGrow(float factor); 
    void SetNextPadding(float px);
    void SetNextAlign(Align align); 
    
    // Phase 20
    void SetNextPosition(int type, float x, float y); // 0=Rel, 1=Abs

    void SetNextBg(Color c);
    void SetNextBgHover(Color c);
    void SetNextBgActive(Color c);
    void SetNextTextColor(Color c);
    void SetNextRadius(float r);
    void SetNextBorder(float width, Color color);
    void SetNextFocusRing(Color color, float width);

    void PushStyle(const Style& style);
    void PopStyle();
    
    // Phase 2: Style Stack
    enum class StyleColor {
        Text,
        Bg,
        BgHover,
        BgActive,
        Border,
        Accent,
        WindowBg,
        TitleBg
    };
    void PushStyleColor(StyleColor idx, Color c);
    void PopStyleColor();

    struct StyleScope {
        StyleScope(const Style& s) { PushStyle(s); }
        ~StyleScope() { PopStyle(); }
        StyleScope(const StyleScope&) = delete;
        StyleScope& operator=(const StyleScope&) = delete;
    };

    // Widgets
    bool Draggable(const char* id, float* x, float* y);
    void PanZoom(float* pan_x, float* pan_y, float* zoom);
    void CanvasGrid(float zoom, float step_size=100.0f);
    
    // Node Widgets
    void BeginNode(const char* id, float* x, float* y);
    void EndNode();
    void GetLastNodeSize(float* w, float* h);
    void NodePin(const char* label, bool is_input);
    
    // Connections
    void DrawConnection(float x1, float y1, float x2, float y2, uint32_t color, float thickness=3.0f);
    
    // Simplified Slider (Visual only)(const char* label);
    bool Button(const char* label);
    bool PrimaryButton(const char* label);
    bool DangerButton(const char* label);
    void Label(const char* text);
    
    // Phase 17: Settings Primitives
    bool Checkbox(const char* label, bool* v);
    bool Slider(const char* label, float* v, float v_min, float v_max);
    bool ColorEdit(const char* label, Color* c);
    
    // Phase 16: Tooltips
    void SetTooltip(const char* text);
    
    // Phase Visuals: Media
    // Phase Visuals: Media
    void DrawImage(void* texture_id, float w, float h);
    void DrawImage(void* texture_id, float w, float h, float u0, float v0, float u1, float v1);
    
    // Core Widget
    void Image(uint32_t texture_id, float w, float h);
    
    // Rich Text
    void Markdown(const char* text);
    const char* Loc(const char* key);
    
    // Async
    void Spinner();

    enum class InputResult { None, Changed, Submitted };
    InputResult TextInput(const char* id, std::string& buffer);
    void Text(const char* content);
    void Spacer(float size);
    
    bool IsKeyDown(int key);
    bool IsKeyPressed(int key);

    void BeginWindow(const char* title, bool* open = nullptr);
    void EndWindow();

    void OpenPopup(const char* id);
    bool BeginPopup(const char* id);
    void EndPopup();
    void CloseCurrentPopup();

    // Phase 1: Essential Widgets
    bool BeginCombo(const char* label, const char* preview_value);
    void EndCombo();

    bool BeginMenuBar();
    void EndMenuBar();
    bool BeginMenu(const char* label);
    void EndMenu();
    bool MenuItem(const char* label, const char* shortcut = nullptr, bool selected = false);
    
    // Horizontal or Vertical Splitter
    // 'size_var' is the pointer to the dimension of the *first* element (Left or Top).
    void Splitter(const char* id, float* size_var, bool vertical);


    // Phase 15
    bool BeginTable(const char* id, int col_count);
    void EndTable();
    void TableNextRow();
    void TableNextColumn();
    
    void BeginTabBar(const char* id);
    void EndTabBar();
    bool TabItem(const char* label); 
    bool IsTabActive(const char* label); 

    void TextColored(const char* text, Color c);
    void TextPositive(const char* text);
    void TextNegative(const char* text);
    void TextDisabled(const char* text);

    // System
    void SetIMEPosition(float x, float y);

    // Advanced Phase 1
    bool Combo(const char* label, int* current_item, const std::vector<std::string>& items);
    void SetScrollX(float x);
    void SetScrollY(float y);
    float GetScrollX();
    void SetScrollY(float y);
    float GetScrollX();
    float GetScrollY();
    
    // Phase 2: Structure
    bool BeginMainMenuBar();
    void EndMainMenuBar();
    bool BeginStatusBar();
    void EndStatusBar();
    void DockSpace(const char* id); // Placeholder for central area
    
    // Phase 2: Scroll
    void BeginScrollArea(const char* id, float w, float h, float* scroll_x, float* scroll_y);
    void EndScrollArea();

    // Phase 3: Complex
    bool TreeNode(const char* label, bool* selected = nullptr, int flags = 0);
    void TreePop();
    bool OpenFileDialog(const char* title, const char* filters, std::string& out_path);
    
    // Clipboard
    void SetClipboardText(const char* text);
    std::string GetClipboardText();
    
    void EndFrame();
}
