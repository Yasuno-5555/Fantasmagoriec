#include "backend/vulkan_backend.hpp"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <set>
#include <algorithm>
#include <fstream>

#include <cstring>

namespace fanta {

// Vulkan Implementation of GpuTexture
class VulkanTexture : public internal::GpuTexture {
public:
    VulkanTexture(VulkanBackend* backend, int w, int h, internal::TextureFormat fmt) 
        : backend_(backend), w_(w), h_(h), format_(fmt) {
        
        VkFormat vk_fmt = (fmt == internal::TextureFormat::R8) ? (VkFormat)9 : (VkFormat)37; // R8_UNORM, R8G8B8A8_UNORM
        
        backend_->create_image(w, h, (int)vk_fmt, 0, 0x00000004 | 0x00000002, 0x00000001, image_, memory_); // SAMPLED | TRANSFER_DST, DEVICE_LOCAL
        
        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = (VkStructureType)15; // VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
        viewInfo.image = image_;
        viewInfo.viewType = (VkImageViewType)1; // 2D
        viewInfo.format = vk_fmt;
        viewInfo.subresourceRange.aspectMask = 0x00000001; // COLOR_BIT
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;
        
        vkCreateImageView(backend_->device_, &viewInfo, nullptr, &view_);
    }
    
    ~VulkanTexture() {
        // Cleanup (omitted)
    }
    
    void upload(const void* data, int w, int h) override {
        VkDeviceSize imageSize = w * h * (format_ == internal::TextureFormat::R8 ? 1 : 4);
        
        VkBuffer stagingBuffer;
        VkDeviceMemory stagingBufferMemory;
        backend_->create_buffer(imageSize, 0x00000001, 0x00000002 | 0x00000004, stagingBuffer, stagingBufferMemory); // TRANSFER_SRC, HOST_VISIBLE | HOST_COHERENT
        
        void* mapped;
        vkMapMemory(backend_->device_, stagingBufferMemory, 0, imageSize, 0, &mapped);
        memcpy(mapped, data, static_cast<size_t>(imageSize));
        vkUnmapMemory(backend_->device_, stagingBufferMemory);
        
        backend_->transition_image_layout(image_, (int)format_, 0, 1); // UNDEFINED -> TRANSFER_DST
        backend_->copy_buffer_to_image(stagingBuffer, image_, static_cast<uint32_t>(w), static_cast<uint32_t>(h));
        backend_->transition_image_layout(image_, (int)format_, 1, 2); // TRANSFER_DST -> SHADER_READ_ONLY
        
        vkDestroyBuffer(backend_->device_, stagingBuffer, nullptr);
        vkFreeMemory(backend_->device_, stagingBufferMemory, nullptr);
    }
    
