#ifndef WINDOW_CONTEXT_ABSTRACT_H
#define WINDOW_CONTEXT_ABSTRACT_H

#include <cstdint>

#if defined(__LP64__) || defined(_WIN64) || defined(__x86_64__) || defined(_M_X64) || defined(__ia64) || defined (_M_IA64) || defined(__aarch64__) || defined(__powerpc64__)
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef struct object##_T *object;
#else
#define VK_DEFINE_NON_DISPATCHABLE_HANDLE(object) typedef uint64_t object;
#endif

VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkSurfaceKHR)
VK_DEFINE_NON_DISPATCHABLE_HANDLE(VkInstance)

class WindowContext {
public:
    virtual void initWindow(uint32_t width, uint32_t height) = 0;
    virtual const char*const* requestInstanceExtensions(uint32_t &ext_count) const = 0;
    virtual void getSurface(const VkInstance&, VkSurfaceKHR*) = 0;

    virtual void getFramebufferSize(int &width, int &height) = 0;
    virtual bool isMinimized() = 0;

    virtual void cleanup() = 0;
};

#endif // #ifndef WINDOW_CONTEXT_ABSTRACT_H
