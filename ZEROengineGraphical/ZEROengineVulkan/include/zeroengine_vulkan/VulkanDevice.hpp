#ifndef ZEROENGINE_VULKANDEVICE_H
#define ZEROENGINE_VULKANDEVICE_H

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.h"

#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>

#include "zeroengine_vulkan/VulkanDefines.hpp"
#include "zeroengine_graphical/GraphicalDevice.hpp"
#include "zeroengine_core/ZERODefines.hpp"

namespace ZEROengine {
    #ifdef NDEBUG
        const bool dbg_enable_validation_layers = false;
    #else
        const bool const_dbg_enable_validation_layers = true;
    #endif

    /**
     * @brief VulkanDevice holds essentials informations about the Vulkan runtime of the application. It also manage the hardware swapchain and auxillary resources.
     * 
     */
    class VulkanDevice : public GraphicalDevice {
    public:
        struct SwapChainSupportDetails {
            VkSurfaceCapabilitiesKHR capabilities;
            std::vector<VkSurfaceFormatKHR> formats;
            std::vector<VkPresentModeKHR> present_modes;
        };

    private:
        VkInstance m_vk_instance;
        VkPhysicalDevice m_vk_physical_device;
        VkDevice m_vk_device;
        
        VmaAllocator m_vma_alloc;

    // initialization and cleanup procedures
    public:
        static constexpr const char* getRequiredExtension();

        VulkanDevice();
        void initVulkan();

        void VKInit_initInstance();
        void VKInit_initPhysicalDevice();
        /**
         * @brief This method should be called externally by either windowing system initialization process.
         * 
         * @param surface 
         */
        void VKInit_initLogicalDevice(const VkSurfaceKHR &surface);
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
        void selectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices_list);
        uint32_t evaluatePhysicalDeviceSuitability(const VkPhysicalDevice &phys_device);
        bool checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device);

        // queue families enumeration and selection
        QueueFamilyIndices queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<uint32_t> &query);
        std::optional<uint32_t> queryPresentationQueueFamily_Surface(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);
        
    // basic wrapping interface for applications to communicate/synchronize with Vulkan per-frame rendering
    private:
        VkQueueInfo m_vk_graphics_queue;
        VkQueueInfo m_vk_presentation_queue;

        
    public:
        VkInstance getInstance();
        VkQueueInfo getGraphicalQueue();
        VkQueueInfo getPresentationQueue();
        

        VkDevice getDevice();
        VkPhysicalDevice getPhysicalDevice();
        SwapChainSupportDetails querySwapChainSupport(const VkPhysicalDevice &phys_device);
        SwapChainSupportDetails querySwapChainSupport_Surface(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device);

        ZEROResult allocateBuffer() override;
        ZEROResult allocateTexture() override;

        void waitForFence(VkFence fence);
        void releaseFence(VkFence fence);
        void stall();
    }; // class VulkanDevice
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANDEVICE_H