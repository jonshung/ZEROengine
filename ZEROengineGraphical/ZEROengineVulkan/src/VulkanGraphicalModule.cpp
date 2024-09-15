#include "zeroengine_vulkan/VulkanGraphicalModule.hpp"
#include "zeroengine_vulkan/Vulkan_define.hpp"
#include "zeroengine_vulkan/VulkanWindowContext.hpp"

#include <cstring>

#include <vulkan/vk_enum_string_helper.h>

static std::vector<BaseVertex> rect = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
};

static std::vector<uint32_t> indices = {
    0, 1, 2, 2, 3, 0
};

void VulkanGraphicalModule::initGraphicalModule() {
    VkResult rslt;
    uint32_t w, h;

    // initializing window and surface
    VulkanWindowContext* vk_window_ctx = dynamic_cast<VulkanWindowContext*>(this->window_context.get());
    if(!vk_window_ctx) {
        throw std::runtime_error("VulkanGraphicalModule::initGraphicalModule() failed, error: window_context null or conversion to a VulkanWindowContext* failed");
    }
    vk_window_ctx->initWindow(640, 480);
    vk_window_ctx->getFramebufferSize(w, h);
    vk_window_ctx->getSurface(this->vulkan_context.getInstance(), &this->vk_surface);

    // initializing VulkanContext
    this->vulkan_context.initVulkan(this->vk_surface);

    // initializing ScreenRenderer and resources
    this->vulkan_screen_renderer.bindSurface(this->vk_surface);
    this->vulkan_screen_renderer.setFrameDimensions(w, h);

    VmaAllocatorCreateInfo vma_create{};
    vma_create.instance = this->vulkan_context.getInstance();
    vma_create.physicalDevice = this->vulkan_context.getPhysicalDevice();
    vma_create.device = this->vulkan_context.getDevice();
    vma_create.vulkanApiVersion = VK_API_VERSION_1_3;
    vma_create.flags = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT;
    if((rslt = vmaCreateAllocator(&vma_create, &this->vma_alloc)) != VK_SUCCESS) {
        throw std::runtime_error("vmaCreateAllocator() failed, error: cannot create VmaAllocator");
    }

    // Screen Renderer
    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;
    VulkanRendererSettings renderer_settings{false};
    screen_renderer.initVulkanRenderer(renderer_settings, this->vulkan_context, this->getVulkanContext().getGraphicalQueue().queueFamilyIndex);
    
    // Rendering contexts
    VulkanRenderContext &render_context = this->vulkan_render_context;
    VulkanRenderContextCreateInfo params{};
    params.device = this->vulkan_context.getDevice();
    params.queue_info = this->vulkan_context.getGraphicalQueue();
    params.primary_buffer_count = screen_renderer.getMaxQueuedFrame();
    render_context.initVulkanRenderContext(params);

    // Graphics Pipeline buffer
    this->vk_graphics_pipeline_buffer = std::make_unique<VulkanGraphicsPipelineBuffer>();

    // Vertex Buffer
    VkDeviceSize vertices_size = sizeof(rect[0]) * rect.size();
    this->vertices_buffer.allocate(
        this->vma_alloc,    // VmaAllocator
        vertices_size,      // VkDeviceSize
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, // VkBufferUsageFlagBits
        0); // VmaAllocationCreateFlagBits
    screen_renderer.setVerticesData(&rect);
    screen_renderer.setVerticesBufferHandle({ this->vertices_buffer.vk_buffer } );

    // Index Buffer
    VkDeviceSize indices_size = sizeof(indices[0]) * indices.size();
    this->index_buffer.allocate(
        this->vma_alloc,    // VmaAllocator
        indices_size,      // VkDeviceSize
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, // VkBufferUsageFlagBits
        0); // VmaAllocationCreateFlagBits
    screen_renderer.setIndexData(&indices);
    screen_renderer.setIndexBufferHandle(this->index_buffer.vk_buffer);

    // ForwardPass RenderPass
    VulkanBaseVertexInputBinding base_vertex_binding {};
    this->forwardpass_pipeline_template.vertex_binding = { base_vertex_binding.bindingDescription(0) };
    this->forwardpass_pipeline_template.vertex_attribute = base_vertex_binding.attributesDescription(0);

    // Vertex and Index Staging buffer
    this->recordAndSubmitStagingCommandBuffer();
}

