#include "VulkanRenderer.hpp"
#include "vulkan/vk_enum_string_helper.h"

void VulkanRenderer::initVulkanRenderer(const VulkanRendererCreateInfo *info) {
    this->reloadDependencies(info->dependencies);
    createCommandPool(info->device, info->queue_family_index);
    createCommandBuffer(info->device, info->max_queue_frame);
    createConcurrencyLock(info->device, info->max_queue_frame); // locks for each frame
}

void VulkanRenderer::reloadDependencies(VulkanRendererDependencies dependencies) {
    this->vk_framebuffers = dependencies.framebuffers;
    this->vk_render_pass = *dependencies.render_pass;
    this->vk_graphics_queue = *dependencies.graphics_queue;
    this->vk_extent = *dependencies.extent;
}

void VulkanRenderer::createCommandPool(VkDevice device, uint32_t queueFamilyIndex) {
    VkCommandPoolCreateInfo cmd_pool_create_info{};
    cmd_pool_create_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    cmd_pool_create_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    cmd_pool_create_info.queueFamilyIndex = queueFamilyIndex;
    VkResult rslt = vkCreateCommandPool(device, &cmd_pool_create_info, nullptr, &this->vk_cmd_pool);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateCommandPool() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::createCommandBuffer(VkDevice device, uint32_t count) {
    this->vk_cmd_buffers.resize(this->vk_cmd_buffers.size() + count);
    VkCommandBufferAllocateInfo alloc_info{};
    alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    alloc_info.commandPool = this->vk_cmd_pool;
    alloc_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    alloc_info.commandBufferCount = count;
    VkResult rslt = vkAllocateCommandBuffers(device, &alloc_info, this->vk_cmd_buffers.data());
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkAllocateCommandBuffers() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::recordRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index, uint32_t image_index) {
    if(this->vk_graphics_pipelines.size() <= 0) return;

    VkCommandBufferBeginInfo cmd_buffer_begin{};
    cmd_buffer_begin.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    cmd_buffer_begin.pInheritanceInfo = nullptr;
    cmd_buffer_begin.flags = 0;
    VkCommandBuffer &cmd_buffer = this->vk_cmd_buffers[frame_cmd_buffer_index];
    VkResult rslt = vkBeginCommandBuffer(cmd_buffer, &cmd_buffer_begin);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkBeginCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }

    VkRenderPassBeginInfo render_pass_begin{};
    render_pass_begin.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    render_pass_begin.framebuffer = (*this->vk_framebuffers)[image_index];
    render_pass_begin.renderPass = this->vk_render_pass;
    render_pass_begin.renderArea.extent = this->vk_extent;
    VkClearValue clear_color = {{{0.0f, 0.0f, 0.0f, 1.0f}}};
    render_pass_begin.clearValueCount = 1;
    render_pass_begin.pClearValues = &clear_color;
    vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, this->vk_graphics_pipelines[0]);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(this->vk_extent.width);
    viewport.height = static_cast<float>(this->vk_extent.height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = this->vk_extent;
    vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);

    vkCmdDraw(cmd_buffer, 3, 1, 0 ,0);
    vkCmdEndRenderPass(cmd_buffer);
    if ((rslt = vkEndCommandBuffer(cmd_buffer)) != VK_SUCCESS) {
        throw std::runtime_error("vkEndCommandBuffer() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::submitRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index) {
    VkSubmitInfo submit_info{};
    submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submit_info.commandBufferCount = 1;
    submit_info.pCommandBuffers = &this->vk_cmd_buffers[frame_cmd_buffer_index];
    VkPipelineStageFlags wait_stages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submit_info.waitSemaphoreCount = 1;
    submit_info.pWaitSemaphores = &this->vk_image_mutex[frame_cmd_buffer_index];
    submit_info.pWaitDstStageMask = wait_stages;
    submit_info.signalSemaphoreCount = 1;
    submit_info.pSignalSemaphores = &this->vk_rendering_mutex[frame_cmd_buffer_index];
    VkResult rslt = vkQueueSubmit(this->vk_graphics_queue, 1, &submit_info, this->vk_presentation_mutex[frame_cmd_buffer_index]);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("submitRenderThreadCommandBuffer()::vkQueueSubmit() failed, err: " + std::string(string_VkResult(rslt)));
    }
}

void VulkanRenderer::resetRenderCommandBuffer(const uint32_t &frame_cmd_buffer_index) {
    vkResetCommandBuffer(this->vk_cmd_buffers[frame_cmd_buffer_index], 0);
}