    uint64_t get_native_handle() const override { return (uint64_t)image_; }
    int width() const override { return w_; }
    int height() const override { return h_; }

private:
    VulkanBackend* backend_;
    VkImage image_ = nullptr;
    VkDeviceMemory memory_ = nullptr;
    VkImageView view_ = nullptr;
    int w_, h_;
    internal::TextureFormat format_;
};

// ... constants ...
const std::vector<const char*> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

VulkanBackend::VulkanBackend(const VulkanConfig& config) : config_(config) {}

VulkanBackend::~VulkanBackend() {
    shutdown();
}

bool VulkanBackend::init(int width, int height, const char* title) {
    if (!glfwInit()) return false;

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    window_ = glfwCreateWindow(width, height, title, nullptr, nullptr);
    if (!window_) return false;

    glfwSetWindowUserPointer(window_, this);
    glfwSetFramebufferSizeCallback(window_, framebuffer_size_callback);

    if (!create_instance()) return false;
    if (!setup_debug_messenger()) return false;
    if (!create_surface()) return false;
    if (!pick_physical_device()) return false;
    if (!create_logical_device()) return false;
    if (!create_swapchain()) return false;
    if (!create_image_views()) return false;
    if (!create_render_pass()) return false;
    if (!create_descriptor_set_layout()) return false;
    if (!create_graphics_pipeline()) return false;
    if (!create_command_pool()) return false;
    if (!create_command_buffers()) return false;
    if (!create_sync_objects()) return false;

    return true;
}

void VulkanBackend::shutdown() {
    if (device_) vkDeviceWaitIdle(device_);

    cleanup_swapchain();

    if (descriptor_set_layout_) vkDestroyDescriptorSetLayout(device_, descriptor_set_layout_, nullptr);

    for (size_t i = 0; i < config_.max_frames_in_flight; i++) {
        if (frames_[i].render_finished_semaphore) vkDestroySemaphore(device_, frames_[i].render_finished_semaphore, nullptr);
        if (frames_[i].image_available_semaphore) vkDestroySemaphore(device_, frames_[i].image_available_semaphore, nullptr);
        if (frames_[i].in_flight_fence) vkDestroyFence(device_, frames_[i].in_flight_fence, nullptr);
    }

    if (command_pool_) vkDestroyCommandPool(device_, command_pool_, nullptr);

    if (device_) vkDestroyDevice(device_, nullptr);
    if (surface_) vkDestroySurfaceKHR(instance_, surface_, nullptr);
    if (instance_) vkDestroyInstance(instance_, nullptr);

    if (window_) {
        glfwDestroyWindow(window_);
        glfwTerminate();
    }
}

bool VulkanBackend::create_instance() {
    if (config_.enable_validation) {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        for (const char* layerName : validationLayers) {
            bool found = false;
            for (const auto& lp : availableLayers) {
                if (strcmp(layerName, lp.layerName) == 0) {
                    found = true;
                    break;
                }
            }
            if (!found) return false;
        }
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Fantasmagorie Crystal";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (config_.enable_validation) {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
        createInfo.ppEnabledLayerNames = validationLayers.data();
    } else {
        createInfo.enabledLayerCount = 0;
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
    createInfo.ppEnabledExtensionNames = extensions.data();

    return vkCreateInstance(&createInfo, nullptr, &instance_) == VK_SUCCESS;
}

bool VulkanBackend::create_surface() {
    return glfwCreateWindowSurface(instance_, window_, nullptr, &surface_) == VK_SUCCESS;
}

bool VulkanBackend::pick_physical_device() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
    if (deviceCount == 0) return false;

    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());

    for (const auto& device : devices) {
        // Simple suitability check: just check for swapchain support and graphics queue
        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        bool foundGraphics = false;
        for (uint32_t i = 0; i < queueFamilyCount; i++) {
            if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                VkBool32 presentSupport = false;
                vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
                if (presentSupport) {
                    foundGraphics = true;
                    break;
                }
            }
        }

        if (foundGraphics) {
            physical_device_ = device;
            break;
        }
    }

    return physical_device_ != nullptr;
}

bool VulkanBackend::create_logical_device() {
    // Find graphics and present queue
    uint32_t graphicsIdx = 0;
    uint32_t presentIdx = 0;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyCount, queueFamilies.data());

    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsIdx = i;
        }
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(physical_device_, i, surface_, &presentSupport);
        if (presentSupport) {
            presentIdx = i;
        }
    }

    std::set<uint32_t> uniqueQueueFamilies = { graphicsIdx, presentIdx };
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{};
        queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queueCreateInfo.queueFamilyIndex = queueFamily;
        queueCreateInfo.queueCount = 1;
        queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    if (vkCreateDevice(physical_device_, &createInfo, nullptr, &device_) != VK_SUCCESS) return false;

    vkGetDeviceQueue(device_, graphicsIdx, 0, &graphics_queue_);
    vkGetDeviceQueue(device_, presentIdx, 0, &present_queue_);

    return true;
}

