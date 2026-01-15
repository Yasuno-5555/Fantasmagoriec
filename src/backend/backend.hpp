#pragma once
// L3: Backend Interface
// Dependencies: types_fwd.hpp (L0), <memory> ONLY.
// DrawList is forward-declared. Full definition not needed.

#include "core/types_fwd.hpp"
#include <memory>
#include <vector>

namespace fanta {

struct Capabilities {
    bool blur_native = false;
};

// GPU Texture Interface
namespace internal {
    enum class TextureFormat { R8, RGBA8, RGBA16F };
    
    class GpuTexture {
    public:
        virtual ~GpuTexture() = default;
        virtual void upload(const void* data, int w, int h) = 0;
        virtual uint64_t get_native_handle() const = 0;
        virtual int width() const = 0;
        virtual int height() const = 0;
    };
    
    using GpuTexturePtr = std::shared_ptr<GpuTexture>;
}

class Backend {
public:
    virtual ~Backend() = default;
    
    virtual bool init(int width, int height, const char* title) = 0;
    virtual void shutdown() = 0;
    virtual bool is_running() = 0;
    
    virtual void begin_frame(int w, int h) = 0;
    virtual void end_frame() = 0;
    
    // Forward declaration: DrawList is defined in drawlist.hpp
    virtual void render(const internal::DrawList& draw_list) = 0;
    
    virtual internal::GpuTexturePtr create_texture(int w, int h, internal::TextureFormat format) = 0;
    virtual void get_mouse_state(float& x, float& y, bool& down, float& wheel) = 0;
    virtual void get_keyboard_events(std::vector<uint32_t>& chars, std::vector<int>& keys) = 0;
    virtual Capabilities capabilities() const = 0;
    
#ifndef NDEBUG
    virtual bool capture_screenshot(const char* filename) = 0;
#endif
};

} // namespace fanta
