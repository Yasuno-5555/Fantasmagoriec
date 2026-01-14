#pragma once
#include <memory>
#include "backend.hpp"

namespace fanta {

std::unique_ptr<Backend> CreateD3D11Backend();
std::unique_ptr<Backend> CreateOpenGLBackend();
std::unique_ptr<Backend> CreateVulkanBackend();
std::unique_ptr<Backend> CreateGLESBackend();

} // namespace fanta
