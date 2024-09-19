#ifndef ZEROENGINE_VULKAN_RENDER_TARGET_H
#define ZEROENGINE_VULKAN_RENDER_TARGET_H

#include <vulkan/vulkan.hpp>

#include <vector>

#include "zeroengine_core/RenderWindow.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"
#include "zeroengine_vulkan/VulkanContext.hpp"

namespace ZEROengine {
    /**
     * @brief A structure to manage Render Targets-related attributes.
     * 
     */
    class VulkanRenderWindow : public RenderWindow {
    private:
        VulkanContext *vulkan_context;
        
        VkSurfaceKHR vk_surface;
        VkSwapchainKHR vk_swapchain;
        std::vector<VkImage> vk_swapchain_images;
        std::vector<VkImageView> vk_swapchain_image_views;
        std::vector<VkFramebuffer> vk_swapchain_framebuffers;
        VkRenderPass vk_swapchain_renderpass;

        VkFormat vk_swapchain_format;

    private:
        VkSurfaceFormatKHR selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats);
        VkPresentModeKHR selectSwapchainPresentationMode(const std::vector<VkPresentModeKHR> &modes);
        bool enable_depth_stencil_subpass = false;

        ZEROResult initSwapChain();
        ZEROResult initSwapChainRenderPass();
        ZEROResult initSwapChainRenderTargets();
        
    public:
        VulkanRenderWindow();
        ZEROResult initVulkanRenderWindow(VulkanContext *vulkan_context, const VkSurfaceKHR &surface);
        ZEROResult getSwapchain(VkSwapchainKHR &ret) const;
        ZEROResult getFramebuffer(const uint32_t &index, VkFramebuffer &ret) const;
        ZEROResult getRenderPass(VkRenderPass &ret) const;
        virtual void handleResize(const uint32_t &new_frame_width, const uint32_t &new_frame_height) override;
        void reload_swapChain();
        void cleanup_swapChain();

        void cleanup();

    }; // class VulkanRenderTarget
} // namespace ZEROengine

#endif //#ifndef VULKAN_RENDER_TARGET_H