#ifndef ZEROENGINE_VULKANDEVICE_H
#define ZEROENGINE_VULKANDEVICE_H

#include <memory>
#include <vector>
#include <string>
#include <cstdint>
#include <optional>
#include <unordered_map>
#include <list>

#include "vulkan/vulkan.hpp"
#include "vk_mem_alloc.h"

#include "zeroengine_core/ZERODefines.hpp"
#include "zeroengine_graphical/GPUDevice.hpp"
#include "zeroengine_vulkan/VulkanQueueManager.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"
#include "zeroengine_vulkan/VulkanWindow.hpp"

namespace ZEROengine {
    class VulkanWindow;

    #ifdef NDEBUG
        const bool const_dbg_enable_validation_layers = false;
    #else
        const bool const_dbg_enable_validation_layers = true;
    #endif

    /**
     * @brief VulkanDevice holds essentials informations about the Vulkan runtime of the application. It also manage the hardware swapchain and auxillary resources.
     * 
     */
    class VulkanDevice : public GPUDevice {
    private:
        VkInstance m_vk_instance;
        VkPhysicalDevice m_vk_physical_device;
        VkDevice m_vk_device;
        
        VmaAllocator m_vma_alloc;

        std::shared_ptr<VulkanQueueManager> m_vulkan_queue_manager;
        
    // initialization and cleanup procedures
    public:
        static constexpr const char* getRequiredExtension();

        VulkanDevice();
        void initVulkan(VulkanWindow* vulkan_window = nullptr);

        void initInstance();
        void initPhysicalDevice();
        void initLogicalDevice(VulkanWindow* vulkan_window = nullptr);
        void cleanup() override;

    // physical device extensions and valdiation layers support
    private:
        // development debug validation layers and Vulkan instance extensions support
        bool checkValidationLayersSupport(std::string *ret = nullptr);
        bool validateExtensionsSupport(const uint32_t &extension_count, const char*const *extensions, std::string *ret = nullptr);

        // physical device enumeration and selection
        void selectPhysicalDevice(const std::vector<VkPhysicalDevice>& devices_list);
        uint32_t evaluatePhysicalDeviceSuitability(const VkPhysicalDevice &phys_device);
        bool checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device);
        
    public:
        VkInstance getInstance();
        std::weak_ptr<VulkanQueueManager> getQueueManager();
        std::weak_ptr<GraphicalContext> allocateGraphicalContext() override final;

        VkDevice getDevice();
        VkPhysicalDevice getPhysicalDevice();

        ZEROResult allocateBuffer() override;
        ZEROResult allocateTexture() override;

        void waitForFence(VkFence fence);
        void releaseFence(VkFence fence);
        void stall();
    }; // class VulkanDevice
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKANDEVICE_H