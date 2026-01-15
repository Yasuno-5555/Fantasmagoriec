#pragma once
#include <memory>
#include "backend.hpp"

namespace fanta {

enum class BackendType {
    OpenGL,
    OpenGLMinimal,
    Vulkan,
    D3D11,
    WebGPU,
    GLES
};

std::unique_ptr<Backend> CreateBackend(BackendType type);

// Individual factory functions
std::unique_ptr<Backend> CreateOpenGLBackend();
std::unique_ptr<Backend> CreateOpenGLMinimalBackend();
std::unique_ptr<Backend> CreateVulkanBackend();
std::unique_ptr<Backend> CreateD3D11Backend();
std::unique_ptr<Backend> CreateWebGPUBackend();
std::unique_ptr<Backend> CreateGLESBackend();

} // namespace fanta
