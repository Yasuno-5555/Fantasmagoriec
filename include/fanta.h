#pragma once
#include "fanta_id.h"
#include "fanta_reactive.h"

#include <cstdint>
#include <functional>
#include <array>
#include <vector>
#include <utility>
#include <type_traits> // Phase 1: V5 Memory Safety
#include <memory>
#include <string>
#include <map>

namespace fanta {

// --- Opaque Handles ---
struct ElementHandle; 

// --- Enums ---
enum class LayoutMode { Column, Row };

// --- V5: View Architecture ---
// Views are transient, POD descriptions of UI.
// They must be allocated on the FrameArena and trivially destructible.
struct View {
    // Common header for all views (if needed, e.g. type tag)
    // For now, it's a marker base.
};

// Safety Check: Views must be trivially destructible to work with FrameArena (no destructors called)
static_assert(std::is_trivially_destructible_v<View>, "fanta::View must be trivially destructible!");

enum class Align { Start, Center, End, Stretch };
enum class Justify { Start, Center, End, SpaceBetween, SpaceAround };
enum class Wrap { NoWrap, Wrap, WrapReverse };

enum class ColorToken : uint8_t {
    None = 0,
    SystemBackground, SecondarySystemBackground, TertiarySystemBackground,
    Label, SecondaryLabel, TertiaryLabel, QuaternaryLabel,
    Fill, SecondaryFill, Separator, Accent, Destructive,
    Count
};

enum class TextToken : uint8_t {
    None = 0,
    LargeTitle, Title1, Title2, Title3,
    Headline, Body, Callout, Subhead, Footnote, 
    Caption1, Caption2,
    Count
};

// --- Color ---
struct Color { 
    uint32_t rgba = 0;      
    ColorToken token = ColorToken::None;
    bool is_semantic = false;

    constexpr Color() : rgba(0), is_semantic(false) {}
    constexpr Color(uint32_t val) : rgba(val), is_semantic(false) {}
    constexpr Color(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
        : rgba((r << 24) | (g << 16) | (b << 8) | a), is_semantic(false) {}
    constexpr Color(ColorToken t) : token(t), is_semantic(true) {}

    static Color Hex(uint32_t c) { return Color(c); }
    static Color Token(ColorToken t) { return Color(t); }
    static Color White() { return Color(255, 255, 255); }
    static Color Black() { return Color(0, 0, 0); }
    
    static Color Background() { return Color(ColorToken::SystemBackground); }
    static Color Label() { return Color(ColorToken::Label); }
    static Color SecondaryLabel() { return Color(ColorToken::SecondaryLabel); }
    static Color TertiaryLabel() { return Color(ColorToken::TertiaryLabel); }
    static Color Separator() { return Color(ColorToken::Separator); }
    static Color Fill() { return Color(ColorToken::Fill); }
    static Color Accent() { return Color(ColorToken::Accent); }
    static Color Destructive() { return Color(ColorToken::Destructive); }
    static Color SecondaryFill() { return Color(ColorToken::SecondaryFill); }
    static Color SecondarySystemBackground() { return Color(ColorToken::SecondarySystemBackground); }
    static Color TertiarySystemBackground() { return Color(ColorToken::TertiarySystemBackground); }

    Color alpha(float a) const;
};

struct TypographyRule {
    float size = 13.0f;
    float weight = 400.0f;
    float tracking = 0.0f;
    float line_height = 1.2f;
};

// --- Theme ---
struct Theme {
    std::array<uint32_t, static_cast<size_t>(ColorToken::Count)> palette;
    std::array<TypographyRule, static_cast<size_t>(TextToken::Count)> typography;

    uint32_t Resolve(const Color& color) const {
        if (color.is_semantic) {
            return palette[static_cast<size_t>(color.token)];
        }
        return color.rgba;
    }

    static Theme Dark();
    static Theme Light();
    static Theme AppleHIG(); 
    static Theme Fantasmagorie(); 
    static Theme HighContrast();
};

enum class LifeCycle { Suspend, Resume, Background, Foreground };
enum class AccessibilityRole { StaticText, Button, Checkbox, Slider, Header, Image, Link };

enum class MaterialType { 
    None, 
    UltraThin, 
    Thin, 
    Regular, 
    Thick, 
    Chrome // iOS-style opaque blurred chrome
};

enum class KeyboardType { Default, ASCII, Number, Email, Password, URL };

// --- Platform Utilities ---
namespace Native {
    void HapticFeedback(int intensity = 1); // 0=Light, 1=Medium, 2=Heavy
    void ShowToast(const std::string& message);
    void Announce(const std::string& message); // Accessibility announcement
    std::string GetAppVersion();
    bool IsDarkMode();
    
