#ifndef ZEROENGINE_VULKAN_WINDOW_H
#define ZEROENGINE_VULKAN_WINDOW_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <cstdint>
#include <unordered_map>

#include "zeroengine_core/WindowSupport.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"
#include "zeroengine_vulkan/VulkanContext.hpp"

namespace ZEROengine {
    /**
     * @brief A structure to manage Render Targets-related attributes.
     * 
     */
    class VulkanWindow : public WindowSupport {
    protected:
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
    
    protected:
        void initSwapChain();
        void initSwapChainRenderPass();
        void initSwapChainRenderTargets();
        
    public:
        VulkanWindow(
            VulkanContext *const &vulkan_context,
            const std::string &title,
            const WindowTransform &transform,
            const WindowSetting &setting,
            const WindowStyle &style
        );

        VkSwapchainKHR* getSwapchain();
        VkFramebuffer getFramebuffer(const uint32_t &index) const;
        VkRenderPass getRenderPass() const;
        VkSurfaceKHR getSurface() const;

        void reload_swapChain();
        void cleanup_swapChain();

        virtual void cleanup() override;

    }; // class VulkanWindow
} // namespace ZEROengine

#endif //#ifndef ZEROENGINE_VULKAN_WINDOW_H