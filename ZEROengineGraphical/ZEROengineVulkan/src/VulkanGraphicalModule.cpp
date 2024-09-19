#include "zeroengine_vulkan/VulkanGraphicalModule.hpp"
#include "zeroengine_vulkan/Vulkan_define.hpp"
#include "zeroengine_vulkan/VulkanWindowContext.hpp"

#include <cstring>
#include <chrono>

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

    void VulkanGraphicalModule::initGraphicalModule() {
        VkResult rslt;
        uint32_t w, h;

        // initializing window and surface
        WindowContext *window_ctx;
        this->getWindowContext(&window_ctx);
        VulkanWindowContext* vk_window_ctx = dynamic_cast<VulkanWindowContext*>(window_ctx);
        VulkanContext *vulkan_context;
        VulkanBasicScreenRenderer *screen_renderer;
        this->getVulkanContext(&vulkan_context);
        this->getScreenRenderer(&screen_renderer);
        if(!vk_window_ctx) {
            throw std::runtime_error("VulkanGraphicalModule::initGraphicalModule() failed, error: window_context null or conversion to a VulkanWindowContext* failed");
        }
        vk_window_ctx->initWindow(640, 480);
        vk_window_ctx->getFramebufferSize(w, h);

        VkInstance vk_instance;
        vulkan_context->getInstance(vk_instance);
        VkSurfaceKHR surface;
        vk_window_ctx->getSurface(vk_instance, &surface);

        // initializing VulkanContext
        vulkan_context->initVulkan(surface);

        // initializing ScreenRenderer and resources
        VulkanRenderWindow *render_window;
        screen_renderer->getRenderWindow(&render_window);
        render_window->initVulkanRenderWindow(vulkan_context, surface);
        render_window->setDimensions(w, h);

        VkPhysicalDevice vk_physical;
        VkDevice vk_device;
        vulkan_context->getPhysicalDevice(vk_physical);
        vulkan_context->getDevice(vk_device);
        VmaAllocatorCreateInfo vma_create{};
        vma_create.instance = vk_instance;
        vma_create.physicalDevice = vk_physical;
        vma_create.device = vk_device;
        vma_create.vulkanApiVersion = VK_API_VERSION_1_3;
        vma_create.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
        if((rslt = vmaCreateAllocator(&vma_create, &this->vma_alloc)) != VK_SUCCESS) {
            throw std::runtime_error("vmaCreateAllocator() failed, error: cannot create VmaAllocator");
        }

        // Screen Renderer
        screen_renderer->initVulkanRenderer(vulkan_context);
        
        // Rendering contexts
        VkQueueInfo graphical_queue, presentation_queue;
        this->vulkan_context.getGraphicalQueue(graphical_queue);
        this->vulkan_context.getPresentationQueue(presentation_queue);
        VulkanRenderContext &render_context = this->vulkan_render_context;
        VulkanRenderContextCreateInfo params{};
        params.device = vk_device;
        params.graphical_queue_info = graphical_queue;
        params.presentation_queue_info = presentation_queue;
        params.primary_buffer_count = screen_renderer->getMaxQueuedFrame();
        render_context.initVulkanRenderContext(params);

        // Vertex Buffer
        VkDeviceSize vertices_size = sizeof(rect[0]) * rect.size();
        this->vertices_buffer.allocate(
            this->vma_alloc,    // VmaAllocator
            vertices_size,      // VkDeviceSize
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // VkBufferUsageFlagBits
            0); // VmaAllocationCreateFlagBits
        screen_renderer->setVerticesData(&rect);
        screen_renderer->setVerticesBufferHandle({ this->vertices_buffer.vk_buffer } );

        // Index Buffer
        VkDeviceSize indices_size = sizeof(indices[0]) * indices.size();
        this->index_buffer.allocate(
            this->vma_alloc,    // VmaAllocator
            indices_size,      // VkDeviceSize
            VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // VkBufferUsageFlagBits
            0); // VmaAllocationCreateFlagBits
        screen_renderer->setIndexData(&indices);
        screen_renderer->setIndexBufferHandle(this->index_buffer.vk_buffer);



        // TODO: Move per frame data to BasicScreenRenderer, including buffers


        
        // Uniform Buffer
        VkDeviceSize uniform_size = sizeof(BaseUniformObject);
        this->uniform_buffers.resize(screen_renderer->getMaxQueuedFrame());
        for(auto &buffer : this->uniform_buffers) {
            buffer.allocate(
                this->vma_alloc,
                uniform_size,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT
            );
        }

        // Descriptor Pool
        VkDescriptorPoolSize descriptor_pool_size{};
        descriptor_pool_size.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor_pool_size.descriptorCount = static_cast<uint32_t>(screen_renderer->getMaxQueuedFrame());
        VkDescriptorPoolCreateInfo descriptor_pool_create_info{};
        descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        descriptor_pool_create_info.pPoolSizes = &descriptor_pool_size;
        descriptor_pool_create_info.poolSizeCount = 1;
        descriptor_pool_create_info.maxSets = static_cast<uint32_t>(screen_renderer->getMaxQueuedFrame());
        if((rslt = vkCreateDescriptorPool(vk_device, &descriptor_pool_create_info, nullptr, &this->vk_descriptor_pool)) != VK_SUCCESS) {
            throw std::runtime_error("vkCreateDescriptorPool() failed, error: " + std::string(string_VkResult(rslt)));
        }

        // ForwardPass RenderPass
        VulkanBaseVertexInputBinding base_vertex_binding {};
        VulkanBaseUniformBufferLayout base_descriptor_layout {};
        this->forwardpass_pipeline_template.descriptor_layout_bindings.push_back(base_descriptor_layout.bindingDescription(0));
        this->forwardpass_pipeline_template.vertex_binding = { base_vertex_binding.bindingDescription(0) };
        this->forwardpass_pipeline_template.vertex_attribute = base_vertex_binding.attributesDescription(0);
        this->forwardpass_pipeline_template.createGraphicsPipelinesLayout(vk_device);

        // Descriptor set
        VkDescriptorSetAllocateInfo descriptor_set_allocate_info{};
        descriptor_set_allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        descriptor_set_allocate_info.descriptorSetCount = screen_renderer->getMaxQueuedFrame();
        descriptor_set_allocate_info.descriptorPool = this->vk_descriptor_pool;
        std::vector<VkDescriptorSetLayout> desc_set_layouts(screen_renderer->getMaxQueuedFrame(), this->forwardpass_pipeline_template.descriptor_layout);
        descriptor_set_allocate_info.pSetLayouts = desc_set_layouts.data();
        this->vk_descriptor_sets.resize(screen_renderer->getMaxQueuedFrame());
        if((rslt = vkAllocateDescriptorSets(vk_device, &descriptor_set_allocate_info, this->vk_descriptor_sets.data())) != VK_SUCCESS) {
            throw std::runtime_error("vkCreateDescriptorPool() failed, error: " + std::string(string_VkResult(rslt)));
        }
        for(std::size_t i = 0; i < screen_renderer->getMaxQueuedFrame(); ++i) {
            VkDescriptorBufferInfo desc_buf_info{};
            desc_buf_info.buffer = this->uniform_buffers[i].vk_buffer;
            desc_buf_info.offset = 0;
            desc_buf_info.range = VK_WHOLE_SIZE;
            VkWriteDescriptorSet desc_write{};
            desc_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            desc_write.dstSet = this->vk_descriptor_sets[i];
            desc_write.dstBinding = 0;
            desc_write.dstArrayElement = 0;
            desc_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            desc_write.descriptorCount = 1;
            desc_write.pBufferInfo = &desc_buf_info;
            vkUpdateDescriptorSets(vk_device, 1, &desc_write, 0, nullptr);
        }

        // Vertex and Index Staging buffer
        this->recordAndSubmitStagingCommandBuffer();
    }

    void VulkanGraphicalModule::drawFrame() {
        WindowContext *window_ctx;
        this->getWindowContext(&window_ctx);
        if(window_ctx->isMinimized()) { // drawing on 0-sized framebuffer is dangerous, thus we wait until window is opened again. Also to save computation cost.
            return;
        }
        VulkanBasicScreenRenderer *screen_renderer;
        this->getScreenRenderer(&screen_renderer);
        const uint32_t &current_frame_index = screen_renderer->getCurrentFrameIndex();
        VulkanRenderWindow *render_window;
        screen_renderer->getRenderWindow(&render_window);
        VulkanRenderContext &render_context = this->vulkan_render_context;

        this->vulkan_context.waitForFence(screen_renderer->getPresentationLock(current_frame_index));
        if(window_ctx->resized()) {
            handleResize();
            return;
        }
        this->vulkan_context.releaseFence(screen_renderer->getPresentationLock(current_frame_index)); // releasing the lock

        // BEGIN rendering pipeline
        // 1. updating uniform buffer

        static auto startTime = std::chrono::high_resolution_clock::now();

        auto currentTime = std::chrono::high_resolution_clock::now();
        float time = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

        rect_uniform[0].model = glm::rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        rect_uniform[0].view = glm::lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
        uint32_t swapchain_w, swapchain_h;
        render_window->getDimensions(swapchain_w, swapchain_h);
        rect_uniform[0].projection = glm::perspective(glm::radians(45.0f), swapchain_w / (float) swapchain_h, 0.1f, 10.0f); 
                                    // accounting for if framebuffer of surface is bigger than 
                                    // swapchain and has to be clamped
        rect_uniform[0].projection[1][1] *= -1;
        vmaCopyMemoryToAllocation(this->vma_alloc, &rect_uniform[0], this->uniform_buffers[current_frame_index].vma_mem_alloc, 0, sizeof(rect_uniform[0]));

        screen_renderer->setUniformBufferDescriptorSets({ this->vk_descriptor_sets[current_frame_index]} );
        
        // 2. recording to the secondary command buffer
        std::vector<VulkanGraphicsRecordingInfo> recordings;
        screen_renderer->record(&this->vk_graphics_pipeline_buffer, recordings);

        // 3. now record to the render context and submit to hardware
        render_context.reset(current_frame_index);
        render_context.begin(current_frame_index);
        for(auto &recording : recordings) {
            render_context.recordRenderPassCommandBuffer(current_frame_index, recording);
        }
        render_context.end(current_frame_index);
        render_context.submit(
            current_frame_index, 
            screen_renderer->getImageLock(current_frame_index), 
            screen_renderer->getRenderingLock(current_frame_index),
            screen_renderer->getPresentationLock(current_frame_index)
        );

        render_context.present(screen_renderer->getPresentImageInfo());
        screen_renderer->queueNextFrame();
        // END rendering pipeline
    }

    void VulkanGraphicalModule::recordAndSubmitStagingCommandBuffer() {
        VulkanContext *vulkan_context;
        VulkanBasicScreenRenderer *screen_renderer;
        VkDevice device;
        VkQueueInfo queue_info;
        this->getVulkanContext(&vulkan_context);
        this->getScreenRenderer(&screen_renderer);
        vulkan_context->getDevice(device);
        vulkan_context->getGraphicalQueue(queue_info);

        // 1. Create Staging buffers for vertices and indices
        VulkanBuffer vertices_staging_buffer, indices_staging_buffer;
        vertices_staging_buffer.allocate(
            this->vma_alloc,    // VmaAllocator
            sizeof(rect[0])*rect.size(),      // VkDeviceSize
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // VkBufferUsageFlagBits
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT); // VmaAllocationCreateFlagBits
        vmaCopyMemoryToAllocation(this->vma_alloc, reinterpret_cast<void*>(rect.data()), vertices_staging_buffer.vma_mem_alloc, 0, vertices_staging_buffer.request_size);
        indices_staging_buffer.allocate(
            this->vma_alloc,    // VmaAllocator
            sizeof(indices[0])*indices.size(),      // VkDeviceSize
            VK_BUFFER_USAGE_TRANSFER_SRC_BIT, // VkBufferUsageFlagBits
            VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT | VMA_ALLOCATION_CREATE_MAPPED_BIT); // VmaAllocationCreateFlagBits
        vmaCopyMemoryToAllocation(this->vma_alloc, reinterpret_cast<void*>(indices.data()), indices_staging_buffer.vma_mem_alloc, 0, indices_staging_buffer.request_size);

        // 2. Create a command pool
        VkResult rslt;
        VkCommandPoolCreateInfo cmd_pool_create_info{};
        VkCommandPool staging_cmd_pool;

        cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        cmd_pool_create_info.queueFamilyIndex = queue_info.queueFamilyIndex;
        if((rslt = vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &staging_cmd_pool)) != VK_SUCCESS) {
            throw std::runtime_error("vkCreateCommandPool() failed, err: " + std::string(string_VkResult(rslt)));
        }

        // 3. Create command buffer for recording
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
        allocInfo.commandPool = staging_cmd_pool;
        allocInfo.commandBufferCount = 1;
        VkCommandBuffer staging_cmd_buffer;
        vkAllocateCommandBuffers(device, &allocInfo, &staging_cmd_buffer);

        // 4. Start recording
        VkCommandBufferBeginInfo begin_info{};
        VkCommandBufferInheritanceInfo inherit_info{};
        inherit_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;

        begin_info.pInheritanceInfo = &inherit_info;
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
        vkBeginCommandBuffer(staging_cmd_buffer, &begin_info);
        VkBufferCopy copy_region{};
        copy_region.srcOffset = 0; // Optional
        copy_region.dstOffset = 0; // Optional
        copy_region.size = vertices_staging_buffer.request_size;
        vkCmdCopyBuffer(staging_cmd_buffer, vertices_staging_buffer.vk_buffer, this->vertices_buffer.vk_buffer, 1, &copy_region);
        copy_region.size = indices_staging_buffer.request_size;
        vkCmdCopyBuffer(staging_cmd_buffer, indices_staging_buffer.vk_buffer, this->index_buffer.vk_buffer, 1, &copy_region);
        vkEndCommandBuffer(staging_cmd_buffer);

        // 5. Submit and wait, then release all used resources
        const uint32_t &current_frame_index = screen_renderer->getCurrentFrameIndex();
        VulkanRenderContext &render_context = this->vulkan_render_context;
        render_context.reset(current_frame_index);
        render_context.begin(current_frame_index);
        render_context.recordTransferCommandBuffer(current_frame_index, { staging_cmd_buffer });
        render_context.end(current_frame_index);
        render_context.submit(
            current_frame_index, 
            VK_NULL_HANDLE, // no need to wait for anything exclusively
            VK_NULL_HANDLE,
            VK_NULL_HANDLE
        );
        render_context.waitOnQueue();
        vkDestroyCommandPool(device, staging_cmd_pool, nullptr);
        vertices_staging_buffer.cleanup(this->vma_alloc);
        indices_staging_buffer.cleanup(this->vma_alloc);
    }

    void VulkanGraphicalModule::handleResize() {
        uint32_t w, h;
        VulkanRenderWindow *render_window;
        VulkanBasicScreenRenderer *screen_renderer;
        WindowContext *window_ctx;
        this->getWindowContext(&window_ctx);
        this->getScreenRenderer(&screen_renderer);
        window_ctx->getFramebufferSize(w, h);
        screen_renderer->getRenderWindow(&render_window);
        render_window->handleResize(w, h);
    }

    void VulkanGraphicalModule::cleanup() {
        VkDevice device;
        WindowContext *window_context;
        VulkanContext *vulkan_context;
        VulkanBasicScreenRenderer *screen_renderer;
        VulkanRenderContext *render_context;
        this->getWindowContext(&window_context);
        this->getVulkanContext(&vulkan_context);
        this->getScreenRenderer(&screen_renderer);
        this->getRenderContext(&render_context);
        this->vulkan_context.getDevice(device);
        vkDeviceWaitIdle(device);

        this->vertices_buffer.cleanup(this->vma_alloc);
        this->index_buffer.cleanup(this->vma_alloc);
        for(auto &buffer : this->uniform_buffers) {
            buffer.cleanup(this->vma_alloc);
        }
        vmaDestroyAllocator(this->vma_alloc);

        vkDestroyDescriptorPool(device, this->vk_descriptor_pool, nullptr);
        
        this->forwardpass_pipeline_template.cleanup(device);
        this->vk_graphics_pipeline_buffer.cleanup(device);

        screen_renderer->cleanup();
        render_context->cleanup();
        this->vulkan_context.cleanup();
        this->window_context->cleanup();
    }
} // namespace ZEROengine
