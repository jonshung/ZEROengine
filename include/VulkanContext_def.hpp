#ifndef VULKAN_CONTEXT_DEF_H
#define VULKAN_CONTEXT_DEF_H

// debug validation layers
const std::vector<const char*> dbg_validation_layers = {
    "VK_LAYER_KHRONOS_validation"
};

struct VkQueueInfo {
    VkQueue queue;
    uint32_t queueFamilyIndex;
};
#endif // #ifndef VULKAN_CONTEXT_DEF_H