    // Phase 41: Mobile Primacy
    void ShowKeyboard(KeyboardType type = KeyboardType::Default);
    void HideKeyboard();
    void SetIMEPos(float x, float y); // Suggest IME position
}

// Phase 23: Native Plugins
namespace Plugins {
    bool Load(const std::string& path);
    void UnloadAll();
}

// Phase 24: Scripting
namespace Scripting {
    bool Run(const std::string& code);
    bool LoadFile(const std::string& path);
}

// --- Localization ---
namespace L10n {
    void SetLocale(const std::string& locale);
    void AddStrings(const std::string& locale, const std::map<std::string, std::string>& strings);
    std::string Translate(const std::string& key);
}
inline std::string t(const std::string& key) { return L10n::Translate(key); }

/**
 * Fantasmagorie Crystal Public Facade
 */

// --- Spacing ---
struct Spacing {
    static constexpr float XS() { return 4.0f; }  
    static constexpr float S()  { return 8.0f; }  
    static constexpr float M()  { return 16.0f; } 
    static constexpr float L()  { return 24.0f; } 
    static constexpr float XL() { return 32.0f; } 
    static constexpr float XXL(){ return 48.0f; } 
};

// --- API ---
struct Rect { float x, y, w, h; };
Rect GetLayout(ID id);
bool IsDown(ID id);
bool IsSelected(ID id);
bool IsClicked(ID id);
std::vector<std::pair<ID, ID>> GetConnectionEvents();

// --- Builder ---
struct Element {
    ElementHandle* handle = nullptr;

    Element(ID id);
    
    Element& label(const char* text);
    Element& size(float w, float h);
    Element& width(float w);
    Element& height(float h);
    
    Element& row(); 
    Element& column(); 
    Element& wrap(bool enable = true);
    Element& padding(float p);
    Element& gap(float g);
    Element& flexGrow(float grow);
    Element& alignContent(Align alignment);
    Element& justify(Justify j);
    Element& align(Align a);
    
    // Positioning
    Element& absolute(); 
    Element& top(float v);
    Element& bottom(float v);
    Element& left(float v);
    Element& right(float v);
    
    Element& bg(Color c);
    Element& color(Color c); 
    Element& fontSize(float size);
    Element& textStyle(TextToken t);
    Element& rounded(float radius, bool squircle = true); // Phase 21: Squircle by default
    Element& backdropBlur(float sigma); // Phase 21: Glassmorphism
    Element& vibrancy(float amount);    // Phase 21: Text/Ui Vibrancy
    Element& material(MaterialType type); // Phase 21: Acrylic materials
    Element& elevation(float e);
    Element& line();
    
    Element& hoverBg(Color c);
    Element& activeBg(Color c);
    Element& hoverLift(float delta); // removed impl?
    Element& activeScale(float delta);
    
    Element& focusable(bool v = true);
    Element& focusBg(Color c);
    Element& focusLift(float delta);
    
    Element& canvas(float zoom, float pan_x, float pan_y);
    Element& node(float x, float y);
    Element& port();
    Element& slider(float& value, float min, float max);
    Element& toggle(bool& value, const char* label = nullptr);
    Element& wire(float x0, float y0, float x1, float y1, float x2, float y2, float x3, float y3, float thickness=2.0f);
    Element& scrollable(bool vertical=true, bool horizontal=false);
    Element& markdown(const char* md_text);
    Element& popup();
    
    Element& blur(float radius);
    Element& backdrop(bool enable = true);

    Element& onClick(std::function<void()> callback);
    Element& disabled(bool enable = true);

    Element& gridOffset(int r, int c, int rs=1, int cs=1); // Phase 10
    
    // Phase 14: Advanced Features
    Element& path(const std::string& svg_path, uint32_t color=0xFFFFFFFF, float stroke=1.0f, bool fill=true);
    Element& richText(const std::string& markup);
    Element& constraint(ID other, int type, float param); // type: 0=Dist, 1=MinDist, etc.

