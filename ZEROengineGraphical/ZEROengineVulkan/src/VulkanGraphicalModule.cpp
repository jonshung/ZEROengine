#include "zeroengine_vulkan/VulkanGraphicalModule.hpp"
#include "zeroengine_vulkan/VulkanDefines.hpp"

#include <cstring>
#include <chrono>
#include <unordered_map>

#include <vulkan/vk_enum_string_helper.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace ZEROengine {
    static std::vector<BaseVertex> rect = {
        {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
        {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
        {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
    };

    static std::vector<uint32_t> indices = {
        0, 1, 2, 2, 3, 0
    };

    static std::vector<BaseUniformObject> rect_uniform = { {} };

    VulkanGraphicalModule::VulkanGraphicalModule() : 
    m_vulkan_device{},
    m_render_window{}
    {}

    void VulkanGraphicalModule::initGraphicalModule() {
        m_is_off = false;
        
        // initializing window and surface
        std::shared_ptr<VulkanDevice> vulkan_device = getVulkanDevice().lock();
        vulkan_device->initVulkan();
    }

    void VulkanGraphicalModule::drawFrame() {
        m_render_window->pollEvent();
        if(m_render_window->isClosing()) {
            m_is_off = true;
            return;
        }
        if(m_render_window->isMinimized() || !m_render_window->isActive() || m_render_window->isHidden()) {
            return; // no need to draw on inactive or minimized windows.
        }
    }

    std::weak_ptr<VulkanDevice> VulkanGraphicalModule::getVulkanDevice() {
        return m_vulkan_device;
    }
        
    std::weak_ptr<VulkanWindow> VulkanGraphicalModule::getRenderWindow() {
        return m_render_window;
    }

    void VulkanGraphicalModule::cleanup() {
        VkDevice device = m_vulkan_device->getDevice();
        vkDeviceWaitIdle(device);

        // always last
        m_vulkan_device->cleanup();
        m_is_off = true;
    }
} // namespace ZEROengine