bool VulkanBackend::create_swapchain() {
    VkSurfaceCapabilitiesKHR capabilities;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physical_device_, surface_, &capabilities);

    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &formatCount, nullptr);
    std::vector<VkSurfaceFormatKHR> formats(formatCount);
    vkGetPhysicalDeviceSurfaceFormatsKHR(physical_device_, surface_, &formatCount, formats.data());

    VkSurfaceFormatKHR surfaceFormat = formats[0];
    for (const auto& availableFormat : formats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            surfaceFormat = availableFormat;
            break;
        }
    }

    swapchain_image_format_ = surfaceFormat.format;

    VkExtent2D extent;
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        extent = capabilities.currentExtent;
    } else {
        int w, h;
        glfwGetFramebufferSize(window_, &w, &h);
        extent.width = std::clamp(static_cast<uint32_t>(w), capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        extent.height = std::clamp(static_cast<uint32_t>(h), capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    }

    swapchain_extent_w_ = extent.width;
    swapchain_extent_h_ = extent.height;

    uint32_t imageCount = capabilities.minImageCount + 1;
    if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount) {
        imageCount = capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    // Simplified: assume single queue or handle concurrent if needed
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.preTransform = capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = VK_PRESENT_MODE_FIFO_KHR; // VSync
    createInfo.clipped = VK_TRUE;

    if (vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_) != VK_SUCCESS) return false;

    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapchain_images_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchain_images_.data());

    return true;
}

bool VulkanBackend::create_image_views() {
    swapchain_image_views_.resize(swapchain_images_.size());
    for (size_t i = 0; i < swapchain_images_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchain_images_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = static_cast<VkFormat>(swapchain_image_format_);
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(device_, &createInfo, nullptr, &swapchain_image_views_[i]) != VK_SUCCESS) return false;
    }
    return true;
}

bool VulkanBackend::create_render_pass() {
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = static_cast<VkFormat>(swapchain_image_format_);
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    return vkCreateRenderPass(device_, &renderPassInfo, nullptr, &render_pass_) == VK_SUCCESS;
}

bool VulkanBackend::create_descriptor_set_layout() {
    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = (VkStructureType)32; // VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO
    layoutInfo.bindingCount = 0; 
    return vkCreateDescriptorSetLayout(device_, &layoutInfo, nullptr, &descriptor_set_layout_) == VK_SUCCESS;
}

bool VulkanBackend::create_graphics_pipeline() {
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = 0x01 | 0x0100; // VERTEX | FRAGMENT
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(VulkanPushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = (VkStructureType)30; // VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
    pipelineLayoutInfo.setLayoutCount = 1;
    pipelineLayoutInfo.pSetLayouts = &descriptor_set_layout_;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

    if (vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipeline_layout_) != VK_SUCCESS) return false;
    
    // Shader modules and pipeline state creation would go here...
    return true;
}

    return true;
}

bool VulkanBackend::create_command_pool() {
    uint32_t graphicsIdx = 0;
    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(physical_device_, &queueFamilyCount, queueFamilies.data());
    for (uint32_t i = 0; i < queueFamilyCount; i++) {
        if (queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
            graphicsIdx = i;
            break;
        }
    }

    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = graphicsIdx;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

    return vkCreateCommandPool(device_, &poolInfo, nullptr, &command_pool_) == VK_SUCCESS;
}

bool VulkanBackend::create_command_buffers() {
    frames_.resize(config_.max_frames_in_flight);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = command_pool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(frames_.size());

    std::vector<VkCommandBuffer> buffers(frames_.size());
    if (vkAllocateCommandBuffers(device_, &allocInfo, buffers.data()) != VK_SUCCESS) return false;

    for (size_t i = 0; i < frames_.size(); i++) {
        frames_[i].command_buffer = buffers[i];
    }

    return true;
}

bool VulkanBackend::create_sync_objects() {
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < config_.max_frames_in_flight; i++) {
        if (vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &frames_[i].image_available_semaphore) != VK_SUCCESS ||
            vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &frames_[i].render_finished_semaphore) != VK_SUCCESS ||
            vkCreateFence(device_, &fenceInfo, nullptr, &frames_[i].in_flight_fence) != VK_SUCCESS) {
            return false;
        }
    }

    return true;
}