void VulkanRenderer::createConcurrencyLock(VkDevice device, uint32_t count) {
    this->vk_image_mutex.resize(this->vk_image_mutex.size() + count);
    this->vk_rendering_mutex.resize(this->vk_rendering_mutex.size() + count);
    this->vk_presentation_mutex.resize(this->vk_presentation_mutex.size() + count);

    VkSemaphoreCreateInfo semaphore_create{};
    semaphore_create.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    VkResult rslt;
    for(uint32_t i = 0; i < count; ++i) {
        rslt = vkCreateSemaphore(device, &semaphore_create, nullptr, &this->vk_image_mutex[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateSemaphore() failed, err: " + std::string(string_VkResult(rslt)));
        }
        rslt = vkCreateSemaphore(device, &semaphore_create, nullptr, &this->vk_rendering_mutex[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateSemaphore() failed, err: " + std::string(string_VkResult(rslt)));
        }
        VkFenceCreateInfo fence_create{};
        fence_create.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fence_create.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        rslt = vkCreateFence(device, &fence_create, nullptr, &this->vk_presentation_mutex[i]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateFence() failed, err: " + std::string(string_VkResult(rslt)));
        }
    }
}

void VulkanRenderer::setVerticesBuffer() {
    // TODO
}

VkFence VulkanRenderer::getPresentationLock(uint32_t frame_index) {
    return this->vk_presentation_mutex[frame_index];
}

VkSemaphore VulkanRenderer::getImageLock(uint32_t frame_index) {
    return this->vk_image_mutex[frame_index];
}

VkSemaphore VulkanRenderer::getRenderingLock(uint32_t frame_index) {
    return this->vk_rendering_mutex[frame_index];
}

VkShaderModule VulkanRenderer::createShaderModule(VkDevice device, const char* data, const size_t &data_size) {
    VkShaderModuleCreateInfo shader_create_info{};
    shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shader_create_info.codeSize = data_size;
    shader_create_info.pCode = reinterpret_cast<const uint32_t*>(data);
    VkShaderModule shader_module;
    VkResult rslt = vkCreateShaderModule(device, &shader_create_info, nullptr, &shader_module);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreateShaderModule() failed, err: " + std::string(string_VkResult(rslt)));
    }
    return shader_module;
}

void VulkanRenderer::createGraphicsPipelines(VkDevice device, std::vector<ShaderData> shaders) {
    // BEGIN Pipeline layout
    VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
    pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout_create_info.pSetLayouts = nullptr;
    pipeline_layout_create_info.setLayoutCount = 0;
    pipeline_layout_create_info.pushConstantRangeCount = 0;
    pipeline_layout_create_info.pPushConstantRanges = nullptr;
    VkResult rslt = vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &this->vk_pipeline_layout);
    if(rslt != VK_SUCCESS) {
        throw std::runtime_error("vkCreatePipelineLayout() failed, err:" + std::string(string_VkResult(rslt)));
    }

    // BEGIN fixed-function create infos
    VkPipelineVertexInputStateCreateInfo vertex_input_info{};
    vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertex_input_info.vertexBindingDescriptionCount = 0;
    vertex_input_info.pVertexBindingDescriptions = nullptr; // Optional
    vertex_input_info.vertexAttributeDescriptionCount = 0;
    vertex_input_info.pVertexAttributeDescriptions = nullptr; // Optional

    VkPipelineInputAssemblyStateCreateInfo input_assembly{};
    input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    input_assembly.primitiveRestartEnable = VK_FALSE;

    VkPipelineViewportStateCreateInfo viewport_state{};
    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = 1;
    viewport_state.scissorCount = 1;
    
    VkPipelineDynamicStateCreateInfo dynamic_state{};
    dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
    dynamic_state.pDynamicStates = dynamic_states.data();

    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState color_blend_attachment{};
    color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    color_blend_attachment.blendEnable = VK_FALSE;
    color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD; // Optional
    color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE; // Optional
    color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO; // Optional
    color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD; // Optional

    /*
    VkPipelineDepthStencilStateCreateInfo depth_stencil{};
    depth_stencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depth_stencil.stencilTestEnable = VK_FALSE;
    depth_stencil.depthTestEnable = VK_FALSE;
    */
    

    VkPipelineColorBlendStateCreateInfo color_blending{};
    color_blending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    color_blending.logicOpEnable = VK_FALSE;
    color_blending.logicOp = VK_LOGIC_OP_COPY; // Optional
    color_blending.attachmentCount = 1;
    color_blending.pAttachments = &color_blend_attachment;
    color_blending.blendConstants[0] = 0.0f; // Optional
    color_blending.blendConstants[1] = 0.0f; // Optional
    color_blending.blendConstants[2] = 0.0f; // Optional
    color_blending.blendConstants[3] = 0.0f; // Optional
    // END fixed-function create infos

    VkGraphicsPipelineCreateInfo graphics_pipeline_create_info{};
    graphics_pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;

    // fixed-function stages
    graphics_pipeline_create_info.pVertexInputState = &vertex_input_info;
    graphics_pipeline_create_info.pInputAssemblyState = &input_assembly;
    graphics_pipeline_create_info.pViewportState = &viewport_state;
    graphics_pipeline_create_info.pRasterizationState = &rasterizer;
    graphics_pipeline_create_info.pMultisampleState = &multisampling;
    graphics_pipeline_create_info.pDepthStencilState = nullptr;//&depth_stencil; // Optional
    graphics_pipeline_create_info.pColorBlendState = &color_blending;
    graphics_pipeline_create_info.pDynamicState = &dynamic_state;
    graphics_pipeline_create_info.layout = this->vk_pipeline_layout;
    graphics_pipeline_create_info.renderPass = this->vk_render_pass;
    graphics_pipeline_create_info.subpass = 0;
    graphics_pipeline_create_info.basePipelineHandle = nullptr;
    graphics_pipeline_create_info.basePipelineIndex = -1;

    size_t index = this->vk_graphics_pipelines.size();
    this->vk_graphics_pipelines.resize(this->vk_graphics_pipelines.size() + shaders.size());
    for(ShaderData data : shaders) {
        // BEGIN programmable-function stages
        VkShaderModule frag_shader = createShaderModule(device, std::get<0>(data.fragment_data), std::get<1>(data.fragment_data));
        VkShaderModule vert_shader = createShaderModule(device, std::get<0>(data.vertex_data), std::get<1>(data.vertex_data));

        VkPipelineShaderStageCreateInfo vert_shader_stage_create_info{};
        vert_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        vert_shader_stage_create_info.module = vert_shader;
        vert_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        vert_shader_stage_create_info.pName = "main";

        VkPipelineShaderStageCreateInfo frag_shader_stage_create_info{};
        frag_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        frag_shader_stage_create_info.module = frag_shader;
        frag_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        frag_shader_stage_create_info.pName = "main";
        
        VkPipelineShaderStageCreateInfo create_jobs[] = {vert_shader_stage_create_info, frag_shader_stage_create_info};
        // programmable shader stages
        graphics_pipeline_create_info.stageCount = 2;
        graphics_pipeline_create_info.pStages = create_jobs;
        // END programmable-function stages

        rslt = vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &graphics_pipeline_create_info, nullptr, &this->vk_graphics_pipelines[index]);
        if(rslt != VK_SUCCESS) {
            throw std::runtime_error("vkCreateGraphicsPipelines() failed, err: " + std::string(string_VkResult(rslt)));
        }
        vkDestroyShaderModule(device, frag_shader, nullptr);
        vkDestroyShaderModule(device, vert_shader, nullptr);
        ++index;
    }
}

void VulkanRenderer::cleanup_concurrency_locks(VkDevice device) {
    // concurrency locks count should be synchronized
    uint32_t buffer_count = this->vk_image_mutex.size();
    for(uint32_t i = 0; i < buffer_count; ++i) {
        vkDestroyFence(device, this->vk_presentation_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_rendering_mutex[i], nullptr);
        vkDestroySemaphore(device, this->vk_image_mutex[i], nullptr);
    }
}

void VulkanRenderer::cleanup_commandBuffers(VkDevice device) {
    vkDestroyCommandPool(device, this->vk_cmd_pool, nullptr);
}

void VulkanRenderer::cleanup_pipelines(VkDevice device) {
    for(auto &pipeline : this->vk_graphics_pipelines) {
        vkDestroyPipeline(device, pipeline, nullptr);
    }
    vkDestroyPipelineLayout(device, this->vk_pipeline_layout, nullptr);
    vkDestroyRenderPass(device, this->vk_render_pass, nullptr);
}

void VulkanRenderer::cleanup(VkDevice device) {
    cleanup_concurrency_locks(device);
    cleanup_commandBuffers(device);
    cleanup_pipelines(device);
}