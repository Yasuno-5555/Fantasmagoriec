// Fantasmagorie v3 - Unified Core (Rich API & Deep Access)
#pragma once

#include <cstdint>
#include <cmath>
#include <algorithm>
#include <string>
#include <vector>
#include <map>
#include <stack>
#include <functional>
#include <memory>
#include <cstdio>

namespace fanta {

// --- Forward Decls ---
struct NodeStore;
class UIContext;
namespace cmd { struct RectCmd; struct CircleCmd; struct TextCmd; struct LayerBeginCmd; struct LayerEndCmd; }

// --- Basic Types ---
typedef uint64_t NodeID;
constexpr NodeID INVALID_NODE = 0;

struct Vec2 { float x=0, y=0; };
struct Rect { float x=0, y=0, w=0, h=0; };
struct Color { 
    float r=0, g=0, b=0, a=1; 
    static constexpr Color Hex(uint32_t c) { return {((c>>24)&0xFF)/255.f, ((c>>16)&0xFF)/255.f, ((c>>8)&0xFF)/255.f, (c&0xFF)/255.f}; }
    static constexpr Color White() { return {1,1,1,1}; }
    static constexpr Color Transparent() { return {0,0,0,0}; }
};

struct CornerRadius { 
    float tl=0, tr=0, br=0, bl=0; 
    CornerRadius(float r=0) : tl(r), tr(r), br(r), bl(r) {}
    CornerRadius(float tl_, float tr_, float br_, float bl_) : tl(tl_), tr(tr_), br(br_), bl(bl_) {}
};

struct Shadow {
    Color color = Color::Transparent();
    float blur=0, spread=0; Vec2 offset={0,0};
    static Shadow Level1() { return {Color::Hex(0x00000040), 4, 0, {0, 2}}; }
};

// --- Component Data (Deep Access) ---
struct LayoutData { float x=0, y=0, w=0, h=0; };
struct ResolvedStyle {
    Color bg = Color::Transparent(), text = Color::White();
    CornerRadius radius = 0;
    float border_width = 0;
    Color border_color = Color::Transparent();
    Shadow shadow;
};
struct RenderData { 
    bool is_text=false; std::string text; 
    float blur_radius=0; bool backdrop_blur=false; 
    uint32_t icon_id=0;
};
struct ConstraintData { float width=-1, height=-1, padding=0, gap=0, grow=0; };
struct TreeData { NodeID parent = 0; std::vector<NodeID> children; };

// --- NodeStore & Handles ---
struct NodeStore {
    std::vector<NodeID> nodes;
    std::map<NodeID, LayoutData> layout;
    std::map<NodeID, ResolvedStyle> style;
    std::map<NodeID, RenderData> render;
    std::map<NodeID, ConstraintData> constraint;
    std::map<NodeID, TreeData> tree;
    
    bool exists(NodeID id) const { return !nodes.empty(); }
};

struct NodeHandle {
    NodeID id; NodeStore* store;
    NodeHandle(NodeID i, NodeStore* s) : id(i), store(s) {}
    
    // Deep Accessors
    LayoutData& layout() { return store->layout[id]; }
    ResolvedStyle& style() { return store->style[id]; }
    RenderData& render() { return store->render[id]; }
    ConstraintData& constraint() { return store->constraint[id]; }
};

// --- Rendering Interface ---
class RenderContext {
public:
    virtual ~RenderContext() = default;
    virtual void draw_rect(const cmd::RectCmd&) = 0;
    virtual void draw_text(const cmd::TextCmd&) = 0;
    virtual void layer_begin(const cmd::LayerBeginCmd&) = 0;
    virtual void layer_end() = 0;
};

} // namespace fanta