void VulkanGraphicalModule::drawFrame() {
    if(this->window_context->isMinimized()) { // drawing on 0-sized framebuffer is dangerous, thus we wait until window is opened again. Also to save computation cost.
        return;
    }
    const uint32_t &current_frame_index = this->vulkan_screen_renderer.getCurrentFrameIndex();
    VulkanBasicScreenRenderer &screen_renderer = this->vulkan_screen_renderer;
    VulkanRenderContext &render_context = this->vulkan_render_context;

    this->vulkan_context.waitForFence(screen_renderer.getPresentationLock(current_frame_index));
    VkSemaphore image_lock = screen_renderer.getImageLock(current_frame_index);
    VkResult rslt = screen_renderer.queryAcquireAndStoreFrame(image_lock); // trying to acquire and internally store a render target in the renderer.
    if(rslt == VK_ERROR_OUT_OF_DATE_KHR) {
        handleResize();
        return;
    }

    else if(rslt != VK_SUCCESS && rslt != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("ZEROengine::drawFrame() error, err: " + std::to_string(rslt));
    }
    this->vulkan_context.releaseFence(screen_renderer.getPresentationLock(current_frame_index)); // releasing the lock

    // BEGIN rendering pipeline
    // recording to the secondary command buffer first
    std::vector<std::pair<VulkanRenderTarget*, VulkanSecondaryCommandBuffer>> 
    draw_cmd_buffers = screen_renderer.record(this->vk_graphics_pipeline_buffer.get());

    // now record to the render context and submit to hardware
    render_context.reset(current_frame_index);
    render_context.begin(current_frame_index);
    for(auto &[ pRenderTarget, cmd_buffer ] : draw_cmd_buffers) {
        if(!pRenderTarget) continue;
        render_context.recordRenderPassCommandBuffer(current_frame_index, cmd_buffer, *pRenderTarget);
    }
    render_context.end(current_frame_index);
    render_context.submit(
        current_frame_index, 
        screen_renderer.getImageLock(current_frame_index), 
        screen_renderer.getRenderingLock(current_frame_index),
        screen_renderer.getPresentationLock(current_frame_index)
    );

    screen_renderer.presentImage(screen_renderer.getRenderingLock(current_frame_index));
    screen_renderer.queueNextFrame();
    // END rendering pipeline
}

void VulkanGraphicalModule::recordAndSubmitStagingCommandBuffer() {
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
    cmd_pool_create_info.queueFamilyIndex = this->vulkan_context.getGraphicalQueue().queueFamilyIndex;
    if((rslt = vkCreateCommandPool(this->vulkan_context.getDevice(), &cmd_pool_create_info, nullptr, &staging_cmd_pool)) != VK_SUCCESS) {
        throw std::runtime_error("vkCreateCommandPool() failed, err: " + std::string(string_VkResult(rslt)));
    }

    // 3. Create command buffer for recording
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
    allocInfo.commandPool = staging_cmd_pool;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer staging_cmd_buffer;
    vkAllocateCommandBuffers(this->vulkan_context.getDevice(), &allocInfo, &staging_cmd_buffer);

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
    const uint32_t &current_frame_index = this->vulkan_screen_renderer.getCurrentFrameIndex();
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
    vkDestroyCommandPool(this->vulkan_context.getDevice(), staging_cmd_pool, nullptr);
    vertices_staging_buffer.cleanup(this->vma_alloc);
    indices_staging_buffer.cleanup(this->vma_alloc);
}

void VulkanGraphicalModule::handleResize() {
    uint32_t w, h;
    this->getWindowContext()->getFramebufferSize(w, h);
    this->getScreenRenderer().handleResize(w, h);
}

void VulkanGraphicalModule::cleanup() {
    vkDeviceWaitIdle(this->vulkan_context.getDevice());

    this->vertices_buffer.cleanup(this->vma_alloc);
    this->index_buffer.cleanup(this->vma_alloc);
    vmaDestroyAllocator(this->vma_alloc);
    
    this->forwardpass_pipeline_template.cleanup(this->getVulkanContext().getDevice());
    (*this->vk_graphics_pipeline_buffer).cleanup(this->vulkan_context.getDevice());

    this->vulkan_screen_renderer.cleanup();

    this->vulkan_render_context.cleanup();
    
    vkDestroySurfaceKHR(this->vulkan_context.getInstance(), this->vk_surface, nullptr);
    
    this->vulkan_context.cleanup();
    this->window_context->cleanup();
}