void VulkanBackend::begin_frame(int w, int h) {
    (void)w; (void)h;
    vkWaitForFences(device_, 1, &frames_[current_frame_].in_flight_fence, VK_TRUE, UINT64_MAX);

    uint32_t imageIndex;
    VkResult result = vkAcquireNextImageKHR(device_, swapchain_, UINT64_MAX, frames_[current_frame_].image_available_semaphore, VK_NULL_HANDLE, &imageIndex);

    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreate_swapchain();
        return;
    } else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        return;
    }

    vkResetFences(device_, 1, &frames_[current_frame_].in_flight_fence);
    vkResetCommandBuffer(frames_[current_frame_].command_buffer, 0);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    vkBeginCommandBuffer(frames_[current_frame_].command_buffer, &beginInfo);
}

void VulkanBackend::end_frame() {
    vkEndCommandBuffer(frames_[current_frame_].command_buffer);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = { frames_[current_frame_].image_available_semaphore };
    VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT };
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitStages = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &frames_[current_frame_].command_buffer;

    VkSemaphore signalSemaphores[] = { frames_[current_frame_].render_finished_semaphore };
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    if (vkQueueSubmit(graphics_queue_, 1, &submitInfo, frames_[current_frame_].in_flight_fence) != VK_SUCCESS) {
        return;
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = &swapchain_;
    presentInfo.pImageIndices = &current_frame_; // Simplified, would need acquired index

    vkQueuePresentKHR(present_queue_, &presentInfo);

    current_frame_ = (current_frame_ + 1) % config_.max_frames_in_flight;
}

void VulkanBackend::render(const fanta::internal::DrawList& draw_list) {
    auto commandBuffer = frames_[current_frame_].command_buffer;

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = (VkStructureType)43; 
    renderPassInfo.renderPass = render_pass_;
    renderPassInfo.framebuffer = swapchain_framebuffers_[current_frame_];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = {(uint32_t)swapchain_extent_w_, (uint32_t)swapchain_extent_h_};

    VkClearValue clearColor = {{{0.1f, 0.1f, 0.1f, 1.0f}}};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, (VkSubpassContents)0);

    if (graphics_pipeline_) {
        vkCmdBindPipeline(commandBuffer, (VkPipelineBindPoint)0, graphics_pipeline_); 
    }

    for (const auto& cmd : draw_list.commands) {
        VulkanPushConstants pc{};
        bool should_draw = false;
        
        if (cmd.type == internal::DrawCmdType::Rectangle) {
            pc.pos[0] = cmd.rectangle.pos_x; pc.pos[1] = cmd.rectangle.pos_y;
            pc.size[0] = cmd.rectangle.size_x; pc.size[1] = cmd.rectangle.size_y;
            pc.color[0] = cmd.rectangle.color_r; pc.color[1] = cmd.rectangle.color_g;
            pc.color[2] = cmd.rectangle.color_b; pc.color[3] = cmd.rectangle.color_a;
            pc.shape_type = 0;
            should_draw = true;
        } else if (cmd.type == internal::DrawCmdType::RoundedRect) {
            pc.pos[0] = cmd.rounded_rect.pos_x; pc.pos[1] = cmd.rounded_rect.pos_y;
            pc.size[0] = cmd.rounded_rect.size_x; pc.size[1] = cmd.rounded_rect.size_y;
            pc.color[0] = cmd.rounded_rect.color_r; pc.color[1] = cmd.rounded_rect.color_g;
            pc.color[2] = cmd.rounded_rect.color_b; pc.color[3] = cmd.rounded_rect.color_a;
            pc.radius = cmd.rounded_rect.radius;
            pc.shape_type = 1;
            pc.is_squircle = cmd.rounded_rect.is_squircle ? 1 : 0;
            should_draw = true;
        }

        if (should_draw) {
            vkCmdPushConstants(commandBuffer, pipeline_layout_, 0x01 | 0x0100, 0, sizeof(VulkanPushConstants), &pc);
            vkCmdDraw(commandBuffer, 4, 1, 0, 0);
        }
    }

    vkCmdEndRenderPass(commandBuffer);
}

