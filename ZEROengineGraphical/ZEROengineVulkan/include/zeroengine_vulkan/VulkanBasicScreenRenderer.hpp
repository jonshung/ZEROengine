#ifndef ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H
#define ZEROENGINE_VULKAN_BASIC_SCREEN_RENDERER_H

#include <vulkan/vulkan.hpp>

#include <vector>
#include <memory>

#include "zeroengine_vulkan/VulkanContext.hpp"
#include "zeroengine_vulkan/VulkanRenderWindow.hpp"
#include "zeroengine_vulkan/VulkanRenderer.hpp"
#include "zeroengine_vulkan/VulkanPipelineBuffer.hpp"
#include "zeroengine_vulkan/Vulkan_define.hpp"
#include "zeroengine_vulkan/VulkanCommand.hpp"
#include "zeroengine_core/ZEROengineDefines.hpp"

namespace ZEROengine {
    /**
     * @brief The basic Renderer class for camera-view into the rendering scene.
     * 
     */
    class VulkanBasicScreenRenderer : public VulkanRenderer {
    // Command buffers
    private:
        std::vector<VulkanSecondaryCommandBuffer> frame_cmd_buffers;
        const uint32_t max_queued_frame = 2;

        void allocateSecondaryCommandBuffer(const uint32_t &count);

    // RenderTargets
    private:
        VulkanRenderWindow render_window;

    // public swapchain related API
    public:
        const uint32_t& getCurrentFrameIndex() const;
        void queueNextFrame();
        uint32_t getMaxQueuedFrame() const {
            return this->max_queued_frame;
        }
        VkPresentInfoKHR getPresentImageInfo();

        uint32_t current_frame_index = 0;
        uint32_t acquired_swapchain_index;

    // Frame data
    private:
        std::vector<BaseVertex> *vertices_data;
        std::vector<VkBuffer> vertices_buffer_handles;

        std::vector<uint32_t> *index_data;
        VkBuffer index_buffer_handle;

        std::vector<VkDescriptorSet> uniform_buffer_desc_set;

    public:
        void setVerticesBufferHandle(const std::vector<VkBuffer> &buffer_handles) {
            this->vertices_buffer_handles = buffer_handles;
        }
        void setVerticesData(std::vector<BaseVertex>* const &p_vertices_data) {
            this->vertices_data = p_vertices_data;
        }
        void setIndexBufferHandle(const VkBuffer &buffer_handle) {
            this->index_buffer_handle = buffer_handle;
        }
        void setIndexData(std::vector<uint32_t>* const &p_index_data) {
            this->index_data = p_index_data;
        }

        void setUniformBufferDescriptorSets(const std::vector<VkDescriptorSet> &desc_sets) {
            this->uniform_buffer_desc_set = desc_sets;
        }
    
    // sync objects
    private:
        std::vector<VkSemaphore> vk_image_mutex; // mutexes use exclusively to swapchain at current moment
        std::vector<VkSemaphore> vk_rendering_mutex; // hardware rendering completion signal mutex
        std::vector<VkFence> vk_presentation_mutex; // host rendering completion signal mutex

    public:
        // allocate additional concurrency lock for synchronization
        void createSyncObjects(const uint32_t &count);
        VkResult tryAcquireSwapchainImage();

        VkFence getPresentationLock(const uint32_t &target_index) {
            return this->vk_presentation_mutex[target_index];
        }
        VkSemaphore getImageLock(const uint32_t &target_index) {
            return this->vk_image_mutex[target_index];
        }
        VkSemaphore getRenderingLock(const uint32_t &target_index) {
            return this->vk_rendering_mutex[target_index];
        }

        void cleanup_syncObjects();

    // Initialization and cleanup procedures
    public:
        VulkanBasicScreenRenderer();
        virtual ZEROResult initVulkanRenderer(VulkanContext *vulkan_context) override;
        ZEROResult getRenderWindow(VulkanRenderWindow** pRenderWindow) {
            *pRenderWindow = &this->render_window;
            return { ZERO_SUCCESS, "" };
        }
        
        virtual void cleanup() override;

    public:
        virtual void reset() override;
        virtual void begin() override;
        virtual void configureViewportAndScissor(VkExtent2D &extent) override;
        // currently directly passing the graphics pipeline buffer to every renderer via here. Should be passing a structure of
        // world objects material and geometry informations in the future
        virtual void draw(VulkanGraphicsPipelineBuffer *const g_pipeline_buffer) override;
        virtual void end() override;
        virtual ZEROResult record(VulkanGraphicsPipelineBuffer *const pipeline_buffer, std::vector<VulkanGraphicsRecordingInfo> &ret) override;
    }; // class VulkanBasicScreenRenderer
} // namespace ZEROengine

#endif // #ifndef VULKAN_BASIC_SCREEN_RENDERER_H