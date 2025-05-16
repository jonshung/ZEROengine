#include <map>
#include <vector>
#include <cstdint>
#include <limits>
#include <algorithm>
#include <unordered_set>
#include <memory>

#include "zeroengine_vulkan/VulkanDevice.hpp"
#include "zeroengine_vulkan/VulkanContext.hpp"
#include <vulkan/vk_enum_string_helper.h>
#include "vk_mem_alloc.h"

namespace ZEROengine {
    // required device extensions
    const std::vector<const char*> const_device_extensions = {
        VK_KHR_SWAPCHAIN_EXTENSION_NAME
    };

    // debug validation layers
    const std::vector<const char*> const_dbg_validation_layers = {
        "VK_LAYER_KHRONOS_validation"
    };

    constexpr const char* VulkanDevice::getRequiredExtension() {
#if defined(VK_USE_PLATFORM_XCB_KHR)
        return VK_KHR_XCB_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WAYLAND_KHR)
        return VK_KHR_WAYLAND_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
        return VK_KHR_WIN32_SURFACE_EXTENSION_NAME;
#elif defined(VK_USE_PLATFORM_ANDROID_KHR)
        return VK_KHR_ANDROID_SURFACE_EXTENSION_NAME;
#endif
        return "";
    }

    VulkanDevice::VulkanDevice() :
    m_vk_instance{},
    m_vk_physical_device{},
    m_vk_device{}
    {
        initInstance();
    }

    void VulkanDevice::initVulkan(VulkanWindow* vulkan_window) {
        if(const_dbg_enable_validation_layers && !checkValidationLayersSupport()) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Validation layer is not supported but required.");
        }
        // preserve order
        initPhysicalDevice();
        initLogicalDevice(vulkan_window);
        
        // VMA
        VmaAllocatorCreateInfo vma_create{};
        vma_create.instance = getInstance();
        vma_create.physicalDevice = getPhysicalDevice();
        vma_create.device = getDevice();
        vma_create.vulkanApiVersion = VK_API_VERSION_1_3;
        vma_create.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
        ZERO_VK_CHECK_EXCEPT(vmaCreateAllocator(&vma_create, &m_vma_alloc));
    }

    void VulkanDevice::initInstance() {
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
            VulkanDevice::getRequiredExtension(),
            "VK_KHR_surface"
        };
        constexpr uint32_t required_extensions_count = std::extent<decltype(required_extensions)>::value;
        if(!validateExtensionsSupport(required_extensions_count, required_extensions, &val_ret)) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Vulkan instance does not suport extension: " + val_ret);
        }

        create_info.enabledExtensionCount = required_extensions_count;
        create_info.ppEnabledExtensionNames = required_extensions;
        create_info.enabledLayerCount = 0;

        if (const_dbg_enable_validation_layers) {
            create_info.enabledLayerCount = static_cast<uint32_t>(const_dbg_validation_layers.size());
            create_info.ppEnabledLayerNames = const_dbg_validation_layers.data();
        } else {
            create_info.enabledLayerCount = 0;
        }
        ZERO_VK_CHECK_EXCEPT(vkCreateInstance(&create_info, nullptr, &m_vk_instance));
    }

    void VulkanDevice::initPhysicalDevice() {
        if(m_vk_instance == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_NULL_POINTER, "Vulkan instance handle is null.");
        }
        uint32_t device_count = 0;
        vkEnumeratePhysicalDevices(m_vk_instance, &device_count, nullptr);
        if(device_count == 0) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "No physical device found.");
        }
        std::vector<VkPhysicalDevice> phys_devices(device_count);
        vkEnumeratePhysicalDevices(m_vk_instance, &device_count, phys_devices.data());
        // choose the best device
        selectPhysicalDevice(phys_devices);
    }

    void VulkanDevice::initLogicalDevice(VulkanWindow* vulkan_window) {
        if(m_vk_instance == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_NULL_POINTER, "Vulkan instance handle is null.");
        }
        if(m_vk_physical_device == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_NULL_POINTER, "Vulkan physical device handle is null.");
        }

        VkPhysicalDeviceFeatures device_features{};

        std::vector<VkDeviceQueueCreateInfo> queue_create_infos = m_vulkan_queue_manager->queryQueueCreation(m_vk_physical_device);
        if(vulkan_window) {
            std::optional<VkDeviceQueueCreateInfo> presentation_queue_create_info = vulkan_window->queryPresentationQueueCreation();
            if(!presentation_queue_create_info.has_value()) {
                ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "VulkanWindow cannot find a presentation queue.");
            }
            queue_create_infos.push_back(std::move(presentation_queue_create_info.value()));
        }

        VkDeviceCreateInfo device_create_info{};
        device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO; 
        device_create_info.pQueueCreateInfos = queue_create_infos.data();
        device_create_info.queueCreateInfoCount = static_cast<uint32_t>(queue_create_infos.size());
        device_create_info.pEnabledFeatures = &device_features;
        device_create_info.enabledExtensionCount = static_cast<uint32_t>(const_device_extensions.size());
        device_create_info.ppEnabledExtensionNames = const_device_extensions.data();

        if(const_dbg_enable_validation_layers) {
            device_create_info.enabledLayerCount = static_cast<uint32_t>(const_dbg_validation_layers.size());
            device_create_info.ppEnabledLayerNames = const_dbg_validation_layers.data();
        } else {
            device_create_info.enabledLayerCount = 0;
        }
        ZERO_VK_CHECK_EXCEPT(vkCreateDevice(m_vk_physical_device, &device_create_info, nullptr, &m_vk_device));

        // initialize queue manager
        m_vulkan_queue_manager->init(m_vk_device);
    }

    bool VulkanDevice::checkValidationLayersSupport(std::string *ret) {
        uint32_t layerCount = 0;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        std::unordered_set<std::string> validation_intersection;
        for(VkLayerProperties &view : availableLayers) {
            validation_intersection.insert(view.layerName);
        }
        for(const char* layer_char : const_dbg_validation_layers) {
            std::string layer_name = std::string(layer_char);
            if(validation_intersection.find(layer_name) == validation_intersection.end()) {
                if(ret) *ret = layer_name;
                return false;
            }
        }
        return true;
    }

    bool VulkanDevice::validateExtensionsSupport(const uint32_t &extension_count, const char*const* extensions, std::string *ret) {
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

    void VulkanDevice::selectPhysicalDevice(const std::vector<VkPhysicalDevice> &phys_devices) {
        std::multimap<uint32_t, const VkPhysicalDevice&, std::greater<uint32_t>> device_scores{};

        for(const VkPhysicalDevice &dev : phys_devices) {
            device_scores.insert({evaluatePhysicalDeviceSuitability(dev), dev});
        }
        auto it = device_scores.begin();
        if(it->first == 0) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "No suitable physical device found.");
        }
        m_vk_physical_device = it->second;

        // debug
        VkPhysicalDeviceProperties device_properties{};
        vkGetPhysicalDeviceProperties(it->second, &device_properties);
        printf("Chosen %s\n", device_properties.deviceName);
    }

    uint32_t VulkanDevice::evaluatePhysicalDeviceSuitability(const VkPhysicalDevice &phys_device) {
        uint32_t score = 0;
        VkPhysicalDeviceProperties device_properties{};
        VkPhysicalDeviceFeatures device_features{};
        VkPhysicalDeviceMemoryProperties device_memory{};
        vkGetPhysicalDeviceFeatures(phys_device, &device_features);
        vkGetPhysicalDeviceProperties(phys_device, &device_properties);
        vkGetPhysicalDeviceMemoryProperties(phys_device, &device_memory);

        if(!device_features.geometryShader) return 0; // since we need geometry shader
        if(!checkDeviceExtensionSupport(phys_device)) return 0;

        if(device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) score += 100;
        else score += 10;
        score += device_properties.limits.maxImageDimension2D;

        VkMemoryHeap* heap_ptr = device_memory.memoryHeaps;
        const auto heaps = std::vector<VkMemoryHeap>(heap_ptr, heap_ptr + device_memory.memoryHeapCount);
        for(const auto& heap : heaps) {
            score += heap.size*10;
        }
        return score;
    }

    bool VulkanDevice::checkDeviceExtensionSupport(const VkPhysicalDevice &phys_device) {
        uint32_t extension_count = 0;
        vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count, nullptr);

        std::vector<VkExtensionProperties> availableExtensions(extension_count);
        vkEnumerateDeviceExtensionProperties(phys_device, nullptr, &extension_count, availableExtensions.data());

        std::unordered_set<std::string> required_extensions(const_device_extensions.begin(), const_device_extensions.end());

        for (const auto& extension : availableExtensions) {
            required_extensions.erase(extension.extensionName);
        }
        return required_extensions.empty();
    }

    std::weak_ptr<GraphicalContext> VulkanDevice::allocateGraphicalContext() {
        VulkanQueueInfo graphical_queue_info;
        if(!m_vulkan_queue_manager->getQueueInfo(VK_QUEUE_GRAPHICS_BIT, graphical_queue_info)) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, "Queue manager cannot find a graphical queue.");
        }
        std::shared_ptr<GraphicalContext> graphical_context = std::make_shared<VulkanGraphicalContext>(m_vk_device, graphical_queue_info.queueFamilyIndex);
        m_graphical_contexts.push_back(graphical_context);

        return graphical_context;
    }

    VkDevice VulkanDevice::getDevice() {
        if(m_vk_device == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_NULL_POINTER, "Vulkan logical device handle is null.");
        }
        return m_vk_device;
    }

    VkPhysicalDevice VulkanDevice::getPhysicalDevice() {
        if(m_vk_physical_device == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_NULL_POINTER, "Vulkan physical device handle is null.");
        }
        return m_vk_physical_device;
    }

    VkInstance VulkanDevice::getInstance() {
        if(m_vk_instance == VK_NULL_HANDLE) {
            ZERO_EXCEPT(ZEROResultEnum::ZERO_NULL_POINTER, "Vulkan instance handle is null.");
        }
        return m_vk_instance;
    }

    std::weak_ptr<VulkanQueueManager> VulkanDevice::getQueueManager() {
        return m_vulkan_queue_manager;
    }

    void VulkanDevice::releaseFence(VkFence fence) {
        vkResetFences(m_vk_device, 1, &fence);
    }

    void VulkanDevice::stall() {
        vkDeviceWaitIdle(m_vk_device);
    }

    ZEROResult VulkanDevice::allocateBuffer() {
        // TODO: Implement
        return { ZERO_SUCCESS, "" };
    }

    ZEROResult VulkanDevice::allocateTexture() {
        // TODO: Implement
        return { ZERO_SUCCESS, "" };
    }

    void VulkanDevice::cleanup() {
        // cleanup should be called in context when the device is idling.
        vmaDestroyAllocator(m_vma_alloc);

        vkDestroyDevice(m_vk_device, nullptr);
        vkDestroyInstance(m_vk_instance, nullptr);
    }
} // namespace ZEROengine