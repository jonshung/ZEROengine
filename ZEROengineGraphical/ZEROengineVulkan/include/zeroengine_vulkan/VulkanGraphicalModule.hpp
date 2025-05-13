#ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H
#define ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H

#include <memory>

#include "zeroengine_graphical/GraphicalModule.hpp"
#include "zeroengine_vulkan/VulkanDevice.hpp"
#include "zeroengine_vulkan/VulkanHardwareBuffer.hpp"
#include "zeroengine_vulkan/VulkanWindow.hpp"

namespace ZEROengine {
    /**
     * @brief The engine graphical module. Providing necessary contexts and modules to draw to render targets, including the application window.
     * The module manages 
     * - the Vulkan Device
     * 
     */
    class VulkanGraphicalModule : public GraphicalModule {
    private:
        std::shared_ptr<VulkanDevice> m_vulkan_device;

    // Main rendering window
    private:
        std::shared_ptr<VulkanWindow> m_render_window;

    private:
        void recordAndSubmitStagingCommandBuffer();

    public:
        VulkanGraphicalModule();
        std::weak_ptr<VulkanDevice> getVulkanDevice();
        std::weak_ptr<VulkanWindow> getRenderWindow();
        
        void initGraphicalModule() override;
        void drawFrame() override;
        
        void cleanup() override;
    }; // class VulkanGraphicalModule

} // namespace ZEROengine

#endif // #ifndef ZEROENGINE_VULKAN_GRAPHICAL_MODULE_H