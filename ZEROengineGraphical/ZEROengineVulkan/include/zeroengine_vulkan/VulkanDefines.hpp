#ifndef ZEROENGINE_VULKAN_DEFINE_H
#define ZEROENGINE_VULKAN_DEFINE_H

#include <vulkan/vulkan.hpp>
#include <vulkan/vk_enum_string_helper.h>
#include <glm/glm.hpp>

#include "zeroengine_core/ZEROengineDefines.hpp"

#include <vector>
#include <string>

namespace ZEROengine {
    struct VkQueueInfo {
        VkQueue queue;
        uint32_t queueFamilyIndex;
    };

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

} // namespace ZEROengine

#define ZERO_VK_CHECK_EXCEPT(func_call) do { \
    VkResult __rslt = func_call; \
    if(__rslt != VK_SUCCESS) { \
        std::string __call_except = std::string(#func_call); \
        std::size_t __call_func_end_pos = __call_except.find('('); \
        __call_except = __call_except.substr(0, __call_func_end_pos); \
        ZERO_EXCEPT(ZEROResultEnum::ZERO_GRAPHICAL_ERROR, __call_except + " failed with " + std::string(string_VkResult(__rslt))); \
    } \
} while(0)
#endif // #ifndef ZEROENGINE_VULKAN_DEFINE_H