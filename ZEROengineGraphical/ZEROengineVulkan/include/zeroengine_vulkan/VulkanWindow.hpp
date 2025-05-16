#ifndef ZEROENGINE_VULKANWINDOW_H
#define ZEROENGINE_VULKANWINDOW_H

#include "vulkan/vulkan.hpp"

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>

#include "zeroengine_graphical/GPUWindow.hpp"
#include "zeroengine_core/ZERODefines.hpp"
#include "zeroengine_vulkan/VulkanDevice.hpp"

namespace ZEROengine {
    class VulkanDevice;
    
    /**
     * @brief A structure to manage Render Targets-related attributes.
     * 
     */
    class VulkanWindow : public GPUWindow {
    protected:
        VkDevice m_vk_device;
        VkPhysicalDevice m_vk_phys_device;

        VkSurfaceKHR m_vk_surface;
        VulkanQueueInfo m_vulkan_presentation_queue;

        VkSwapchainKHR m_vk_swapchain;
        std::vector<VkImage> m_vk_swapchain_images;
        std::vector<VkImageView> m_vk_swapchain_image_views;
        std::vector<VkFramebuffer> m_vk_swapchain_framebuffers;
        VkRenderPass m_vk_swapchain_renderpass;
        VkFormat m_vk_swapchain_format;

        int64_t m_graphical_queue_family;
        uint32_t m_acquired_swapchain;

    private:
        VkSurfaceFormatKHR selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
        VkPresentModeKHR selectSwapchainPresentationMode(const std::vector<VkPresentModeKHR> &modes);
        bool m_enable_depth_stencil_subpass = false;
    
    protected:
        void initSwapChain();
        void initSwapChainRenderPass();
        void initSwapChainRenderTargets();
        
    public:
        VulkanWindow(
            const VkDevice &vk_device,
            const VkPhysicalDevice &vk_phys_device,
            const VkSurfaceKHR &vk_surface,
            const std::string &title,
            const WindowTransform &transform,
            const WindowSetting &setting,
            const WindowStyle &style
        );
        void init() override final;
        void setGraphicalQueueFamily(const uint32_t &v);

        std::optional<uint32_t> queryPresentationQueueIndex() const;
        std::optional<VkDeviceQueueCreateInfo> queryPresentationQueueCreation() const;

        SwapChainSupportDetails querySwapChainSupport();

        VkSwapchainKHR getSwapchain() const;
        VkFramebuffer getFramebuffer(const uint32_t &index) const;
        VkRenderPass getRenderPass() const;
        VkSurfaceKHR getSurface() const;
        bool tryAcquireSwapchainImage(const VkSemaphore &wait_semaphore, const uint32_t &call_depth = 0);
        uint32_t getAcquiredSwapchain() const;

        void reload_swapChain();
        void cleanup_swapChain();

        virtual void cleanup() override;

    }; // class VulkanWindow
} // namespace ZEROengine

#endif //#ifndef ZEROENGINE_VULKANWINDOW_H