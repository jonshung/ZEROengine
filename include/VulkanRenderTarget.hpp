#ifndef ZEROENGINE_VULKAN_RENDER_TARGET_H
#define ZEROENGINE_VULKAN_RENDER_TARGET_H

#include <vulkan/vulkan.hpp>
#include <memory>

struct VulkanRenderTargetSettings {
    bool enableDepthStencil = false;
};

/**
 * @brief A structure to manage Render Targets-related attributes.
 * 
 */
class VulkanRenderTarget {
public:
    VkImage image;
    VkImageView image_view;
    VkFramebuffer framebuffer;
    std::shared_ptr<VkRenderPass> render_pass;

    VkFormat framebuffer_format;
    VkExtent2D framebuffer_extent;

    /**
     * @brief Fast utility to set all attributes at once
     * 
     * @param image 
     * @param image_view 
     * @param framebuffer 
     * @param framebuffer_format 
     * @param framebuffer_extent 
     * @param render_pass 
     */
    void set(
        const VkImage &image, 
        const VkImageView &image_view, 
        const VkFramebuffer &framebuffer, 
        const VkFormat &framebuffer_format,
        const VkExtent2D &framebuffer_extent,
        const std::shared_ptr<VkRenderPass> &render_pass
    );
};

#endif //#ifndef VULKAN_RENDER_TARGET_H