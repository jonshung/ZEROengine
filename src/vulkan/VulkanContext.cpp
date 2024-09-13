#include "VulkanContext.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include <map>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <unordered_set>
#include <memory>

#include "project_env.h"

void VulkanContext::initVulkan(WindowContext* const &window_context) {
    if(dbg_enable_validation_layers && !checkValidationLayersSupport()) {
        throw std::runtime_error("validation layers requested, but not available!");
    }
    this->window_ctx = window_context;
    this->vk_swapchain_render_pass = std::make_shared<VkRenderPass>();
    // preserve order
    VKInit_initInstance();
    VKInit_initWindowSurface();
    VKInit_initPhysicalDevice();
    VKInit_initLogicalDevice();
    
    VKInit_initSwapChain();
    VKInit_initSwapChainRenderPass();
    VKInit_initSwapChainRenderTargets();
}

void VulkanContext::VKInit_initInstance() {
    VkApplicationInfo app_info{};
    app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    app_info.pApplicationName = "ZEROengine";
    app_info.applicationVersion = VK_MAKE_VERSION(ZEROENGINE_VERSION_MAJOR, ZEROENGINE_VERSION_MINOR, ZEROENGINE_VERSION_PATCH);
    app_info.pEngineName = "ZEROengine";
    app_info.engineVersion = VK_MAKE_VERSION(ZEROENGINE_VERSION_MAJOR, ZEROENGINE_VERSION_MINOR, ZEROENGINE_VERSION_PATCH);
    app_info.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo create_info{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &app_info;

    uint32_t window_extension_count;
    const char*const* window_extensions = this->window_ctx->requestInstanceExtensions(window_extension_count);
    std::string val_ret;
    if(!validateExtensionsSupport(window_extension_count, window_extensions, &val_ret)) {
        throw std::runtime_error("initVulkan() failed, err: insufficient instance extensions, missing " + val_ret);
    }

    create_info.enabledExtensionCount = window_extension_count;
    create_info.ppEnabledExtensionNames = window_extensions;
    create_info.enabledLayerCount = 0;

    if (dbg_enable_validation_layers) {
        create_info.enabledLayerCount = static_cast<uint32_t>(dbg_validation_layers.size());
        create_info.ppEnabledLayerNames = dbg_validation_layers.data();
    } else {
        create_info.enabledLayerCount = 0;
    }
    VkResult rslt;
    if((rslt = vkCreateInstance(&create_info, nullptr, &this->vk_instance)) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateInstance() failed, err: " + std::string(string_VkResult(rslt)));
    }

}

void VulkanContext::VKInit_initWindowSurface() {
    this->window_ctx->getSurface(this->vk_instance, &this->vk_surface);
}

void VulkanContext::VKInit_initPhysicalDevice() {
    uint32_t device_count = 0;
    vkEnumeratePhysicalDevices(this->vk_instance, &device_count, nullptr);
    if(device_count == 0) {
        throw std::runtime_error("initVulkan() failed, error: failed to find GPUs with Vulkan support");
    }
    std::vector<VkPhysicalDevice> phys_devices(device_count);
    vkEnumeratePhysicalDevices(this->vk_instance, &device_count, phys_devices.data());

    // choose the best device
    selectPhysicalDeviceVulkan(phys_devices);
    if(this->vk_physical_device == VK_NULL_HANDLE) {
        throw std::runtime_error("initVulkan() failed, error: failed to find suitable GPU for Vulkan");
    }
}

void VulkanContext::VKInit_initLogicalDevice() {
    QueueFamilyIndices queue_families = queryQueueFamily(this->vk_physical_device, {VK_QUEUE_GRAPHICS_BIT});
    std::unordered_set<uint32_t> queue_indices;
    for(auto &[family, index] : queue_families.queue_indices) {
        queue_indices.insert(index.value());
    }
    this->vk_graphics_queue.queueFamilyIndex = queue_families.queue_indices[VK_QUEUE_GRAPHICS_BIT].value(); // should exists since we already check suitability.
    int32_t presentation_queue = queryPresentationQueueFamily(this->vk_physical_device); // again, should exists
    this->vk_presentation_queue.queueFamilyIndex = presentation_queue;
    queue_indices.insert(static_cast<uint32_t>(presentation_queue));
    std::vector<VkDeviceQueueCreateInfo> queue_create_infos;

    float queuePriority = 1.0f;
    for (uint32_t queueFamily : queue_indices) {
        VkDeviceQueueCreateInfo queue_create_info{};
        queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        queue_create_info.queueFamilyIndex = queueFamily;
        queue_create_info.queueCount = 1;
        queue_create_info.pQueuePriorities = &queuePriority;
        queue_create_infos.push_back(queue_create_info);
    }

    VkPhysicalDeviceFeatures device_features{};

    VkDeviceCreateInfo device_create_info{};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pQueueCreateInfos = queue_create_infos.data();
    device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
    device_create_info.pEnabledFeatures = &device_features;
    device_create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
    device_create_info.ppEnabledExtensionNames = device_extensions.data();

    if(dbg_enable_validation_layers) {
        device_create_info.enabledLayerCount = static_cast<uint32_t>(dbg_validation_layers.size());
        device_create_info.ppEnabledLayerNames = dbg_validation_layers.data();
    } else {
        device_create_info.enabledLayerCount = 0;
    }
    VkResult rslt;
    if((rslt = vkCreateDevice(this->vk_physical_device, &device_create_info, nullptr, &this->vk_device)) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateDevice() failed, err: " + std::string(string_VkResult(rslt)));
    }
    vkGetDeviceQueue(this->vk_device, this->vk_graphics_queue.queueFamilyIndex, 0, &this->vk_graphics_queue.queue);
    vkGetDeviceQueue(this->vk_device, this->vk_presentation_queue.queueFamilyIndex, 0, &this->vk_presentation_queue.queue);
}

void VulkanContext::VKInit_initSwapChain() {
    SwapChainSupportDetails swap_chain_support = querySwapChainSupport(this->vk_physical_device);

    VkSurfaceFormatKHR surface_format = selectSwapChainSurfaceFormat(swap_chain_support.formats);
    VkPresentModeKHR present_mode = selectSwapChainPresentationMode(swap_chain_support.present_modes);
    VkExtent2D extent = this->vk_swapchain_extent = selectSwapChainExtent(swap_chain_support.capabilities);
    this->vk_swapchain_image_format = surface_format.format;
    
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
    
    if (this->vk_graphics_queue.queueFamilyIndex != this->vk_presentation_queue.queueFamilyIndex) {
        swap_chain_create_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        swap_chain_create_info.queueFamilyIndexCount = 2;
        uint32_t queue_indices[2] = { this->vk_graphics_queue.queueFamilyIndex, this->vk_presentation_queue.queueFamilyIndex};
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
    VkResult rslt;
    if((rslt = vkCreateSwapchainKHR(this->vk_device, &swap_chain_create_info, nullptr, &this->vk_swapchain)) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateSwapchainKHR() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanContext::VKInit_initSwapChainRenderPass() {
    std::array<VkAttachmentDescription, 2> attachments = {};

    attachments[0].format = this->vk_swapchain_image_format;
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
    VkResult rslt = vkCreateRenderPass(this->vk_device, &render_pass_info, nullptr, this->vk_swapchain_render_pass.get());
    if (rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateRenderPass failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanContext::VKInit_initSwapChainRenderTargets() {
    uint32_t image_count = 0;
    vkGetSwapchainImagesKHR(this->vk_device, this->vk_swapchain, &image_count, nullptr);
    this->vk_swapchain_render_targets.resize(image_count);
    std::vector<VkImage> image_buffer(image_count);
    vkGetSwapchainImagesKHR(this->vk_device, this->vk_swapchain, &image_count, image_buffer.data());

    VkResult rslt;
    for(std::size_t i = 0; i < image_buffer.size(); ++i) {
        this->vk_swapchain_render_targets[i].image = image_buffer[i];
        this->vk_swapchain_render_targets[i].framebuffer_extent = this->vk_swapchain_extent;
        this->vk_swapchain_render_targets[i].framebuffer_format = this->vk_swapchain_image_format;
        this->vk_swapchain_render_targets[i].render_pass = this->vk_swapchain_render_pass;

        VkImageViewCreateInfo img_view_create_info{};
        img_view_create_info.format = this->vk_swapchain_image_format;
        img_view_create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        img_view_create_info.image = image_buffer[i];
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
        if((rslt = vkCreateImageView(this->vk_device, &img_view_create_info, nullptr, &this->vk_swapchain_render_targets[i].image_view)) != VK_SUCCESS) {
            throw std::runtime_error("vkCreateImageView() failed, err: " + std::string(string_VkResult(rslt)));
        }

        VkImageView* img_view_ref = &this->vk_swapchain_render_targets[i].image_view;
        VkFramebufferCreateInfo framebuffers_create_info{};
        framebuffers_create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebuffers_create_info.width = this->vk_swapchain_extent.width;
        framebuffers_create_info.height = this->vk_swapchain_extent.height;
        framebuffers_create_info.layers = 1;
        framebuffers_create_info.attachmentCount = 1;
        framebuffers_create_info.pAttachments = img_view_ref;
        framebuffers_create_info.renderPass = *this->vk_swapchain_render_pass;
        if((rslt = vkCreateFramebuffer(this->vk_device, &framebuffers_create_info, nullptr, &this->vk_swapchain_render_targets[i].framebuffer)) != VK_SUCCESS) {
            throw std::runtime_error("vkCreateFramebuffer() failed, err: + " + std::string(string_VkResult(rslt)));
        }
    }
}

bool VulkanContext::checkValidationLayersSupport(std::string *ret) {
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::unordered_set<std::string> validation_intersection;
    for(VkLayerProperties &view : availableLayers) {
        validation_intersection.insert(view.layerName);
    }
    for(const char* layer_char : dbg_validation_layers) {
        std::string layer_name = std::string(layer_char);
        if(validation_intersection.find(layer_name) == validation_intersection.end()) {
            if(ret) *ret = layer_name;
            return false;
        }
    }
    return true;
}

bool VulkanContext::validateExtensionsSupport(const uint32_t &extension_count, const char*const* extensions, std::string *ret) {
    uint32_t vk_extension_count;
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_extension_count, nullptr);
    std::vector<VkExtensionProperties> vk_extensions(vk_extension_count);
    vkEnumerateInstanceExtensionProperties(nullptr, &vk_extension_count, vk_extensions.data());

    std::unordered_set<std::string> validation_intersection;
    for(VkExtensionProperties &view : vk_extensions) {
        validation_intersection.insert(view.extensionName);
    }
    for(uint32_t i = 0; i < extension_count; ++i) {
        std::string ext_name = std::string(extensions[i]);
        if(validation_intersection.find(ext_name) == validation_intersection.end()) {
            if(ret) *ret = ext_name;
            return false;
        }
    }
    return true;
}

void VulkanContext::selectPhysicalDeviceVulkan(std::vector<VkPhysicalDevice> &phys_devices) {
    std::multimap<uint32_t, const VkPhysicalDevice&, std::greater<uint32_t>> device_scores;

    for(VkPhysicalDevice &dev : phys_devices) {
        device_scores.insert({evaluatePhysicalDeviceSuitability(dev), dev});
    }
    auto it = device_scores.begin();
    if(it->first == 0) return;
    this->vk_physical_device = it->second;
}

uint32_t VulkanContext::evaluatePhysicalDeviceSuitability(const VkPhysicalDevice &phys_device) {
    uint32_t score = 0;
    VkPhysicalDeviceProperties device_properties;
    VkPhysicalDeviceFeatures device_features;
    vkGetPhysicalDeviceProperties(phys_device, &device_properties);
    vkGetPhysicalDeviceFeatures(phys_device, &device_features);

    if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
    else score += 10;
    score += device_properties.limits.maxImageDimension2D;

    // preliminary checks
    if(!device_features.geometryShader) return 0; // since we need geometry shader
    QueueFamilyIndices queue_indices = queryQueueFamily(phys_device, {VK_QUEUE_GRAPHICS_BIT});
    int32_t presentation_queue = queryPresentationQueueFamily(phys_device);
    if(!queue_indices.exists() || presentation_queue == -1) return 0;
    if(!checkDeviceExtensionSupport(phys_device)) return 0;
    
    bool swapChainAdequate = false;
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport(phys_device);
    swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
    if(!swapChainAdequate) return 0;
    

    return score;
}

bool VulkanContext::checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device) {
    uint32_t extension_count;
    vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extension_count);
    vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count, availableExtensions.data());

    std::unordered_set<std::string> required_extensions(device_extensions.begin(), device_extensions.end());

    for (const auto& extension : availableExtensions) {
        required_extensions.erase(extension.extensionName);
    }
    return required_extensions.empty();
}

VulkanContext::QueueFamilyIndices VulkanContext::queryQueueFamily(const VkPhysicalDevice &phys_device, const std::vector<uint32_t> &query) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);

    std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, queue_families.data());
    
    QueueFamilyIndices q_index;
    std::unordered_set<uint32_t> query_set(query.begin(), query.end());
    uint32_t query_size = query_set.size();

    uint32_t i = 0;
    for(const auto& family : queue_families) {
        if(query_size <= 0) break; // found all queries
        for(const uint32_t& q : query_set) {
            if(q_index.queue_indices[q].has_value()) continue; // already found a queue
            if(family.queueFlags & q) {
                q_index.queue_indices[q] = i;
                query_size--;
            }
        }
        i++;
    }
    return q_index;
}

