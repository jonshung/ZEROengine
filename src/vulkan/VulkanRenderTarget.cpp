#include "VulkanRenderTarget.hpp"

void VulkanRenderTarget::set(
    const VkImage &image, 
    const VkImageView &image_view, 
    const VkFramebuffer &framebuffer, 
    const VkFormat &framebuffer_format,
    const VkExtent2D &framebuffer_extent,
    const std::shared_ptr<VkRenderPass> &render_pass
) {
    this->image = image;
    this->image_view = image_view;
    this->framebuffer = framebuffer;
    this->framebuffer_format = framebuffer_format;
    this->framebuffer_extent = framebuffer_extent;
    this->render_pass = render_pass; // copying to a new shared_ptr
}