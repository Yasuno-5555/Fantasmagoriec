#include "backend/gpu_resources.hpp"

namespace fanta {

struct Capabilities {
    bool blur_native = false; 
};

class Backend {
public:
    virtual ~Backend() = default;
    
    virtual bool init(int width, int height, const char* title) = 0;
    virtual void shutdown() = 0;
    virtual bool is_running() = 0;
    
    virtual void begin_frame(int w, int h) = 0;
    virtual void end_frame() = 0;
    
    virtual void render(const fanta::internal::DrawList& draw_list) = 0;
    
    // Resource Creation
    virtual internal::GpuTexturePtr create_texture(int w, int h, internal::TextureFormat format) = 0;

    // Phase 6.1: Input Handling
    virtual void get_mouse_state(float& x, float& y, bool& down, float& wheel) = 0;
    
    virtual Capabilities capabilities() const = 0;
    
    // Phase 5.2: Screenshot
    virtual bool capture_screenshot(const char* filename) = 0;
};

} // namespace fanta
