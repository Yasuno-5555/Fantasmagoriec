#pragma once

#include "backend/backend.hpp"
#include <vector>
#include <optional>
#include <memory>

// Vulkan handle forward declarations 
typedef struct VkInstance_T* VkInstance;
typedef struct VkPhysicalDevice_T* VkPhysicalDevice;
typedef struct VkDevice_T* VkDevice;
typedef struct VkQueue_T* VkQueue;
typedef struct VkSurfaceKHR_T* VkSurfaceKHR;
typedef struct VkSwapchainKHR_T* VkSwapchainKHR;
typedef struct VkImage_T* VkImage;
typedef struct VkImageView_T* VkImageView;
typedef struct VkRenderPass_T* VkRenderPass;
typedef struct VkDescriptorSetLayout_T* VkDescriptorSetLayout;
typedef struct VkPipelineLayout_T* VkPipelineLayout;
typedef struct VkPipeline_T* VkPipeline;
typedef struct VkFramebuffer_T* VkFramebuffer;
typedef struct VkCommandPool_T* VkCommandPool;
typedef struct VkCommandBuffer_T* VkCommandBuffer;
typedef struct VkSemaphore_T* VkSemaphore;
typedef struct VkFence_T* VkFence;
typedef struct VkBuffer_T* VkBuffer;
typedef struct VkDeviceMemory_T* VkDeviceMemory;
typedef struct VkDescriptorPool_T* VkDescriptorPool;
typedef struct VkDescriptorSet_T* VkDescriptorSet;
typedef struct VkSampler_T* VkSampler;

struct GLFWwindow;

namespace fanta {

struct VulkanConfig {
    bool enable_validation = true;
    bool vsync = true;
    int max_frames_in_flight = 2;
};

class VulkanBackend : public Backend {
public:
    VulkanBackend(const VulkanConfig& config = {});
    virtual ~VulkanBackend();

    bool init(int width, int height, const char* title) override;
    void shutdown() override;
    bool is_running() override;

    void begin_frame(int w, int h) override;
    void end_frame() override;

    void render(const fanta::internal::DrawList& draw_list) override;

    // Phase 6.1: Input Handling
    void get_mouse_state(float& x, float& y, bool& down, float& wheel) override;

    // Resource Creation
    internal::GpuTexturePtr create_texture(int w, int h, internal::TextureFormat format) override;

    // Phase 5.2: Screenshot
    bool capture_screenshot(const char* filename) override;

    // Vulkan Helpers
    uint32_t find_memory_type(uint32_t typeFilter, int properties);
    void create_buffer(unsigned long long size, int usage, int properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void create_image(uint32_t width, uint32_t height, int format, int tiling, int usage, int properties, VkImage& image, VkDeviceMemory& imageMemory);
    void transition_image_layout(VkImage image, int format, int oldLayout, int newLayout);
    void copy_buffer_to_image(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkCommandBuffer begin_single_time_commands();
    void end_single_time_commands(VkCommandBuffer commandBuffer);

private:
    struct FrameData {
        VkSemaphore image_available_semaphore;
        VkSemaphore render_finished_semaphore;
        VkFence in_flight_fence;
        VkCommandBuffer command_buffer;
    };

    bool create_instance();
    bool setup_debug_messenger();
    bool create_surface();
    bool pick_physical_device();
    bool create_logical_device();
    bool create_swapchain();
    bool create_image_views();
    bool create_render_pass();
    bool create_descriptor_set_layout();
    bool create_graphics_pipeline();
    bool create_command_pool();
    bool create_command_buffers();
    bool create_sync_objects();

    void cleanup_swapchain();
    void recreate_swapchain();

    VulkanConfig config_;
    GLFWwindow* window_ = nullptr;
    
    VkInstance instance_ = nullptr;
    void* debug_messenger_ = nullptr;
    VkSurfaceKHR surface_ = nullptr;
    VkPhysicalDevice physical_device_ = nullptr;
    VkDevice device_ = nullptr;
    VkQueue graphics_queue_ = nullptr;
    VkQueue present_queue_ = nullptr;

    VkSwapchainKHR swapchain_ = nullptr;
    std::vector<VkImage> swapchain_images_;
    int swapchain_image_format_; // Using int to avoid header dependency
    int swapchain_extent_w_, swapchain_extent_h_;
    std::vector<VkImageView> swapchain_image_views_;
    std::vector<VkFramebuffer> swapchain_framebuffers_;

    VkRenderPass render_pass_ = nullptr;
    VkDescriptorSetLayout descriptor_set_layout_ = nullptr;
    VkPipelineLayout pipeline_layout_ = nullptr;
    VkPipeline graphics_pipeline_ = nullptr;

    VkCommandPool command_pool_ = nullptr;
    std::vector<FrameData> frames_;
    uint32_t current_frame_ = 0;
    bool framebuffer_resized_ = false;

    // Drawing resources
    VkBuffer vertex_buffer_ = nullptr;
    VkDeviceMemory vertex_buffer_memory_ = nullptr;

    static void framebuffer_size_callback(GLFWwindow* window, int width, int height);
};

std::unique_ptr<Backend> CreateVulkanBackend(const VulkanConfig& config = {});

} // namespace fanta
