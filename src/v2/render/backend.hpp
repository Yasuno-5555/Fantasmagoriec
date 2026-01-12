// Fantasmagorie v2 - Render Backend
// Abstract interface for rendering backends (OpenGL, Vulkan, Metal, Software)
#pragma once

#include "draw_list.hpp"

namespace fanta {

// Forward declarations
struct Font;

// ============================================================================
// Render Backend (Abstract Interface)
// ============================================================================

class RenderBackend {
public:
    virtual ~RenderBackend() = default;
    
    // Lifecycle
    virtual bool init() = 0;
    virtual void shutdown() = 0;
    
    // Frame
    virtual void begin_frame(int width, int height) = 0;
    virtual void end_frame() = 0;
    
    // Rendering
    virtual void render(const DrawList& list, Font* font = nullptr) = 0;
    
    // Texture Management
    virtual void* create_texture(int w, int h, const void* pixels, int channels = 4) = 0;
    virtual void update_texture(void* tex, int x, int y, int w, int h, const void* pixels) = 0;
    virtual void destroy_texture(void* tex) = 0;
    
    // Capabilities (for theme feature detection)
    virtual bool supports_blur() const { return false; }
    virtual bool supports_shadows() const { return true; }
    virtual void set_backdrop_blur(float radius) {}
};

// ============================================================================
// Text Measurement
// ============================================================================

struct TextMetrics {
    float width = 0;
    float height = 0;
    float ascent = 0;
    float descent = 0;
};

} // namespace fanta
