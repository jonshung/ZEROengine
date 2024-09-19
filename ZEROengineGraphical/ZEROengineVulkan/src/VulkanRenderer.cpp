#include "zeroengine_vulkan/VulkanRenderer.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include <array>

namespace ZEROengine {
    VulkanRenderer::VulkanRenderer() :
    vk_render_cmd_pool{},
    vulkan_context{nullptr}
    {}
    
    ZEROResult VulkanRenderer::initVulkanRenderer(VulkanContext *vulkan_context) {
        this->vulkan_context = vulkan_context;

        VkDevice device{};
        this->vulkan_context->getDevice(device);
        VkQueueInfo graphical_queue{};
        this->vulkan_context->getGraphicalQueue(graphical_queue);

        // Command pools
        VkCommandPoolCreateInfo cmd_pool_create_info{};
        cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmd_pool_create_info.queueFamilyIndex = graphical_queue.queueFamilyIndex;
        VkResult rslt = vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &this->vk_render_cmd_pool);
        if(rslt != VK_SUCCESS) {
            return { ZERO_VULKAN_CREATE_POOL_FAILED, std::string(string_VkResult(rslt)) };
        }
        return { ZERO_SUCCESS, "" };
    }
} // namespace ZEROengine
