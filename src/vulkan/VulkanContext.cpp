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
    // preserve order
    VKInit_initInstance();
    VKInit_initWindowSurface();
    VKInit_initPhysicalDevice();
    VKInit_initLogicalDevice();
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

void VulkanContext::waitForFence(VkFence fence) {
    vkWaitForFences(this->vk_device, 1, &fence, VK_TRUE, UINT64_MAX);
}

void VulkanContext::releaseFence(VkFence fence) {
    vkResetFences(this->vk_device, 1, &fence);
}

void VulkanContext::cleanup() {
    // cleanup should be called in context when the device is idling.
    vkDestroySurfaceKHR(this->vk_instance, this->vk_surface, nullptr);
    vkDestroyDevice(this->vk_device, nullptr);
    vkDestroyInstance(this->vk_instance, nullptr);
}