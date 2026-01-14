// Fantasmagorie v3 - Unified Rendering Implementation Pass
#pragma once
#include "fanta_core_unified.hpp"

namespace fanta {

namespace cmd {
    struct RectCmd { Rect bounds; ResolvedStyle style; };
    struct TextCmd { Vec2 pos; std::string text; Color color; bool bold; };
    struct LayerBeginCmd { Rect bounds; float blur; bool backdrop; };
}

class PaintList {
    struct Command {
        virtual ~Command() = default;
        virtual void execute(RenderContext& ctx) = 0;
    };
    template<typename T> struct CommandImpl : Command {
        T data; CommandImpl(const T& d) : data(d) {}
        void execute(RenderContext& ctx) override; // Defined in cpp
    };
    std::vector<std::unique_ptr<Command>> queue;
public:
    void rect(Rect b, ResolvedStyle s) { queue.push_back(std::make_unique<CommandImpl<cmd::RectCmd>>(cmd::RectCmd{b, s})); }
    void text(Vec2 p, std::string t, Color c, bool b=false) { queue.push_back(std::make_unique<CommandImpl<cmd::TextCmd>>(cmd::TextCmd{p, t, c, b})); }
    void layer_begin(Rect b, float blur, bool backdrop) { queue.push_back(std::make_unique<CommandImpl<cmd::LayerBeginCmd>>(cmd::LayerBeginCmd{b, blur, backdrop})); }
    void layer_end() { /* ... push LayerEnd placeholder ... */ }
    
    void flush(RenderContext& ctx) { for(auto& c : queue) c->execute(ctx); queue.clear(); }
};

class UIRenderer {
public:
    static void render(const NodeStore& store, PaintList& paint);
private:
    static void render_recursive(NodeID id, const NodeStore& store, PaintList& paint);
};

}
