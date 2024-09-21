#include "zeroengine_vulkan/VulkanRenderer.hpp"

#include "zeroengine_vulkan/VulkanDefines.hpp"

#include <array>

namespace ZEROengine {
    VulkanRenderer::VulkanRenderer() :
    vk_render_cmd_pool{},
    frame_cmd_buffers{}
    {}
    
    void VulkanRenderer::initVulkanRenderer(VulkanContext *vulkan_context) {
        this->vulkan_context = vulkan_context;

        VkDevice device = this->vulkan_context->getDevice();
        VkQueueInfo *graphical_queue = this->vulkan_context->getGraphicalQueue();

        // Command pools
        VkCommandPoolCreateInfo cmd_pool_create_info{};
        cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmd_pool_create_info.queueFamilyIndex = graphical_queue->queueFamilyIndex;
        ZERO_VK_CHECK_EXCEPT(vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &this->vk_render_cmd_pool));
    }
} // namespace ZEROengine
