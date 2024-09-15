#ifndef ZEROENGINE_VULKAN_RENDERING_CONTEXT_H
#define ZEROENGINE_VULKAN_RENDERING_CONTEXT_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "zeroengine_vulkan/VulkanRenderTarget.hpp"
#include "zeroengine_vulkan/Vulkan_define.hpp"
#include "zeroengine_core/GraphicsAPIContext.hpp"

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
class VulkanContext : GraphicsAPIContext {
private:
    VkInstance vk_instance;
    VkPhysicalDevice vk_physical_device;
    VkDevice vk_device;

    VkQueueInfo vk_graphics_queue;
    VkQueueInfo vk_presentation_queue;

// initialization and cleanup procedures
public:
    static constexpr const char* getRequiredExtension() {
#if defined(VK_USE_PLATFORM_XLIB_KHR)
        return VK_KHR_XLIB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#endif
        return "";
    }
    VulkanContext() {
        VKInit_initInstance();
    }
    void initVulkan(const VkSurfaceKHR &surface);

    void VKInit_initInstance();
    void VKInit_initPhysicalDevice(const VkSurfaceKHR &surface);
    void VKInit_initLogicalDevice(const VkSurfaceKHR &surface);
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

public:
    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> present_modes;
    };

// physical device extensions and valdiation layers support
private:
    bool checkValidationLayersSupport(std::string* ret = nullptr);
    bool validateExtensionsSupport(const uint32_t &extension_count, const char*const* extensions, std::string *ret = nullptr);

    void selectPhysicalDeviceVulkan(const VkSurfaceKHR &surface, std::vector<VkPhysicalDevice>& devices_list);
    uint32_t evaluatePhysicalDeviceSuitability(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);
    bool checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device);

    QueueFamilyIndices queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<uint32_t> &query);
    int32_t queryPresentationQueueFamily(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);
    
// basic wrapping interface for applications to communicate/synchronize with Vulkan per-frame rendering
public:
    VkInstance& getInstance() {
        return this->vk_instance;
    }
    VkQueueInfo& getGraphicalQueue() {
        return this->vk_graphics_queue;
    }
    VkQueueInfo& getPresentationQueue() {
        return this->vk_presentation_queue;
    }
    VkDevice& getDevice() {
        return this->vk_device;
    }
    VkPhysicalDevice& getPhysicalDevice() {
        return this->vk_physical_device;
    }
    SwapChainSupportDetails querySwapChainSupport(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);

    void waitForFence(VkFence fence);
    void releaseFence(VkFence fence);
};

#endif // #ifndef VULKAN_RENDERING_CONTEXT_H