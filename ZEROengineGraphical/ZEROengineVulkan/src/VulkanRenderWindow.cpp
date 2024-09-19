#include "zeroengine_vulkan/VulkanRenderWindow.hpp"

#include <limits>

#include <vulkan/vk_enum_string_helper.h>

namespace ZEROengine {
    ZEROResult VulkanRenderWindow::initVulkanRenderWindow(VulkanContext *vulkan_context, const VkSurfaceKHR &surface) {
        this->vulkan_context = vulkan_context;
        this->vk_surface = surface;

        initSwapChain();
        initSwapChainRenderPass();
        initSwapChainRenderTargets();
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanRenderWindow::initSwapChain() {
        VkPhysicalDevice phys_device;
        VkDevice device;
        VkResult rslt;

        this->vulkan_context->getPhysicalDevice(phys_device);
        this->vulkan_context->getDevice(device);

        VulkanContext::SwapChainSupportDetails swap_chain_support = this->vulkan_context->querySwapChainSupport_Surface(this->vk_surface, phys_device);

        VkSurfaceFormatKHR surface_format = selectSwapchainSurfaceFormat(swap_chain_support.formats);
        VkPresentModeKHR present_mode = selectSwapchainPresentationMode(swap_chain_support.present_modes);
        if(swap_chain_support.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
            VkExtent2D current_extent = swap_chain_support.capabilities.currentExtent;
            this->setDimensions(current_extent.width, current_extent.height);
        } else {
            uint32_t actual_width = std::clamp(
                                        this->target_width, 
                                        swap_chain_support.capabilities.minImageExtent.width, 
                                        swap_chain_support.capabilities.maxImageExtent.width);
            uint32_t actual_height = std::clamp(
                                        this->target_height, 
                                        swap_chain_support.capabilities.minImageExtent.height, 
                                        swap_chain_support.capabilities.maxImageExtent.height);
            this->setDimensions(actual_width, actual_height);
        }
        VkExtent2D extent = { this->target_width, this->target_height };
        this->vk_swapchain_format = surface_format.format;
        
        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
        if(swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) 
            image_count = swap_chain_support.capabilities.maxImageCount;
        VkSwapchainCreateInfoKHR swap_chain_create_info{};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = this->vk_surface;
        swap_chain_create_info.minImageCount = image_count;
        swap_chain_create_info.imageFormat = surface_format.format;
        swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
        swap_chain_create_info.imageExtent = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        VkQueueInfo graphical_queue, presentation_queue;
        this->vulkan_context->getGraphicalQueue(graphical_queue);
        this->vulkan_context->getPresentationQueue(presentation_queue);
        if (graphical_queue.queueFamilyIndex != presentation_queue.queueFamilyIndex) {
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount = 2;
            uint32_t queue_indices[2] = { graphical_queue.queueFamilyIndex, presentation_queue.queueFamilyIndex};
            swap_chain_create_info.pQueueFamilyIndices = queue_indices;
        } else {
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
            swap_chain_create_info.queueFamilyIndexCount = 0; // Optional
            swap_chain_create_info.pQueueFamilyIndices = nullptr; // Optional
        }
        swap_chain_create_info.preTransform = swap_chain_support.capabilities.currentTransform;
        swap_chain_create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
        swap_chain_create_info.presentMode = present_mode;
        swap_chain_create_info.clipped = VK_TRUE;
        swap_chain_create_info.oldSwapchain = VK_NULL_HANDLE;
        if((rslt = vkCreateSwapchainKHR(device, &swap_chain_create_info, nullptr, &this->vk_swapchain)) != VK_SUCCESS) {
            return { ZERO_FAILED, "" };
            // return
        }

        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanRenderWindow::getSwapchain(VkSwapchainKHR &ret) const {
        ret = this->vk_swapchain;
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanRenderWindow::getFramebuffer(const uint32_t &index, VkFramebuffer &ret) const {
        ret = this->vk_swapchain_framebuffers[index];
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanRenderWindow::getRenderPass(VkRenderPass &ret) const {
        ret = this->vk_swapchain_renderpass;
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanRenderWindow::initSwapChainRenderPass() {
        VkDevice device;
        this->vulkan_context->getDevice(device);

        std::array<VkAttachmentDescription, 2> attachments = {};

        attachments[0].format = this->vk_swapchain_format;
        attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
        attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        attachments[0].finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

        VkAttachmentReference color_attachment_ref{};
        color_attachment_ref.attachment = 0;
        color_attachment_ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

        VkSubpassDescription subpass{};
        subpass.colorAttachmentCount = 1;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.pColorAttachments = &color_attachment_ref;

        std::array<VkSubpassDependency, 2> subpass_dep = {};
        subpass_dep[0].srcSubpass = VK_SUBPASS_EXTERNAL;
        subpass_dep[0].dstSubpass = 0;
        subpass_dep[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
        subpass_dep[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        subpass_dep[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
        subpass_dep[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
        subpass_dep[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

        /* future support
        if(this->settings.enableDepthStencil) {
            attachments[1].format = VK_FORMAT_D16_UNORM;
            attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
            attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
            attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
            attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
            attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
            attachments[1].finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            VkAttachmentReference depth_attachment_ref = {};
            depth_attachment_ref.attachment = 1;
            depth_attachment_ref.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

            subpass.pDepthStencilAttachment = &depth_attachment_ref;
            subpass_dep[1].srcSubpass = VK_SUBPASS_EXTERNAL;
            subpass_dep[1].dstSubpass = 0;
            subpass_dep[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
            subpass_dep[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
            subpass_dep[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
            subpass_dep[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
            subpass_dep[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
        }
        */

        VkRenderPassCreateInfo render_pass_info{};
        render_pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        render_pass_info.attachmentCount = static_cast<uint32_t>(this->enable_depth_stencil_subpass ? 2 : 1);
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = static_cast<uint32_t>(this->enable_depth_stencil_subpass ? 2 : 1);
        render_pass_info.pDependencies = subpass_dep.data();
        VkResult rslt = vkCreateRenderPass(device, &render_pass_info, nullptr, &this->vk_swapchain_renderpass);
        if (rslt != VK_SUCCESS) {
            return { ZERO_FAILED, "" };
        }
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanRenderWindow::initSwapChainRenderTargets() {
        VkDevice device;
        this->vulkan_context->getDevice(device);
        VkResult rslt;

        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(device, this->vk_swapchain, &image_count, nullptr);
        this->vk_swapchain_images.resize(image_count);
        this->vk_swapchain_image_views.resize(image_count);
        this->vk_swapchain_framebuffers.resize(image_count);
        if((rslt = vkGetSwapchainImagesKHR(device, this->vk_swapchain, &image_count, this->vk_swapchain_images.data())) != VK_SUCCESS) {
            // return
        }

        for(std::size_t i = 0; i < image_count; ++i) {
            VkImageViewCreateInfo img_view_create_info{};
            img_view_create_info.format = this->vk_swapchain_format;
            img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            img_view_create_info.image = this->vk_swapchain_images[i];
            img_view_create_info.viewType = VK_IMAGE_VIEW_TYPE_2D;
            img_view_create_info.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
            img_view_create_info.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
            img_view_create_info.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
            img_view_create_info.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
            img_view_create_info.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            img_view_create_info.subresourceRange.layerCount = 1;
            img_view_create_info.subresourceRange.levelCount = 1;
            img_view_create_info.subresourceRange.baseMipLevel = 0;
            img_view_create_info.subresourceRange.baseArrayLayer = 0;
            if((rslt = vkCreateImageView(device, &img_view_create_info, nullptr, &this->vk_swapchain_image_views[i])) != VK_SUCCESS) {
                return { ZERO_FAILED, "" };
            }

            VkImageView* img_view_ref = &this->vk_swapchain_image_views[i];
            VkFramebufferCreateInfo framebuffers_create_info{};
            framebuffers_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffers_create_info.width = this->target_width;
            framebuffers_create_info.height = this->target_height;
            framebuffers_create_info.layers = 1;
            framebuffers_create_info.attachmentCount = 1;
            framebuffers_create_info.pAttachments = img_view_ref;
            framebuffers_create_info.renderPass = this->vk_swapchain_renderpass;
            if((rslt = vkCreateFramebuffer(
                device, 
                &framebuffers_create_info, 
                nullptr, 
                &this->vk_swapchain_framebuffers[i]))
            != VK_SUCCESS) {
                return { ZERO_FAILED, "" };
            }
        }
        return { ZERO_SUCCESS, "" };
    }

    VkSurfaceFormatKHR VulkanRenderWindow::selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR VulkanRenderWindow::selectSwapchainPresentationMode(const std::vector<VkPresentModeKHR> &modes) {
        for(const auto& mode : modes) {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    void VulkanRenderWindow::handleResize(const uint32_t &new_frame_width, const uint32_t &new_frame_height) {
        VkDevice device;
        this->vulkan_context->getDevice(device);

        this->setDimensions(new_frame_width, new_frame_height);
        vkDeviceWaitIdle(device);
        this->reload_swapChain();
    }

    void VulkanRenderWindow::cleanup_swapChain() {
        VkDevice device;
        this->vulkan_context->getDevice(device);

        for(auto &framebuffer : this->vk_swapchain_framebuffers) {
            vkDestroyFramebuffer(device, framebuffer, nullptr);
        }
        for(auto &img_view : this->vk_swapchain_image_views) {
            vkDestroyImageView(device, img_view, nullptr);
        }
        vkDestroySwapchainKHR(device, this->vk_swapchain, nullptr);
    }

    void VulkanRenderWindow::reload_swapChain() {
        // should only be called in context when none of these is in used.
        cleanup_swapChain();

        initSwapChain();
        initSwapChainRenderTargets();
    }

    void VulkanRenderWindow::cleanup() {
        VkDevice device;
        this->vulkan_context->getDevice(device);
        vkDestroyRenderPass(device, this->vk_swapchain_renderpass, nullptr);
        cleanup_swapChain();
    }
} // namespace ZEROengine
