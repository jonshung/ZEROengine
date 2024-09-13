#ifndef ZEROENGINE_WINDOW_CONTEXT_ABSTRACT_H
#define ZEROENGINE_WINDOW_CONTEXT_ABSTRACT_H

#include <cstdint>

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkInstance)

/**
 * @brief General purpose WindowContext to manage the application window.
 * 
 */
class WindowContext {
public:
    /**
     * @brief Initialization function. Any uses to this class of objects (including it's child class) 
     * MUST call this function first before any window operation.
     * 
     * @param width 
     * @param height 
     */
    virtual void initWindow(uint32_t width, uint32_t height) = 0;

    /**
     * @brief Requesting all extensions supported by this Window Context surface backend.
     * 
     * @param ext_count 
     * @return const char* const* 
     */
    virtual const char*const* requestInstanceExtensions(uint32_t &ext_count) const = 0;
    
    /**
     * @brief Get the Surface object. Currently only supports Vulkan
     * 
     * @param vk_instance Vulkan Instance.
     * @param[out] vk_return_surface Pointer to a VkSurfaceKHR handle.
     */
    virtual void getSurface(const VkInstance& vk_instance, VkSurfaceKHR* vk_return_surface) = 0;

    /**
     * @brief Get the Framebuffer size.
     * 
     * @param[out] width 
     * @param[out] height 
     */
    virtual void getFramebufferSize(int &width, int &height) = 0;
    virtual bool isMinimized() = 0;

    virtual void cleanup() = 0;
};

#endif // #ifndef WINDOW_CONTEXT_ABSTRACT_H
