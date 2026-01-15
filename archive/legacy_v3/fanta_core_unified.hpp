#pragma once
#include <vector>
#include <string>
#include <map>
#include <functional>
#include <cmath>
#include <cstdint>
#include <stack>

namespace fanta {

using NodeID = uint64_t;

struct Vec2 { float x=0,y=0; };
struct Rect { float x=0,y=0,w=0,h=0; };
struct Color { 
    float r=0,g=0,b=0,a=1; 
    static constexpr Color Hex(uint32_t c) { return {((c>>24)&0xFF)/255.f, ((c>>16)&0xFF)/255.f, ((c>>8)&0xFF)/255.f, (c&0xFF)/255.f}; }
    static constexpr Color White() { return {1,1,1,1}; }
};

struct ResolvedStyle {
    Color bg = {0,0,0,0};
    Color text = {1,1,1,1};
    float radius = 0;
    float border_width = 0;
    Color border_color = {0,0,0,0};
};

struct LayoutData { float x=0,y=0,w=0,h=0; };
struct RenderData { 
    bool is_text=false; std::string text; 
    float blur=0; bool backdrop=false; 
};
struct ConstraintData { float w=-1,h=-1,p=0; };
struct TreeData { NodeID parent=0; std::vector<NodeID> children; };

struct NodeStore {
    std::vector<NodeID> nodes;
    std::map<NodeID,LayoutData> layout;
    std::map<NodeID,ResolvedStyle> style;
    std::map<NodeID,RenderData> render;
    std::map<NodeID,ConstraintData> constraint;
    std::map<NodeID,TreeData> tree;
};

struct NodeHandle {
    NodeID id; NodeStore* s;
    NodeHandle(NodeID i, NodeStore* st) : id(i), s(st) {}
    ResolvedStyle& style() { return s->style[id]; }
    RenderData& render() { return s->render[id]; }
    ConstraintData& constraint() { return s->constraint[id]; }
};

class UIContext {
public:
    NodeStore store;
    std::stack<NodeID> stack;
    NodeID active_id=0;
    bool mouse_released=false;
    
    NodeID begin_node(const char* name);
    void end_node();
    void begin_frame();
    void end_frame();
    void solve_layout(int w, int h);
};

namespace cmd {
    struct RectCmd { Rect b; ResolvedStyle s; };
    struct TextCmd { Vec2 p; std::string t; Color c; };
    struct LayerBeginCmd { Rect b; float blur; bool backdrop; };
    struct LayerEndCmd {};
}

class RenderContext {
public:
    virtual ~RenderContext() = default;
    virtual void rect(const cmd::RectCmd&) = 0;
    virtual void text(const cmd::TextCmd&) = 0;
    virtual void layer_begin(const cmd::LayerBeginCmd&) = 0;
    virtual void layer_end(const cmd::LayerEndCmd&) = 0;
};

class PaintList {
    using Op = std::function<void(RenderContext&)>;
    std::vector<Op> ops;
public:
    void rect(Rect b, ResolvedStyle s) { ops.push_back([=](RenderContext& c){ c.rect({b,s}); }); }
    void text(Vec2 p, std::string t, Color col) { ops.push_back([=](RenderContext& c){ c.text({p,t,col}); }); }
    void layer_begin(Rect b, float blur, bool back) { ops.push_back([=](RenderContext& c){ c.layer_begin({b,blur,back}); }); }
    void layer_end() { ops.push_back([](RenderContext& c){ c.layer_end({}); }); }
    void flush(RenderContext& c) { for(auto& op:ops) op(c); ops.clear(); }
};

} // namespace fanta
