#include "zeroengine_vulkan/VulkanGraphicsPipelineTemplate.hpp"

#include "zeroengine_vulkan/VulkanDefines.hpp"

namespace ZEROengine {
    VulkanGraphicsPipelineTemplate::VulkanGraphicsPipelineTemplate() :
    pipeline_layout{},
    descriptor_layout{},
    descriptor_layout_bindings{},
    vertex_binding{},
    vertex_attribute{}
    {
        rasterization_state = {};
        rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        rasterization_state.depthClampEnable = false;
        rasterization_state.rasterizerDiscardEnable = false;
        rasterization_state.polygonMode = VK_POLYGON_MODE_FILL;
        rasterization_state.lineWidth = 1.0f;
        rasterization_state.cullMode = VK_CULL_MODE_BACK_BIT;
        rasterization_state.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        rasterization_state.depthBiasEnable = VK_FALSE;
        rasterization_state.depthBiasConstantFactor = 0.0f; // Optional
        rasterization_state.depthBiasClamp = 0.0f; // Optional
        rasterization_state.depthBiasSlopeFactor = 0.0f; // Optional

        multisample_state = {};
        multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        multisample_state.sampleShadingEnable = VK_FALSE;
        multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        multisample_state.minSampleShading = 1.0f; // Optional
        multisample_state.pSampleMask = nullptr; // Optional
        multisample_state.alphaToCoverageEnable = VK_FALSE; // Optional
        multisample_state.alphaToOneEnable = VK_FALSE; // Optional
    }

    VkShaderModule VulkanGraphicsPipelineTemplate::createShaderModule(const VkDevice &device, const char* data, const std::size_t &data_size) const {
        VkShaderModuleCreateInfo shader_create_info{};
        shader_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        shader_create_info.codeSize = data_size;
        shader_create_info.pCode = reinterpret_cast<const uint32_t*>(data);
        VkShaderModule shader_module;
        ZERO_VK_CHECK_EXCEPT(vkCreateShaderModule(device, &shader_create_info, nullptr, &shader_module));
        return shader_module;
    }

    void VulkanGraphicsPipelineTemplate::createGraphicsPipelinesLayout(const VkDevice &device) {
        VkDescriptorSetLayoutCreateInfo descriptor_set_create_info{};
        descriptor_set_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
        descriptor_set_create_info.bindingCount = this->descriptor_layout_bindings.size();
        descriptor_set_create_info.pBindings = this->descriptor_layout_bindings.data();
        ZERO_VK_CHECK_EXCEPT(vkCreateDescriptorSetLayout(device, &descriptor_set_create_info, nullptr, &this->descriptor_layout));
        
        VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
        pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipeline_layout_create_info.pSetLayouts = &this->descriptor_layout;
        pipeline_layout_create_info.setLayoutCount = 1;
        pipeline_layout_create_info.pushConstantRangeCount = 0;
        pipeline_layout_create_info.pPushConstantRanges = nullptr;
        ZERO_VK_CHECK_EXCEPT(vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &this->pipeline_layout));
    }

    std::vector<VkPipeline>
    VulkanGraphicsPipelineTemplate::produce(
        const VkDevice &device, 
        const VkRenderPass &render_pass,
        const std::vector<ShaderData> &shaders) const
    {
        // BEGIN fixed-function create infos
        VkPipelineVertexInputStateCreateInfo vertex_input_info{};
        vertex_input_info.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertex_input_info.vertexBindingDescriptionCount = this->vertex_binding.size();
        vertex_input_info.pVertexBindingDescriptions = this->vertex_binding.data(); // Optional
        vertex_input_info.vertexAttributeDescriptionCount = this->vertex_attribute.size();
        vertex_input_info.pVertexAttributeDescriptions = this->vertex_attribute.data(); // Optional

        // TODO: support for input assembly customization
        VkPipelineInputAssemblyStateCreateInfo input_assembly{};
        input_assembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        input_assembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        input_assembly.primitiveRestartEnable = VK_FALSE;

        // TODO: support for viewport and scissor state customization
        VkPipelineViewportStateCreateInfo viewport_state{};
        viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        viewport_state.viewportCount = 1;
        viewport_state.scissorCount = 1;
        
        // since we support resizing, dynamic state should be included
        VkPipelineDynamicStateCreateInfo dynamic_state{};
        dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_states.size());
        dynamic_state.pDynamicStates = dynamic_states.data();

        VkPipelineRasterizationStateCreateInfo rasterizer = this->rasterization_state;
        VkPipelineMultisampleStateCreateInfo multisampling = this->multisample_state;

        // TODO: support for color blending customization
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
        graphics_pipeline_create_info.layout = pipeline_layout;
        graphics_pipeline_create_info.renderPass = render_pass;
        graphics_pipeline_create_info.subpass = 0;
        graphics_pipeline_create_info.basePipelineHandle = nullptr;
        graphics_pipeline_create_info.basePipelineIndex = -1;

        // BEGIN programmable-function stages
        std::vector<VkPipeline> ret(shaders.size());
        std::vector<VkGraphicsPipelineCreateInfo> jobs(shaders.size());
        std::vector<std::pair<VkShaderModule, VkShaderModule>> shader_modules(shaders.size());
        std::size_t i = 0;
        for(const ShaderData &data : shaders) {
            shader_modules[i].first = createShaderModule(device, data.vertex_data.first, data.vertex_data.second);
            shader_modules[i].second = createShaderModule(device, data.fragment_data.first, data.fragment_data.second);

            VkPipelineShaderStageCreateInfo vert_shader_stage_create_info{};
            vert_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            vert_shader_stage_create_info.module = shader_modules[i].first;
            vert_shader_stage_create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
            vert_shader_stage_create_info.pName = "main";

            VkPipelineShaderStageCreateInfo frag_shader_stage_create_info{};
            frag_shader_stage_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
            frag_shader_stage_create_info.module = shader_modules[i].second;
            frag_shader_stage_create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
            frag_shader_stage_create_info.pName = "main";
            
            VkPipelineShaderStageCreateInfo shader_program_jobs[] = {vert_shader_stage_create_info, frag_shader_stage_create_info};
            graphics_pipeline_create_info.stageCount = 2;
            graphics_pipeline_create_info.pStages = shader_program_jobs;
            jobs[i] = graphics_pipeline_create_info;
            ++i;
        }
        // END programmable-function stages
        ZERO_VK_CHECK_EXCEPT(vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, jobs.size(), jobs.data(), nullptr, ret.data()));
        for(auto &[vert_shader, frag_shader] : shader_modules) {
            vkDestroyShaderModule(device, frag_shader, nullptr);
            vkDestroyShaderModule(device, vert_shader, nullptr);
        }
        return ret;
    }

} // namespace ZEROengine