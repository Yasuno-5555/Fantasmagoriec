#include "fanta.h"
#include <span>
#include <string_view>
#include <functional>

namespace fanta {

    // --- V5 Node Types (POD) ---
    
    using StringView = std::string_view;
    template<typename T> using Span = std::span<T>;

    // --- V5 Node Types (POD) ---
    // All nodes inherit from View (marker) and must be trivially destructible.

    enum class NodeType { Generic, Container, Text, Button };

    struct Node : public View {
        NodeType type = NodeType::Generic;
        
        // Core Identity
        ID id = {}; // 0 = Auto
        
        // Layout
        float width = 0; // 0 = Auto
        float height = 0;
        float flex_grow = 0.0f;
        LayoutMode layout_mode = LayoutMode::Column;
        Align align_self = Align::Start;
        
        // Style
        Color bg_color = Color::Token(ColorToken::None);
        float padding = 0;
        float margin = 0;
    };

    struct Container : public Node {
        Span<Node*> children; // Array of pointers to nodes
        
        Align align_items = Align::Start;
        Justify justify_content = Justify::Start;
        float gap = 0;
    };

    struct Text : public Node {
        StringView content;
        Color color = Color::Token(ColorToken::Label);
        float size = 16.0f;
        TextToken font_token = TextToken::Body;
    };

    struct Button : public Container {
        std::function<void()> on_click; // Wait! std::function is NOT trivially destructible.
        // STOP.
        // IsTriviallyDestructible check will fail if we use std::function.
        // V5 Principle: "View must be POD".
        // How to handle callbacks?
        // Option 1: Function Pointers (C-style). Safe but limited (no closure captures).
        // Option 2: Store callback in a separate side-table (Id -> Callback) using the ID.
        // Option 3: Use a custom POD callback wrapper (e.g. captures raw pointer + function pointer).
        
        // For Phase 3, let's use Option 1 (Function Pointer + UserData) or just standard C++ function pointer.
        // Or, we violate the trivial destructor rule for *just* this field? 
        // No, FrameArena::reset() assumes no destructors. std::function destructor might be needed to release captures.
        // If we use simple lambdas without captures, it might be fine, but std::function wraps them.
        
        // Decision: Events are signals.
        // But for "lazy" porting, let's use a "SignalID" or "EventID".
        // Or, we store the callback in the *Persistant State*?
        // Let's use a raw function pointer for now: void(*on_click)(void* user_data).
        
        // Revision:
        // We will store an ID to an "Action" or simply ignore callbacks here and handle them via "Signals" (Phase 7).
        // But we need buttons now.
        // Let's use a simple listener interface or "OnClick" that takes an ID.
        
        // Temporary Hack for Phase 3:
        // Raw pointer to a callback struct allocated elsewhere?
        // Or just disable the static_assert for Buttons? No, that's dangerous.
        
        // Let's look at how IMGUI does it: it returns `bool`.
        // But V5 is "View as Data".
        // React-style: `on_click` prop.
        
        // Solution: Store a raw pointer to a `fanta::EventHandler`. 
        // Allocating the handler on Arena? Handler must be POD?
        
        // Let's go with "Callbacks are not part of View Data" for strict V5?
        // No, that makes it useless.
        // Let's use a `void*` context + `void(*)(void*)` function.
        // `Callback click_handler;` where Callback is POD.
    };
    
    struct Callback {
        void (*func)(void*) = nullptr;
        void* data = nullptr;
        
        void invoke() const { if(func) func(data); }
    };
    
    struct ButtonNode : public Container {
        Callback on_click;
    };

    // --- Builders ---
    // These allocate on the current FrameArena
    
    Node* Box(const Node& props = {});
    
    // Helper for variable arguments
    // C++20 doesn't have std::initializer_list for Span easy conversion without allocation?
    // We'll use older variadic templates or just vector (but vector allocates).
    // We need `ArenaVector` or similar helper.
    
    Container* Row(std::initializer_list<Node*> children);
    Container* Column(std::initializer_list<Node*> children);
    
    Text* Label(StringView text);
    ButtonNode* TextButton(StringView text, Callback on_click);
    
    // Phase 4: Entry Point
    void Render(Node* root);

} // namespace fanta
