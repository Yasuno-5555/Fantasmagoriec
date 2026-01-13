// Fantasmagorie v2 - Icon Implementation
#include "icon.hpp"
#include "../style/theme.hpp"

namespace fanta {

void Icon::build() {
    if (!g_ctx) return;
    
    // Create node (auto ID based on type if no user ID?)
    // Actually we need a unique ID. 
    // Usually Icon is just a visual, but it needs a node for layout.
    // Let's use "Icon" string + some salt? Or require an ID if interacting?
    // For now, "Icon" + stack mixing handles basic cases.
    NodeID id = g_ctx->begin_node("Icon");
    NodeHandle n = g_ctx->get(id);
    
    // Defaults
    if (common.width <= 0) common.width = 16;
    if (common.height <= 0) common.height = 16;
    
    common.apply(n);
    
    // Style (Just for info, mostly Custom Render)
    n.style().text = Color::Hex(color_);
    
    // Render
    // We can use Custom render type or just a direct DrawList call?
    // BUT renderer logic loops over nodes.
    // So we should flag this node as needing custom render OR usage the new DrawType::Icon if we supported it in Renderer?
    // Wait, I implemented DrawList::add_icon which decomposes to lines.
    // But Renderer::submit_node iterates the tree.
    // It doesn't call DrawList::add_icon automatically unless I tell it to.
    
    // Currently Renderer::submit_node handles: Shadow, Bg, Text, Image, Border.
    // It does NOT handle "Icon".
    
    // Option A: Add "Icon" support to Renderer::submit_node.
    // Option B: Usage a callback? N/A.
    // Option C: Make Icon a "Text" node with special font? No.
    
    // I need to update Renderer to support Icon.
    // Let's add `IconType` to `RenderState` (RenderData)?
    // core/node.hpp > RenderData struct.
    
    // Ah, I missed that step. I need to store the Icon type in the NodeStore so the Renderer sees it.
    
    // Temporary Hack until I update NodeStore:
    // Abuse `RenderData::texture` to store IconType casted to void*?
    // And `RenderData::is_image` = true?
    // No, that confuses image path.
    
    // Correct way: Update RenderData struct in node.hpp.
    // But since I cannot edit node.hpp easily (it's big and fragile context), 
    // maybe I can reuse `text` string? "ICON:123"?
    // Parsing string in renderer is slow.
    
    // Let's UPDATE node.hpp. It's safe if I just add a field.
    
    n.render().is_icon = true;
    n.render().icon = type_;
    n.render().icon_color = color_;
    
    g_ctx->end_node();
}

} // namespace fanta
