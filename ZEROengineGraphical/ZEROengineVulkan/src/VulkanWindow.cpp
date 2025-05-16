#include "zeroengine_vulkan/VulkanWindow.hpp"

#include <limits>

namespace ZEROengine {
    VulkanWindow::VulkanWindow(
        const VkDevice &vk_device,
        const VkPhysicalDevice &vk_phys_device,
        const VkSurfaceKHR &vk_surface,
        const std::string &title,
        const WindowTransform &transform,
        const WindowSetting &setting,
        const WindowStyle &style
    ) :
    GPUWindow(title, transform, setting, style),
    m_vk_device{vk_device},
    m_vk_phys_device{vk_phys_device},
    m_vk_surface{vk_surface},
    m_vulkan_presentation_queue{},
    m_vk_swapchain{},
    m_vk_swapchain_images{},
    m_vk_swapchain_image_views{},
    m_vk_swapchain_framebuffers{},
    m_vk_swapchain_renderpass{},
    m_vk_swapchain_format{},
    m_graphical_queue_family{-1},
    m_acquired_swapchain{}
    {}

    void VulkanWindow::init() {
        std::optional<uint32_t> queue_family_index = queryPresentationQueueIndex();
        if(!queue_family_index.has_value()) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Cannot find a presentation queue matching the requested surface.");
        }
        vkGetDeviceQueue(m_vk_device, queue_family_index.value(), 0, &m_vulkan_presentation_queue.queue);
        if(m_vulkan_presentation_queue.queue == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Cannot get a presentation queue!");
        }
        m_vulkan_presentation_queue.queueFamilyIndex = queue_family_index.value();
    }

    void VulkanWindow::setGraphicalQueueFamily(const uint32_t &v) {
        m_graphical_queue_family = static_cast<int64_t>(v);
    }

    void VulkanWindow::initSwapChain() {
        SwapChainSupportDetails swap_chain_support = querySwapChainSupport();

        VkSurfaceFormatKHR surface_format = selectSwapchainSurfaceFormat(swap_chain_support.formats);
        VkPresentModeKHR present_mode = selectSwapchainPresentationMode(swap_chain_support.present_modes);

        uint32_t actual_width = std::clamp(
                                    getWidth(), 
                                    swap_chain_support.capabilities.minImageExtent.width, 
                                    swap_chain_support.capabilities.maxImageExtent.width);
        uint32_t actual_height = std::clamp(
                                    getHeight(), 
                                    swap_chain_support.capabilities.minImageExtent.height, 
                                    swap_chain_support.capabilities.maxImageExtent.height);
        GPUWindow::resize(actual_width, actual_height);
        
        VkExtent2D extent = { getWidth(), getHeight() };
        m_vk_swapchain_format = surface_format.format;
        
        uint32_t image_count = swap_chain_support.capabilities.minImageCount + 1;
        if(swap_chain_support.capabilities.maxImageCount > 0 && image_count > swap_chain_support.capabilities.maxImageCount) 
            image_count = swap_chain_support.capabilities.maxImageCount;
        VkSwapchainCreateInfoKHR swap_chain_create_info{};
        swap_chain_create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
        swap_chain_create_info.surface = m_vk_surface;
        swap_chain_create_info.minImageCount = image_count;
        swap_chain_create_info.imageFormat = surface_format.format;
        swap_chain_create_info.imageColorSpace = surface_format.colorSpace;
        swap_chain_create_info.imageExtent = extent;
        swap_chain_create_info.imageArrayLayers = 1;
        swap_chain_create_info.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
        
        if(m_graphical_queue_family == -1) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "VulkanWindow cannot find a graphical queue.");
        }
        if(m_vulkan_presentation_queue.queue == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "VulkanWindow cannot find a presentation queue.");
        }
        
        if (m_graphical_queue_family != m_vulkan_presentation_queue.queueFamilyIndex) {
            swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
            swap_chain_create_info.queueFamilyIndexCount = 2;
            uint32_t queue_indices[2] = { static_cast<uint32_t>(m_graphical_queue_family), m_vulkan_presentation_queue.queueFamilyIndex};
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
        ZERO_VK_CHECK_EXCEPT(vkCreateSwapchainKHR(m_vk_device, &swap_chain_create_info, nullptr, &m_vk_swapchain));
    }

    VkSwapchainKHR VulkanWindow::getSwapchain() const {
        return m_vk_swapchain;
    }

    VkFramebuffer VulkanWindow::getFramebuffer(const uint32_t &index) const {
        return m_vk_swapchain_framebuffers[index];
    }

    VkRenderPass VulkanWindow::getRenderPass() const {
        return m_vk_swapchain_renderpass;
    }

    VkSurfaceKHR VulkanWindow::getSurface() const {
        return m_vk_surface;
    }

    // TODO: implement this
    // SwapChainSupportDetails VulkanWindow::querySwapChainSupport() {
    //     std::shared_ptr<VulkanDevice> vulkan_device = m_vulkan_device.lock();
    //     if(!vulkan_device) {
    //         ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Vulkan Device expired.");
    //     }
    //     VkPhysicalDevice phys_device = vulkan_device->getPhysicalDevice();

    //     SwapChainSupportDetails details{};
    //     uint32_t format_count = 0, present_mode_count = 0;
    //     (void) phys_device;
    //     (void) format_count;
    //     (void) present_mode_count;
    //     #if defined(VK_USE_PLATFORM_ANDROID_KHR)
    //         // not supported yet
    //     #elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
    //         // not supported yet
    //     #elif defined(VK_USE_PLATFORM_XLIB_KHR)
    //         // not supported yet
    //     #endif
    //     return details;
    // }

    SwapChainSupportDetails VulkanWindow::querySwapChainSupport() {
        SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(m_vk_phys_device, m_vk_surface, &details.capabilities);

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_phys_device, m_vk_surface, &format_count, nullptr);
        if(format_count > 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(m_vk_phys_device, m_vk_surface, &format_count, details.formats.data());
        }

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_phys_device, m_vk_surface, &present_mode_count, nullptr);
        if(present_mode_count > 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(m_vk_phys_device, m_vk_surface, &present_mode_count, details.present_modes.data());
        }
        return details;
    }

    std::optional<uint32_t> VulkanWindow::queryPresentationQueueIndex() const {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(m_vk_phys_device, &queue_family_count, nullptr);
        for(uint32_t i = 0; i < queue_family_count; ++i) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(m_vk_phys_device, i, m_vk_surface, &presentSupport);

            if(presentSupport) {
                return std::optional<uint32_t>(static_cast<uint32_t>(i));
            }
        }
        return {};
    }

    std::optional<VkDeviceQueueCreateInfo> VulkanWindow::queryPresentationQueueCreation() const {
        std::optional<uint32_t> queue_index = queryPresentationQueueIndex();
        if(!queue_index.has_value()) {
            return {};
        }
        float queuePriority = 1.0f;

        VkDeviceQueueCreateInfo queue_info{};
        queue_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_info.pNext = nullptr;
        queue_info.queueCount = 1;
        queue_info.queueFamilyIndex = queue_index.value();
        queue_info.pQueuePriorities = &queuePriority;
        return std::optional(queue_info);
    }

    void VulkanWindow::initSwapChainRenderPass() {
        std::array<VkAttachmentDescription, 2> attachments = {};

        attachments[0].format = m_vk_swapchain_format;
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
        if(settings.enableDepthStencil) {
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
        render_pass_info.attachmentCount = static_cast<uint32_t>(m_enable_depth_stencil_subpass ? 2 : 1);
        render_pass_info.pAttachments = attachments.data();
        render_pass_info.subpassCount = 1;
        render_pass_info.pSubpasses = &subpass;
        render_pass_info.dependencyCount = static_cast<uint32_t>(m_enable_depth_stencil_subpass ? 2 : 1);
        render_pass_info.pDependencies = subpass_dep.data();
        ZERO_VK_CHECK_EXCEPT(vkCreateRenderPass(m_vk_device, &render_pass_info, nullptr, &m_vk_swapchain_renderpass));
    }

    void VulkanWindow::initSwapChainRenderTargets() {
        uint32_t image_count = 0;
        vkGetSwapchainImagesKHR(m_vk_device, m_vk_swapchain, &image_count, nullptr);
        m_vk_swapchain_images.resize(image_count);
        m_vk_swapchain_image_views.resize(image_count);
        m_vk_swapchain_framebuffers.resize(image_count);
        ZERO_VK_CHECK_EXCEPT(vkGetSwapchainImagesKHR(m_vk_device, m_vk_swapchain, &image_count, m_vk_swapchain_images.data()));

        for(std::size_t i = 0; i < image_count; ++i) {
            VkImageViewCreateInfo img_view_create_info{};
            img_view_create_info.format = m_vk_swapchain_format;
            img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
            img_view_create_info.image = m_vk_swapchain_images[i];
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
            ZERO_VK_CHECK_EXCEPT(vkCreateImageView(m_vk_device, &img_view_create_info, nullptr, &m_vk_swapchain_image_views[i]));

            VkImageView* img_view_ref = &m_vk_swapchain_image_views[i];
            VkFramebufferCreateInfo framebuffers_create_info{};
            framebuffers_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
            framebuffers_create_info.width = getWidth();
            framebuffers_create_info.height = getHeight();
            framebuffers_create_info.layers = 1;
            framebuffers_create_info.attachmentCount = 1;
            framebuffers_create_info.pAttachments = img_view_ref;
            framebuffers_create_info.renderPass = m_vk_swapchain_renderpass;
            ZERO_VK_CHECK_EXCEPT(vkCreateFramebuffer(
                m_vk_device, 
                &framebuffers_create_info, 
                nullptr, 
                &m_vk_swapchain_framebuffers[i]));
        }
    }

    VkSurfaceFormatKHR VulkanWindow::selectSwapchainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
        for (const auto& format : formats) {
            if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
                return format;
            }
        }

        return formats[0];
    }

    VkPresentModeKHR VulkanWindow::selectSwapchainPresentationMode(const std::vector<VkPresentModeKHR> &modes) {
        for(const auto& mode : modes) {
            if(mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
        }
        return VK_PRESENT_MODE_FIFO_KHR;
    }

    bool VulkanWindow::tryAcquireSwapchainImage(const VkSemaphore &wait_semaphore, const uint32_t &call_depth) {
        if(call_depth == 64u) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Maximum swapchin reacquisition depth.");
        }
        
        VkResult rslt = vkAcquireNextImageKHR(
            m_vk_device, m_vk_swapchain, 
            UINT64_MAX, 
            wait_semaphore, 
            VK_NULL_HANDLE, 
            &m_acquired_swapchain
        ); // handle resize
        if(rslt == VK_ERROR_OUT_OF_DATE_KHR) {
            handleMoveOrResize();
            // reacquire
            return tryAcquireSwapchainImage(wait_semaphore, call_depth+1);
        } else if(rslt != VK_SUCCESS && rslt != VK_SUBOPTIMAL_KHR) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Error while trying to acquire swapchain image.");
        }
        return call_depth == 0;
    }

    uint32_t VulkanWindow::getAcquiredSwapchain() const {
        return m_acquired_swapchain;
    }

    void VulkanWindow::cleanup_swapChain() {
        for(auto &framebuffer : m_vk_swapchain_framebuffers) {
            vkDestroyFramebuffer(m_vk_device, framebuffer, nullptr);
        }
        for(auto &img_view : m_vk_swapchain_image_views) {
            vkDestroyImageView(m_vk_device, img_view, nullptr);
        }
        vkDestroySwapchainKHR(m_vk_device, m_vk_swapchain, nullptr);
    }

    void VulkanWindow::reload_swapChain() {
        // should only be called in context when none of these is in used.
        cleanup_swapChain();

        initSwapChain();
        initSwapChainRenderTargets();
    }

    void VulkanWindow::cleanup() {
        vkDestroyRenderPass(m_vk_device, m_vk_swapchain_renderpass, nullptr);
        cleanup_swapChain();
    }
} // namespace ZEROengine