bool VulkanBackend::is_running() {
    return !glfwWindowShouldClose(window_);
}

void VulkanBackend::get_mouse_state(float& x, float& y, bool& down, float& wheel) {
    double mx, my;
    glfwGetCursorPos(window_, &mx, &my);
    x = (float)mx;
    y = (float)my;
    down = glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    wheel = 0; // Would need scroll callback
}

internal::GpuTexturePtr VulkanBackend::create_texture(int w, int h, internal::TextureFormat format) {
    (void)w; (void)h; (void)format;
    return nullptr;
}

bool VulkanBackend::capture_screenshot(const char* filename) {
    (void)filename;
    return false; // Stub
}

void VulkanBackend::cleanup_swapchain() {
    for (auto framebuffer : swapchain_framebuffers_) vkDestroyFramebuffer(device_, framebuffer, nullptr);
    for (auto imageView : swapchain_image_views_) vkDestroyImageView(device_, imageView, nullptr);
    if (swapchain_) vkDestroySwapchainKHR(device_, swapchain_, nullptr);
}

void VulkanBackend::recreate_swapchain() {
    int w = 0, h = 0;
    glfwGetFramebufferSize(window_, &w, &h);
    while (w == 0 || h == 0) {
        glfwGetFramebufferSize(window_, &w, &h);
        glfwWaitEvents();
    }

    vkDeviceWaitIdle(device_);
    cleanup_swapchain();
    create_swapchain();
    create_image_views();
    create_render_pass();
    create_graphics_pipeline();
    create_framebuffers();
}

bool VulkanBackend::create_framebuffers() {
    swapchain_framebuffers_.resize(swapchain_image_views_.size());
    for (size_t i = 0; i < swapchain_image_views_.size(); i++) {
        VkImageView attachments[] = { swapchain_image_views_[i] };
        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = render_pass_;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapchain_extent_w_;
        framebufferInfo.height = swapchain_extent_h_;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &swapchain_framebuffers_[i]) != VK_SUCCESS) return false;
    }
    return true;
}

internal::GpuTexturePtr VulkanBackend::create_texture(int w, int h, internal::TextureFormat format) {
    return std::make_shared<VulkanTexture>(this, w, h, format);
}

uint32_t VulkanBackend::find_memory_type(uint32_t typeFilter, int properties) {
    VkPhysicalDeviceMemoryProperties memProperties;
    vkGetPhysicalDeviceMemoryProperties(physical_device_, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
        if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
            return i;
        }
    }
    return 0;
}

void VulkanBackend::create_buffer(unsigned long long size, int usage, int properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    VkBufferCreateInfo bufferInfo{};
    bufferInfo.sType = (VkStructureType)12; // VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO
    bufferInfo.size = size;
    bufferInfo.usage = usage;
    bufferInfo.sharingMode = (VkSharingMode)0; // EXCLUSIVE

    if (vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer) != VK_SUCCESS) return;

    VkMemoryRequirements memRequirements;
    vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = (VkStructureType)5; // VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory) != VK_SUCCESS) return;
    vkBindBufferMemory(device_, buffer, bufferMemory, 0);
}

