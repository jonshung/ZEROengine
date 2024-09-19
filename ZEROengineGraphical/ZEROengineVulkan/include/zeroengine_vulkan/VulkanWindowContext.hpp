#ifndef ZEROENGINE_VULKAN_WINDOW_CONTEXT_H
#define ZEROENGINE_VULKAN_WINDOW_CONTEXT_H


#include <vulkan/vulkan.hpp>

#include "zeroengine_core/WindowContext.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"

namespace ZEROengine {
    /**
     * @brief Engine system which uses Vulkan rendering system should expand this class in order to use the default VulkanGraphicalModule
     * 
     */
    class VulkanWindowContext : public WindowContext {
    protected:
        VkSurfaceKHR *surface_handler = nullptr;

    public:
        virtual ZEROResult getSurface(const VkInstance& vk_instance, VkSurfaceKHR* vk_surface) = 0;
    };
} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_WINDOW_CONTEXT_H