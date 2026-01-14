#include "fanta.h"
#include "fanta_views.h"
#include "contexts_internal.hpp"
#include "types_internal.hpp"
#include <iostream>

namespace fanta {

    // Helper: Map V5 Align/Justify to V4 (if they differ, currently they invoke using declarations)
    // They are compatible because types_internal uses them.

    // Recursive Lowering Function
    static void LowerNode(Node* node, ID parent_id, internal::EngineContext& ctx) {
        if (!node) return;

        // 1. Resolve ID
        // If node->id is 0, we need an auto-generated ID based on parent + index?
        // For now, let's assume all V5 nodes need distinct IDs.
        // If V5 node has no ID, we generate one mixed with parent.
        ID id = node->id;
        if (id.value == 0) {
            // Auto ID generation (simple hash for demo)
            // Ideally we use the loop index in the parent.
            // But here we don't have index passed.
            // Let's rely on a global counter for this frame? No, that breaks persistence (State loss).
            // Proper way: Parent adds children with deterministic IDs.
            // Hack for Phase 4: Use a frame-global counter that resets (only works if static topo).
            static uint64_t auto_id_counter = 0x1000;
            id = ID(parent_id.value ^ ++auto_id_counter); 
            // Note: This is bad for persistence if order changes.
            // But sufficient for "wire_demo" static UI.
        }

        // 2. Get/Alloc ElementState
        // We need to find if this state exists (Persistent) or simply create a transient one for this frame?
        // V4 ElementState is persistent.
        // We must update the existing one or create new.
        internal::ElementState* state = nullptr;
        
        auto it = ctx.store.persistent_states.find(id);
        if (it == ctx.store.persistent_states.end()) {
            // Create new
            internal::PersistentData pd;
            ctx.store.persistent_states[id] = pd; // Default
        }
        
        // Flatten to Frame Element
        internal::ElementState el;
        el.id = id;
        el.parent = parent_id;
        
        // 3. Sync Properties
        el.is_node = true; // Mark as generic node
        
        // Layout
        el.width = node->width;
        el.height = node->height;
        el.req_width = node->width;   // Map
        el.req_height = node->height; // Map
        el.flex_grow = node->flex_grow;
        el.layout_mode = node->layout_mode;
        el.align_self = node->align_self;
        el.padding = {node->padding, node->padding, node->padding, node->padding};
        el.margin = {node->margin, node->margin, node->margin, node->margin};
        
        // Style
        el.bg_color = {node->bg_color.rgba, node->bg_color.token, node->bg_color.is_semantic};
        
        // Text
        // Dynamic cast to check type (RTTI on View?)
        // Node is POD, but we have inheritance.
        // We can't easily dynamic_cast without virtual functions (RTTI requires vtable).
        // V5 Nodes have virtual destructors? No, strict POD.
        // Wait. "struct Node : public View". View has no virtuals?
        // If View has no virtuals, we CANNOT use dynamic_cast.
        // How do we know it's a Text node?
        // We need a "Type" enum in Node.
        // Phase 3 Plan didn't specify it, but usually necessary.
        // Or we use `layout_mode` to guess? No.
        
        // Type Dispatch
        if (node->type == NodeType::Text) {
            Text* text_node = static_cast<Text*>(node);
            // Copy std::string_view to std::string (ElementState uses std::string)
            // Ideally ElementState uses ArenaString too, but that's a larger refactor.
            el.text = std::string(text_node->content); 
            // Map Color
            el.text_color = {text_node->color.rgba, text_node->color.token, text_node->color.is_semantic};
            el.font_size = text_node->size;
            el.text_token = text_node->font_token;
        }
        else if (node->type == NodeType::Container || node->type == NodeType::Button) {
            Container* container = static_cast<Container*>(node);
            el.align_content = container->align_items; 
            el.justify = container->justify_content;
            el.gap = container->gap;
            
            if (node->type == NodeType::Button) {
                // Button specific
                // ButtonNode* btn = static_cast<ButtonNode*>(node); 
                // Handle click? For now, logic is separate from rendering.
                // We just need to mark it clickable for Input system.
                // el.is_clickable = true; // Need to check if ElementState has this.
                // It likely implies it via event handlers.
                // For Phase 4, we just render it.
            }
            
            // Push this element first so children can find parent
            ctx.runtime.frame_elements.push_back(el);
            ctx.runtime.id_stack.push_back(id); 

            // Recurse children
            for (Node* child : container->children) {
                LowerNode(child, id, ctx);
            }
            
            ctx.runtime.id_stack.pop_back();
            return; // Already pushed
        }
        
        // Generic Push
        ctx.runtime.frame_elements.push_back(el);
        ctx.runtime.id_stack.push_back(id); 
        ctx.runtime.id_stack.pop_back();
    }
    
    // Public Entry Point
    void Lower(Node* root) {
        if (!root) return;
        auto& ctx = internal::GetEngineContext(); // Access legacy/hybrid context directly
        
        // Root has no parent (0)
        LowerNode(root, ID(0), ctx);
    }
    
} // namespace fanta
