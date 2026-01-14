#pragma once

#include <webgpu/webgpu.h>
#include <iostream>
#include <vector>
#include <string>

// Error Handling Macro
#define WEBGPU_CHECK(expr) \
    if (!(expr)) { \
        std::cerr << "[WebGPU] Error at " << __FILE__ << ":" << __LINE__ << std::endl; \
        return false; \
    }

#define WEBGPU_LOG(msg) \
    std::cout << "[WebGPU] " << msg << std::endl

namespace fanta {
namespace internal {

    // Helper to print adapter properties
    inline void PrintAdapterProperties(WGPUAdapter adapter) {
        WGPUAdapterProperties props = {};
        wgpuAdapterGetProperties(adapter, &props);
        std::cout << "[WebGPU] Adapter: " << (props.name ? props.name : "Unknown") << std::endl;
        std::cout << "[WebGPU] Driver: " << (props.driverDescription ? props.driverDescription : "Unknown") << std::endl;
        std::cout << "[WebGPU] VendorID: " << props.vendorID << ", DeviceID: " << props.deviceID << std::endl;
        
        const char* typeStr = "Unknown";
        switch (props.adapterType) {
            case WGPUAdapterType_DiscreteGPU: typeStr = "Discrete GPU"; break;
            case WGPUAdapterType_IntegratedGPU: typeStr = "Integrated GPU"; break;
            case WGPUAdapterType_CPU: typeStr = "CPU"; break;
            default: break;
        }
        std::cout << "[WebGPU] Type: " << typeStr << std::endl;
    }

    // Request Adapter Callback Wrapper
    struct AdapterRequest {
        WGPUAdapter adapter = nullptr;
        bool completed = false;
    };

    // Request Device Callback Wrapper
    struct DeviceRequest {
        WGPUDevice device = nullptr;
        bool completed = false;
    };

} // namespace internal
} // namespace fanta
