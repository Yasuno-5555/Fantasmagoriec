#pragma once
#include <cstdint>
#include <memory>

namespace fanta {
namespace internal {

    enum class TextureFormat {
        R8,
        RGBA8,
        RGBA16F, // HDR
    };

    // Abstract handle for a GPU texture. 
    // Decouples high-level systems (like GlyphCache) from specific APIs.
    class GpuTexture {
    public:
        virtual ~GpuTexture() = default;
        
        // Upload data to the texture.
        virtual void upload(const void* data, int width, int height) = 0;
        
        // Get API-specific handle (GLuint for GL, VkImage for Vulkan, etc.)
        // Used by the backend's internal render loop.
        virtual uint64_t get_native_handle() const = 0;
        
        virtual int width() const = 0;
        virtual int height() const = 0;
    };

    using GpuTexturePtr = std::shared_ptr<GpuTexture>;

} // namespace internal
} // namespace fanta