void VulkanBackend::create_image(uint32_t width, uint32_t height, int format, int tiling, int usage, int properties, VkImage& image, VkDeviceMemory& imageMemory) {
    VkImageCreateInfo imageInfo{};
    imageInfo.sType = (VkStructureType)14; // VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
    imageInfo.imageType = (VkImageType)1; // 2D
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.format = (VkFormat)format;
    imageInfo.tiling = (VkImageTiling)tiling;
    imageInfo.initialLayout = (VkImageLayout)0; // UNDEFINED
    imageInfo.usage = usage;
    imageInfo.samples = (VkSampleCountFlagBits)1; // 1_BIT
    imageInfo.sharingMode = (VkSharingMode)0; // EXCLUSIVE

    if (vkCreateImage(device_, &imageInfo, nullptr, &image) != VK_SUCCESS) return;

    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(device_, image, &memRequirements);

    VkMemoryAllocateInfo allocInfo{};
    allocInfo.sType = (VkStructureType)5; // VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = find_memory_type(memRequirements.memoryTypeBits, properties);

    if (vkAllocateMemory(device_, &allocInfo, nullptr, &imageMemory) != VK_SUCCESS) return;
    vkBindImageMemory(device_, image, imageMemory, 0);
}

void VulkanBackend::transition_image_layout(VkImage image, int format, int oldLayout, int newLayout) {
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkImageMemoryBarrier barrier{};
    barrier.sType = (VkStructureType)45; // VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER
    barrier.oldLayout = (VkImageLayout)oldLayout;
    barrier.newLayout = (VkImageLayout)newLayout;
    barrier.srcQueueFamilyIndex = (uint32_t)-1; // VK_QUEUE_FAMILY_IGNORED
    barrier.dstQueueFamilyIndex = (uint32_t)-1;
    barrier.image = image;
    barrier.subresourceRange.aspectMask = 0x00000001; // COLOR_BIT
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = 1;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if (oldLayout == 0 && newLayout == 1) { // UNDEFINED -> TRANSFER_DST
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = 0x00001000; // TRANSFER_WRITE_BIT
        sourceStage = 0x00000001; // TOP_OF_PIPE_BIT
        destinationStage = 0x00001000; // TRANSFER_BIT
    } else if (oldLayout == 1 && newLayout == 2) { // TRANSFER_DST -> SHADER_READ_ONLY
        barrier.srcAccessMask = 0x00001000; // TRANSFER_WRITE_BIT
        barrier.dstAccessMask = 0x00000020; // SHADER_READ_BIT
        sourceStage = 0x00001000; // TRANSFER_BIT
        destinationStage = 0x00000080; // FRAGMENT_SHADER_BIT
    } else {
        sourceStage = 0x00000001;
        destinationStage = 0x00000001;
    }

    vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, 0, 0, nullptr, 0, nullptr, 1, &barrier);

    end_single_time_commands(commandBuffer);
}

void VulkanBackend::copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height) {
    VkCommandBuffer commandBuffer = begin_single_time_commands();

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = 0x00000001; // COLOR_BIT
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, (VkImageLayout)1, 1, &region); // TRANSFER_DST_OPTIMAL

    end_single_time_commands(commandBuffer);
}

VkCommandBuffer VulkanBackend::begin_single_time_commands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = (VkStructureType)40; // VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO
    allocInfo.level = (VkCommandBufferLevel)0; // PRIMARY
    allocInfo.commandPool = command_pool_;
    allocInfo.commandBufferCount = 1;

    VkCommandBuffer commandBuffer;
    vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer);

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = (VkStructureType)42; // VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
    beginInfo.flags = 0x00000001; // ONE_TIME_SUBMIT
    vkBeginCommandBuffer(commandBuffer, &beginInfo);
    return commandBuffer;
}

void VulkanBackend::end_single_time_commands(VkCommandBuffer commandBuffer) {
    vkEndCommandBuffer(commandBuffer);
    VkSubmitInfo submitInfo{};
    submitInfo.sType = (VkStructureType)4; // VK_STRUCTURE_TYPE_SUBMIT_INFO
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    vkQueueSubmit(graphics_queue_, 1, &submitInfo, nullptr);
    vkQueueWaitIdle(graphics_queue_);
    vkFreeCommandBuffers(device_, command_pool_, 1, &commandBuffer);
}

} // namespace fanta