int32_t VulkanContext::queryPresentationQueueFamily(const VkPhysicalDevice &phys_device) {
    uint32_t queue_family_count = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);
    for(uint32_t i = 0; i < queue_family_count; ++i) {
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, i, this->vk_surface, &presentSupport);
        if(presentSupport) return static_cast<int32_t>(i);
        i++;
    }
    return -1;
}

VulkanContext::SwapChainSupportDetails VulkanContext::querySwapChainSupport(const VkPhysicalDevice &phys_device) {
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, this->vk_surface, &details.capabilities);

    uint32_t format_count;
    vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, this->vk_surface, &format_count, nullptr);
    if(format_count > 0) {
        details.formats.resize(format_count);
        vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, this->vk_surface, &format_count, details.formats.data());
    }

    uint32_t present_mode_count;
    vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, this->vk_surface, &present_mode_count, nullptr);
    if(present_mode_count > 0) {
        details.present_modes.resize(present_mode_count);
        vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, this->vk_surface, &present_mode_count, details.present_modes.data());
    }
    return details;
}

VkSurfaceFormatKHR VulkanContext::selectSwapChainSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &formats) {
    for (const auto& format : formats) {
        if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return format;
        }
    }

    return formats[0];
}


VkPresentModeKHR VulkanContext::selectSwapChainPresentationMode(const std::vector<VkPresentModeKHR> &modes) {
    for(const auto& mode : modes) {
        if(mode == VK_PRESENT_MODE_MAILBOX_KHR) return mode;
    }
    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D VulkanContext::selectSwapChainExtent(const VkSurfaceCapabilitiesKHR &capabilities) {
    if(capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    }
    int w, h;
    this->window_ctx->getFramebufferSize(w, h);
    VkExtent2D actual_extent = {
            static_cast<uint32_t>(w),
            static_cast<uint32_t>(h)
    };
    actual_extent.width = std::clamp(actual_extent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
    actual_extent.height = std::clamp(actual_extent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
    return actual_extent;
}

VkResult VulkanContext::acquireSwapChainImageIndex(uint32_t &index, VkSemaphore semaphore_lock) {
    return vkAcquireNextImageKHR(this->vk_device, this->vk_swapchain, UINT64_MAX, semaphore_lock, VK_NULL_HANDLE, &index);
}

void VulkanContext::waitForFence(VkFence fence) {
    vkWaitForFences(this->vk_device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void VulkanContext::releaseFence(VkFence fence) {
    vkResetFences(this->vk_device, 1, &fence);
}

void VulkanContext::presentLatestImage(const uint32_t &image_index, VkSemaphore semaphore_lock) {
    VkPresentInfoKHR present_info{};
    present_info.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    present_info.waitSemaphoreCount = 1;
    present_info.pWaitSemaphores = &semaphore_lock;
    present_info.swapchainCount = 1;
    present_info.pSwapchains = &this->vk_swapchain;
    present_info.pImageIndices = &image_index;
    present_info.pResults = nullptr;
    VkResult rslt = vkQueuePresentKHR(this->vk_presentation_queue.queue, &present_info);
    (void)(rslt); // debug
}

const uint32_t& VulkanContext::getCurrentFrameIndex() const {
    return this->current_frame_index;
}

void VulkanContext::queueNextFrame() {
    this->current_frame_index = (this->current_frame_index + 1) % MAX_QUEUED_FRAME;
}

void VulkanContext::VKReload_swapChain() {
    // should only be called in context when none of these is in used.
    cleanup_swapChain();

    VKInit_initSwapChain();
    VKInit_initSwapChainRenderTargets();
}

void VulkanContext::cleanup_swapChain() {
    for(auto &render_target : this->vk_swapchain_render_targets) {
        vkDestroyFramebuffer(this->vk_device, render_target.framebuffer, nullptr);
        vkDestroyImageView(this->vk_device, render_target.image_view, nullptr);
    }
    vkDestroySwapchainKHR(this->vk_device, this->vk_swapchain, nullptr);
}

void VulkanContext::cleanup() {
    // cleanup should be called in context when the device is idling.
    vkDestroyRenderPass(this->vk_device, *this->vk_swapchain_render_pass, nullptr);
    cleanup_swapChain();

    vkDestroySurfaceKHR(this->vk_instance, this->vk_surface, nullptr);
    vkDestroyDevice(this->vk_device, nullptr);
    vkDestroyInstance(this->vk_instance, nullptr);
}