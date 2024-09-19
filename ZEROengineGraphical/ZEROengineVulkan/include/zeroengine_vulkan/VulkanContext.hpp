#ifndef ZEROENGINE_VULKAN_RENDERING_CONTEXT_H
#define ZEROENGINE_VULKAN_RENDERING_CONTEXT_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "zeroengine_vulkan/Vulkan_define.hpp"
#include "zeroengine_core/GraphicsAPIContext.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"

/* future support
// WSI intergration
#if defined(VK_USE_PLATFORM_XLIB_KHR)
#include <X11/Xlib.h>
#endif
*/

namespace ZEROengine {
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
    class VulkanContext : public GraphicsAPIContext {
    public:
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

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

        VulkanContext();
        ZEROResult initVulkan(const VkSurfaceKHR &surface);

        ZEROResult VKInit_initInstance();
        ZEROResult VKInit_initPhysicalDevice(const VkSurfaceKHR &surface);
        ZEROResult VKInit_initLogicalDevice(const VkSurfaceKHR &surface);
        void cleanup() override;

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

    // physical device extensions and valdiation layers support
    private:
        // development debug validation layers and Vulkan instance extensions support
        bool checkValidationLayersSupport(std::string *ret = nullptr);
        bool validateExtensionsSupport(const uint32_t &extension_count, const char*const *extensions, std::string *ret = nullptr);

        // physical device enumeration and selection
        ZEROResult selectPhysicalDevice(const VkSurfaceKHR &surface, const std::vector<VkPhysicalDevice>& devices_list);
        uint32_t evaluatePhysicalDeviceSuitability(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);
        bool checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device);

        // queue families enumeration and selection
        QueueFamilyIndices queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<uint32_t> &query);
        std::optional<uint32_t> queryPresentationQueueFamily_Surface(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);
        
    // basic wrapping interface for applications to communicate/synchronize with Vulkan per-frame rendering
    public:
        ZEROResult getInstance(VkInstance &ret) {
            if(this->vk_instance == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan instance handle is null." };
            }
            ret = this->vk_instance;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getGraphicalQueue(VkQueueInfo &ret) {
            if(this->vk_instance == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan instance handle is null." };
            }
            if(this->vk_graphics_queue.queue == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan graphics queue handle is null." };
            }
            ret = this->vk_graphics_queue;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getPresentationQueue(VkQueueInfo &ret) {
            if(this->vk_instance == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan instance handle is null." };
            }
            if(this->vk_graphics_queue.queue == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan presentation queue handle is null." }; 
            }
            ret = this->vk_presentation_queue;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getDevice(VkDevice &ret) {
            if(this->vk_instance == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan instance handle is null." };
            }
            if(this->vk_device == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan logical device handle is null." };
            }
            ret = this->vk_device;
            return { ZERO_SUCCESS, "" };
        }
        ZEROResult getPhysicalDevice(VkPhysicalDevice &ret) {
            if(this->vk_instance == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan instance handle is null." };
            }
            if(this->vk_physical_device == VK_NULL_HANDLE) {
                return { ZERO_VULKAN_NULL_HANDLE, "Vulkan physical device handle is null." };
            }
            ret = this->vk_physical_device;
            return { ZERO_SUCCESS, "" };
        }
        SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &phys_device);
        SwapChainSupportDetails querySwapChainSupport_Surface(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);

        ZEROResult allocateBuffer() override;
        ZEROResult allocateTexture() override;

        void waitForFence(VkFence fence);
        void releaseFence(VkFence fence);
    }; // class VulkanContext
} // namespace ZEROengine
#endif // #ifndef VULKAN_RENDERING_CONTEXT_H