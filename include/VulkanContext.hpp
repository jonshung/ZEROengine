#ifndef ZEROENGINE_VULKAN_RENDERING_CONTEXT_H
#define ZEROENGINE_VULKAN_RENDERING_CONTEXT_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "VulkanRenderer.hpp"
#include "VulkanRenderTarget.hpp"
#include "WindowContext.hpp"
#include "Vulkan_define.hpp"

const uint32_t MAX_QUEUED_FRAME = 2;

// required device extensions
const std::vector<const char*> device_extensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

// debug validation layers
const std::vector<const char*> dbg_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
    const bool dbg_enable_validation_layers = false;
#else
    const bool dbg_enable_validation_layers = true;
#endif

/**
 * @brief VulkanContext holds essentials informations about the Vulkan runtime of the application. It also manage the hardware swapchain and auxillary resources.
 * 
 */
class VulkanContext {
private:
    VkInstance vk_instance;
    VkPhysicalDevice vk_physical_device;
    VkDevice vk_device;
    WindowContext* window_ctx = nullptr;

    VkQueueInfo vk_graphics_queue;
    VkQueueInfo vk_presentation_queue;

    VkSurfaceKHR vk_surface;
    VkSwapchainKHR vk_swapchain;
    std::shared_ptr<VkRenderPass> vk_swapchain_render_pass;
    std::vector<VulkanRenderTarget> vk_swapchain_render_targets;
    bool enable_depth_stencil_subpass = false;
    
    VkFormat vk_swapchain_image_format;
    VkExtent2D vk_swapchain_extent;

    // starting here should be user's programmable data
    uint32_t current_frame_index = 0;
    std::vector<BaseVertex> vertices;

// initialization and cleanup procedures
public:
    void initVulkan(WindowContext* const &window_context);

    void VKInit_initInstance();
    void VKInit_initWindowSurface();
    void VKInit_initPhysicalDevice();
    void VKInit_initLogicalDevice();
    void VKInit_initSwapChain();
    void VKInit_initSwapChainRenderPass();
    void VKInit_initSwapChainRenderTargets();

    void VKReload_swapChain();

    void cleanup_swapChain();
    void cleanup();

private:
    struct QueueFamilyIndices {
        std::unordered_map<uint32_t, std::optional<uint32_t>> queue_indices;

        bool exists() {
            for(auto &[family, index] : queue_indices) {
                if(!index.has_value()) return false;
            }
            return true;
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

// physical device extensions and valdiation layers support
private:
    bool checkValidationLayersSupport(std::string* ret = nullptr);
    bool validateExtensionsSupport(const uint32_t &extension_count, const char*const* extensions, std::string *ret = nullptr);

    void selectPhysicalDeviceVulkan(std::vector<VkPhysicalDevice>& devices_list);
    uint32_t evaluatePhysicalDeviceSuitability(const VkPhysicalDevice &phys_device);
    bool checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device);

    SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &phys_device);
    VkSurfaceFormatKHR selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
    VkPresentModeKHR selectSwapChainPresentationMode(const std::vector<VkPresentModeKHR> &modes);
    VkExtent2D selectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    QueueFamilyIndices queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<uint32_t> &query);
    int32_t queryPresentationQueueFamily(const VkPhysicalDevice &phys_device);

private:

// basic wrapping interface for applications to communicate/synchronize with Vulkan per-frame rendering
public:
    VkQueueInfo& getGraphicalQueue() {
        return this->vk_graphics_queue;
    }
    VkDevice& getDevice() {
        return this->vk_device;
    }
    std::vector<VulkanRenderTarget>& requestSwapChainRenderTargets() {
        return this->vk_swapchain_render_targets;
    }
    VkRenderPass& requestSwapChainRenderPass() {
        return (*this->vk_swapchain_render_pass);
    }
    VkResult acquireSwapChainImageIndex(uint32_t &index, VkSemaphore semaphore_lock);
    void presentLatestImage(const uint32_t &image_index, VkSemaphore semaphore_lock);
    const uint32_t& getCurrentFrameIndex() const;
    void queueNextFrame();

    void waitForFence(VkFence fence);
    void releaseFence(VkFence fence);
};

#endif // #ifndef VULKAN_RENDERING_CONTEXT_H