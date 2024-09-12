#ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H
#define ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H

#include "SDLWindowContext.hpp"
#include "VulkanContext.hpp"
#include "VulkanBasicScreenRenderer.hpp"

class VulkanGraphicalModule {
// Renderer should be here
private:
    std::shared_ptr<WindowContext> window_context;
    VulkanContext vulkan_context;

    std::vector<size_t> frame_command_buffer_indices;
    VulkanRenderContext vulkan_render_context;
    VulkanBasicScreenRenderer vulkan_screen_renderer;

public:
    VulkanContext& getVulkanContext() {
        return this->vulkan_context;
    }
    VulkanRenderContext& getRenderContext() {
        return this->vulkan_render_context;
    }
    VulkanBasicScreenRenderer& getScreenRenderer() {
        return this->vulkan_screen_renderer;
    }
    VulkanGraphicalModule() = delete;
    VulkanGraphicalModule(const std::shared_ptr<WindowContext> window_ctx);
    void drawFrame();
    void handleResize();
    void cleanup();
};

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H