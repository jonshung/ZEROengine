#ifndef ZEROENGINE_VULKAN_DEFINE_H
#define ZEROENGINE_VULKAN_DEFINE_H

#include <vulkan/vulkan.hpp>
#include <glm/glm.hpp>

#include <vector>

struct VkQueueInfo {
    VkQueue queue;
    uint32_t queueFamilyIndex;
};

typedef VkCommandBuffer VulkanSecondaryCommandBuffer;

struct BaseVertex {
    glm::vec2 v_pos;
    glm::vec3 v_color;
};

class VulkanVertexInputBinding {
public:
    virtual VkVertexInputBindingDescription bindingDescription(const uint32_t &binding_index) = 0;
    virtual std::vector<VkVertexInputAttributeDescription> attributesDescription(const uint32_t &binding_index) = 0;
};

class VulkanBaseVertexInputBinding : public VulkanVertexInputBinding {
public:
    virtual VkVertexInputBindingDescription bindingDescription(const uint32_t &binding_index) override {
        VkVertexInputBindingDescription info {};
        info.binding = binding_index;
        info.stride = sizeof(BaseVertex);
        info.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
        return info;
    }
    
    virtual std::vector<VkVertexInputAttributeDescription> attributesDescription(const uint32_t &binding_index) override {
        std::vector<VkVertexInputAttributeDescription> attributes_desc(2);
        attributes_desc[0].binding = binding_index;
        attributes_desc[0].format = VK_FORMAT_R32G32_SFLOAT;
        attributes_desc[0].location = 0;
        attributes_desc[0].offset = 0;
        //
        attributes_desc[1].binding = binding_index;
        attributes_desc[1].format = VK_FORMAT_R32G32B32_SFLOAT;
        attributes_desc[1].location = 1;
        attributes_desc[1].offset = offsetof(BaseVertex, v_color);
        return attributes_desc;
    }
};

struct BaseUniformObject {
    glm::mat4 model;
    glm::mat4 view;
    glm::mat4 projection;
};

class VulkanUniformBufferLayout {
public:
    virtual VkDescriptorSetLayoutBinding bindingDescription(const uint32_t &binding_index) = 0;
};

class VulkanBaseUniformBufferLayout : public VulkanUniformBufferLayout {
public:
    virtual VkDescriptorSetLayoutBinding bindingDescription(const uint32_t &binding_index) override {
        VkDescriptorSetLayoutBinding ubo_layout_binding{};
        ubo_layout_binding.binding = binding_index;
        ubo_layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        ubo_layout_binding.descriptorCount = 1;
        ubo_layout_binding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        return ubo_layout_binding;
    }
};

#endif // #ifndef ZEROENGINE_VULKAN_DEFINE_H