#include "zeroengine_vulkan/VulkanContext.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include <map>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <unordered_set>

namespace ZEROengine {
    VulkanContext::VulkanContext() :
    vk_instance{},
    vk_physical_device{},
    vk_device{},
    vk_graphics_queue{},
    vk_presentation_queue{}
    {
        VKInit_initInstance();
    }

    ZEROResult VulkanContext::initVulkan(const VkSurfaceKHR &surface) {
        if(dbg_enable_validation_layers && !checkValidationLayersSupport()) {
            return { ZERO_VULKAN_VALIDATION_NOT_EXISTS, "Validation layer is not supported but required." };
        }
        // preserve order
        ZERO_CHECK_RESULT_RETURN(VKInit_initPhysicalDevice(surface));
        ZERO_CHECK_RESULT_RETURN(VKInit_initLogicalDevice(surface));
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanContext::VKInit_initInstance() {
        VkApplicationInfo app_info{};
        app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        app_info.pApplicationName = "ZEROengine";
        app_info.applicationVersion = VK_MAKE_VERSION(0, 0, 1);
        app_info.pEngineName = "ZEROengine";
        app_info.engineVersion = VK_MAKE_VERSION(0, 0, 1);
        app_info.apiVersion = VK_API_VERSION_1_3;

        VkInstanceCreateInfo create_info{};
        create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        create_info.pApplicationInfo = &app_info;

        std::string val_ret;
        // Checking required instance extensions
        constexpr const char* required_extensions[] = {
            VulkanContext::getRequiredExtension(),
            "VK_KHR_surface"
        };
        constexpr uint32_t required_extensions_count = std::extent<decltype(required_extensions)>::value;
        if(!validateExtensionsSupport(required_extensions_count, required_extensions, &val_ret)) {
            ZERO_CHECK_RESULT_RETURN( ZEROResult({ ZERO_VULKAN_VALIDATION_INSUFFICIENT, val_ret }) );
        }

        create_info.enabledExtensionCount = required_extensions_count;
        create_info.ppEnabledExtensionNames = required_extensions;
        create_info.enabledLayerCount = 0;

        if (dbg_enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(dbg_validation_layers.size());
            create_info.ppEnabledLayerNames = dbg_validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
        }
        VkResult rslt{};
        if((rslt = vkCreateInstance(&create_info, nullptr, &this->vk_instance)) != VK_SUCCESS) {
            ZERO_CHECK_RESULT_RETURN( ZEROResult({ ZERO_VULKAN_CREATE_INSTANCE_FAILED, string_VkResult(rslt) }) );
        }
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanContext::VKInit_initPhysicalDevice(const VkSurfaceKHR &surface) {
        if(this->vk_instance == VK_NULL_HANDLE) {
            return { ZERO_VULKAN_NULL_HANDLE, "Handle to Vulkan instance is null, maybe it was not initialized?" };
        }
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(this->vk_instance, &device_count, nullptr);
        if(device_count == 0) {
            return { ZERO_VULKAN_NO_PHYSICAL_DEVICE, "No physical device found." };
        }
        std::vector<VkPhysicalDevice> phys_devices(device_count);
        vkEnumeratePhysicalDevices(this->vk_instance, &device_count, phys_devices.data());

        // choose the best device
        ZEROResult rslt = selectPhysicalDevice(surface, phys_devices);
        ZERO_CHECK_RESULT_RETURN(rslt);
        return { ZERO_SUCCESS, "" };;
    }

    ZEROResult VulkanContext::VKInit_initLogicalDevice(const VkSurfaceKHR &surface) {
        if(this->vk_instance == VK_NULL_HANDLE) {
            return { ZERO_VULKAN_NULL_HANDLE, "Handle to Vulkan instance is null, maybe it was not initialized?" };
        }
        if(this->vk_physical_device == VK_NULL_HANDLE) {
            return { ZERO_VULKAN_NULL_HANDLE, "Handle to Vulkan physical device is null, maybe it was not initialized?" };
        }
        QueueFamilyIndices queue_families = queryQueueFamily(this->vk_physical_device, {VK_QUEUE_GRAPHICS_BIT});
        std::unordered_set<uint32_t> queue_indices;
        for(auto &[family, index] : queue_families.queue_indices) {
            if(!index.has_value()) {
                return { ZERO_VULKAN_QUEUE_NOT_EXISTS, "A queue is not found for family: " + std::to_string(family) };
            }
            queue_indices.insert(index.value());
        }
        this->vk_graphics_queue.queueFamilyIndex = queue_families.queue_indices[VK_QUEUE_GRAPHICS_BIT].value(); // should exists since we already check suitability.
        std::optional<uint32_t> presentation_queue = queryPresentationQueueFamily_Surface(surface, this->vk_physical_device); // again, should exists
        if(!presentation_queue.has_value()) {
            return { ZERO_VULKAN_QUEUE_NOT_EXISTS, "Presentation queue is not exists, this should not be happening!" };
        }
        this->vk_presentation_queue.queueFamilyIndex = presentation_queue.value();
        queue_indices.insert(static_cast<uint32_t>(presentation_queue.value()));

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos{};
        float queuePriority = 1.0f;
        for (uint32_t queue_family : queue_indices) {
            VkDeviceQueueCreateInfo queue_create_info{};
            queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queue_create_info.queueFamilyIndex = queue_family;
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
        return { ZERO_SUCCESS, "" };
    }

    bool VulkanContext::checkValidationLayersSupport(std::string *ret) {
        uint32_t layerCount = 0;
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
        uint32_t vk_extension_count = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &vk_extension_count, nullptr);
        std::vector<VkExtensionProperties> vk_extensions(vk_extension_count);
        vkEnumerateInstanceExtensionProperties(nullptr, &vk_extension_count, vk_extensions.data());

        std::unordered_set<std::string> validation_intersection{};
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

    ZEROResult VulkanContext::selectPhysicalDevice(const VkSurfaceKHR &surface, const std::vector<VkPhysicalDevice> &phys_devices) {
        std::multimap<uint32_t, const VkPhysicalDevice&, std::greater<uint32_t>> device_scores{};

        for(const VkPhysicalDevice &dev : phys_devices) {
            device_scores.insert({evaluatePhysicalDeviceSuitability(surface, dev), dev});
        }
        auto it = device_scores.begin();
        if(it->first == 0) return { ZERO_VULKAN_NO_SUITABLE_PHYS_DEVICE, "No suitable physical device found." };
        this->vk_physical_device = it->second;
        return { ZERO_SUCCESS, "" };
    }

    uint32_t VulkanContext::evaluatePhysicalDeviceSuitability(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device) {
        uint32_t score = 0;
        VkPhysicalDeviceProperties device_properties{};
        VkPhysicalDeviceFeatures device_features{};
        vkGetPhysicalDeviceProperties(phys_device, &device_properties);
        vkGetPhysicalDeviceFeatures(phys_device, &device_features);

        if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 1000;
        else score += 10;
        score += device_properties.limits.maxImageDimension2D;

        // preliminary checks
        if(!device_features.geometryShader) return 0; // since we need geometry shader
        QueueFamilyIndices queue_indices = queryQueueFamily(phys_device, {VK_QUEUE_GRAPHICS_BIT});
        std::optional<uint32_t> presentation_queue = queryPresentationQueueFamily_Surface(surface, phys_device);

        if(!queue_indices.exists() || !presentation_queue.has_value()) return 0;
        if(!checkDeviceExtensionSupport(phys_device)) return 0;
        
        bool swapChainAdequate = false;
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport_Surface(surface, phys_device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.present_modes.empty();
        if(!swapChainAdequate) return 0;
        
        return score;
    }

    bool VulkanContext::checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device) {
        uint32_t extension_count = 0;
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
        
        QueueFamilyIndices q_index{};
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

    // TODO: add support for native presentation queue querying with respect to each platform's current active primary display
    std::optional<uint32_t> VulkanContext::queryPresentationQueueFamily_Surface(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device) {
        uint32_t queue_family_count = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &queue_family_count, nullptr);
        for(uint32_t i = 0; i < queue_family_count; ++i) {
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, i, surface, &presentSupport);
            if(presentSupport) return std::optional(static_cast<int32_t>(i));
            i++;
        }
        return {};
    }

    // todo: add support for native queue querying outside of surfaces.
    VulkanContext::SwapChainSupportDetails VulkanContext::querySwapChainSupport(const VkPhysicalDevice &phys_device) {
        SwapChainSupportDetails details{};
        uint32_t format_count = 0, present_mode_count = 0;
        (void) phys_device;
        (void) format_count;
        (void) present_mode_count;
        #if defined(VK_USE_PLATFORM_ANDROID_KHR)
            // not supported yet
        #elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
            // not supported yet
        #elif defined(VK_USE_PLATFORM_XLIB_KHR)
            // not supported yet
        #endif
        return details;
    }

    VulkanContext::SwapChainSupportDetails VulkanContext::querySwapChainSupport_Surface(const VkSurfaceKHR &surface, const VkPhysicalDevice &phys_device) {
        if(surface == VK_NULL_HANDLE) {
            ZERO_EXCEPTION(ZERO_VULKAN_NULL_HANDLE, "Surface handle is null.");
        }
        if(phys_device == VK_NULL_HANDLE) {
            ZERO_EXCEPTION(ZERO_VULKAN_NULL_HANDLE, "Physical device handle is null.");
        }
        SwapChainSupportDetails details{};
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(phys_device, surface, &details.capabilities);

        uint32_t format_count = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &format_count, nullptr);
        if(format_count > 0) {
            details.formats.resize(format_count);
            vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, surface, &format_count, details.formats.data());
        }

        uint32_t present_mode_count = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, surface, &present_mode_count, nullptr);
        if(present_mode_count > 0) {
            details.present_modes.resize(present_mode_count);
            vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, surface, &present_mode_count, details.present_modes.data());
        }
        return details;
    }

    void VulkanContext::waitForFence(VkFence fence) {
        vkWaitForFences(this->vk_device, 1, &fence, VK_TRUE, UINT64_MAX);
    }

    void VulkanContext::releaseFence(VkFence fence) {
        vkResetFences(this->vk_device, 1, &fence);
    }


    ZEROResult VulkanContext::allocateBuffer() {
        // TODO: Implement
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanContext::allocateTexture() {
        // TODO: Implement
        return { ZERO_SUCCESS, "" };
    }

    void VulkanContext::cleanup() {
        // cleanup should be called in context when the device is idling.
        vkDestroyDevice(this->vk_device, nullptr);
        vkDestroyInstance(this->vk_instance, nullptr);
    }
} // namespace ZEROengine