// Fantasmagorie v3 - RenderBackend Header
#pragma once
#include "render/paint_list.hpp"
#include <memory>

namespace fanta {

class RenderContextImplementation;

class RenderBackend {
    std::unique_ptr<RenderContextImplementation> impl_;
public:
    RenderBackend();
    ~RenderBackend();

    bool init();
    void shutdown();
    
    void begin_frame(int w, int h, float dpr);
    void end_frame();
    
    void execute(const PaintList& list);
    
    // Temporary direct access helpers if needed
    void draw_rect(const cmd::RectCmd& r);
};

void load_gl_functions();

}
