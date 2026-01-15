#include "backend_factory.hpp"

namespace fanta {

std::unique_ptr<Backend> CreateBackend(BackendType type) {
    switch (type) {
#ifdef FANTA_BACKEND_OPENGL
        case BackendType::OpenGL:
            return CreateOpenGLBackend();
#endif
        case BackendType::OpenGLMinimal:
            return CreateOpenGLMinimalBackend();
#ifdef FANTA_BACKEND_VULKAN
        case BackendType::Vulkan:
            return CreateVulkanBackend();
#endif
#ifdef FANTA_BACKEND_D3D11
        case BackendType::D3D11:
            return CreateD3D11Backend();
#endif
#ifdef FANTA_BACKEND_WEBGPU
        case BackendType::WebGPU:
            return CreateWebGPUBackend();
#endif
#ifdef FANTA_BACKEND_GLES
        case BackendType::GLES:
            return CreateGLESBackend();
#endif
        default:
            return CreateOpenGLMinimalBackend();
    }
}

} // namespace fanta
