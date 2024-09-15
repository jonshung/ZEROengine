#include "zeroengine_vulkan/VulkanRenderer.hpp"
#include <vulkan/vk_enum_string_helper.h>

#include <array>

void VulkanRenderer::initVulkanRenderer(const VulkanRendererSettings &settings, VulkanContext &vulkan_context, const uint32_t &queue_family_index) {
    this->reloadSettings(settings);
    this->vulkan_context = &vulkan_context;
    // Command pools
    VkCommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_create_info.queueFamilyIndex = queue_family_index;
    VkResult rslt = vkCreateCommandPool(this->vulkan_context->getDevice(), &cmd_pool_create_info, nullptr, &this->vk_render_cmd_pool);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateCommandPool() failed, err: " + std::string(string_VkResult(rslt)));
    }
}