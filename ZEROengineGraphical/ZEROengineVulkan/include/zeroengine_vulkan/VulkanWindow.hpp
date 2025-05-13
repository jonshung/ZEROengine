#ifndef ZEROENGINE_VULKANWINDOW_H
#define ZEROENGINE_VULKANWINDOW_H

#include "vulkan/vulkan.hpp"

#include <vector>
#include <cstdint>
#include <unordered_map>
#include <memory>

#include "zeroengine_graphical/GraphicalWindow.hpp"
#include "zeroengine_core/ZERODefines.hpp"
#include "zeroengine_vulkan/VulkanDevice.hpp"

namespace ZEROengine {
    /**
     * @brief A structure to manage Render Targets-related attributes.
     * 
     */
    class VulkanWindow : public GraphicalWindow {
    protected:
        std::weak_ptr<VulkanDevice> m_vulkan_device;
        VkSurfaceKHR m_vk_surface;

        VkSwapchainKHR m_vk_swapchain;
        std::vector<VkImage> m_vk_swapchain_images;
        std::vector<VkImageView> m_vk_swapchain_image_views;
        std::vector<VkFramebuffer> m_vk_swapchain_framebuffers;
        VkRenderPass m_vk_swapchain_renderpass;
        VkFormat m_vk_swapchain_format;

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
            const std::shared_ptr<VulkanDevice>&,
            const std::string &title,
            const WindowTransform &transform,
            const WindowSetting &setting,
            const WindowStyle &style
        );

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