    // Phase 15: Mobile Support
    Element& safeArea(bool enable = true);
    Element& onLifeCycle(std::function<void(LifeCycle)> callback);

    // Phase 17: Accessibility Support
    Element& accessibilityLabel(const std::string& label);
    Element& accessibilityHint(const std::string& hint);


    // Phase 18: RTL Support
    Element& rtl(bool enable = true);

    // Phase 20: Flutter-like Animations
    Element& animate(float duration_ms, int curve_type = 2); // 2=EaseOut
    Element& hero(const std::string& tag); // Phase 22: Shared element transition
    Element& script(const std::string& source); // Phase 24: Embedded logic
    
    // Phase 1: TextInput & TreeNode
    Element& textInput(std::string& value, const char* placeholder = nullptr);
    Element& treeNode(bool& expanded, const char* label);
    
    // Missing activeElevation/hoverElevation from previous usage?
    Element& activeElevation(float e);
    Element& hoverElevation(float e);
    
    // Phase 2: Table
    Element& beginTable(int columns);
    Element& tableHeader(const char* label, float width = 0);
    Element& tableCell();
    Element& tableRow();
    
    // Phase 3: Context Menu
    Element& contextMenu();
    Element& menuItem(const char* label, const char* shortcut = nullptr);
    
    // Phase 4: Menu Bar
    Element& beginMenuBar();
    Element& menu(const char* label);
    // Phase 37: Virtualization
    Element& virtualList(size_t count, float item_height, std::function<void(size_t index, Element& item)> item_builder);
    
    // Phase 36: Reactive Binding
    template<typename T>
    Element& bind(Observable<T>& obs) {
        return *this; // Specialized below
    }

    Element& bind(Observable<float>& obs);
    Element& bind(Observable<bool>& obs);
    Element& bind(Observable<std::string>& obs);

    // Stage 3: Professional Productivity
    Element& beginGroup(const char* label);
    Element& endGroup();
    Element& property(const char* label, float& value, float min=0, float max=1);
    Element& property(const char* label, bool& value);
    Element& property(const char* label, std::string& value);
    Element& property(const char* label, Color& value);
    
    // Phase 35: Accessibility
    Element& accessibleName(const char* name);
    Element& role(AccessibilityRole role);
    
    Element& shader(const std::string& source);

    Element& endMenuBar();
};

void Init(int width, int height, const char* title);
void Shutdown();
void SetTheme(const Theme& theme);
Theme GetTheme();
bool Run(std::function<void()> app_loop);

// Phase 31: Multi-Context API
struct Context; // Opaque handle
struct ContextConfig {
    int width = 1280;
    int height = 720;
    const char* title = "Fantasmagorie";
};
Context* CreateContext(const ContextConfig& config = {});
void MakeContextCurrent(Context* ctx);
Context* GetCurrentContext();
void DestroyContext(Context* ctx);

void GetMousePos(float& x, float& y);
bool IsMouseDown();
bool IsMouseJustPressed();
bool IsKeyDown(int key);
bool IsKeyJustPressed(int key);

void BeginFrame();
void EndFrame();

bool CaptureScreenshot(const char* filename);
void SetDebugHitTest(bool enable);

// ID Stack
void PushID(ID id);
void PopID();
void PushID(const char* str_id); // Helper shorthand

// Input Capture (V2-B)
void Capture(ID id);
void Release();
bool IsCaptured(ID id);
bool IsAnyCaptured();

// Phase 1: Native Dialogs
std::string OpenFileDialog(const char* filter = nullptr, const char* title = "Open File");
std::string SaveFileDialog(const char* filter = nullptr, const char* title = "Save File");
std::string OpenFolderDialog(const char* title = "Select Folder");

// Phase 4: Clipboard
void SetClipboardText(const char* text);
std::string GetClipboardText();
bool HasClipboardText();

// Phase 4: Undo/Redo
class Command {
public:
    virtual ~Command() = default;
    virtual void execute() = 0;
    virtual void undo() = 0;
};
void PushCommand(std::unique_ptr<Command> cmd);
void Undo();
void Redo();
bool CanUndo();
bool CanRedo();

} // namespace fanta
