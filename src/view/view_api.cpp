#include "fanta_views.h"
#include "core/contexts_internal.hpp"
#include "core/contexts_internal.hpp"

#include <initializer_list>
#include <algorithm>

namespace fanta {

    // Helper accessor for View API (needs context)
    #define g_ctx (fanta::internal::GetEngineContext())

    Node* Box(const Node& props) {
        auto& arena = internal::GetContext().frame->arena;
        Node* node = arena.alloc<Node>(props);
        return node;
    }

    Container* Row(std::initializer_list<Node*> children) {
        auto& arena = internal::GetContext().frame->arena;
        Container* node = arena.alloc<Container>();
        node->type = NodeType::Container; 
        node->layout_mode = LayoutMode::Row;
        
        if (children.size() > 0) {
            auto span = arena.alloc_array<Node*>(children.size());
            std::copy(children.begin(), children.end(), span.begin());
            node->children = span;
        }
        return node;
    }

    Container* Column(std::initializer_list<Node*> children) {
        auto& arena = internal::GetContext().frame->arena;
        Container* node = arena.alloc<Container>();
        node->type = NodeType::Container; 
        node->layout_mode = LayoutMode::Column;
        
        if (children.size() > 0) {
            auto span = arena.alloc_array<Node*>(children.size());
            std::copy(children.begin(), children.end(), span.begin());
            node->children = span;
        }
        return node;
    }

    Text* Label(StringView text) {
        auto& arena = internal::GetContext().frame->arena;
        Text* node = arena.alloc<Text>();
        node->type = NodeType::Text; 
        node->content = arena.push_string(text);
        return node;
    }
    
    ButtonNode* TextButton(StringView text, Callback on_click) {
        auto& arena = internal::GetContext().frame->arena;
        ButtonNode* node = arena.alloc<ButtonNode>();
        node->type = NodeType::Button; 
        node->on_click = on_click;
        
        Text* label = Label(text);
        label->color = Color::Token(ColorToken::Label); 
        
        auto children = arena.alloc_array<Node*>(1);
        children[0] = label;
        node->children = children;
        
        node->bg_color = Color::Token(ColorToken::Fill);
        node->padding = 8.0f;
        node->align_items = Align::Center;
        node->justify_content = Justify::Center;
        
        return node;
    }

} // namespace fanta
