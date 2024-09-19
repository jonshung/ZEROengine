#ifndef ZEROENGINE_ERRORS_H
#define ZEROENGINE_ERRORS_H

#include <stdexcept>
#include <string>
#include <cstdint>

// Some third party libraries requires fallthrough of switch block, this define should generalize the effects for all supported compilers
#define ZEROengine_FALLTHROUGH [[fallthrough]]

typedef enum ZEROResultEnum {
    // Generic Results
    ZERO_SUCCESS = 0,
    ZERO_FAILED = 1,
    ZERO_NULL_POINTER = 2,

    // Vulkan Graphics API Results
    ZERO_VULKAN_NULL_HANDLE = 10001,
    ZERO_VULKAN_VALIDATION_NOT_EXISTS = 10002,
    ZERO_VULKAN_VALIDATION_INSUFFICIENT = 10003,

    ZERO_VULKAN_CREATE_INSTANCE_FAILED = 10010,

    ZERO_VULKAN_NO_PHYSICAL_DEVICE = 10021,
    ZERO_VULKAN_NO_SUITABLE_PHYS_DEVICE = 10022,
    ZERO_VULKAN_CREATE_SURFACE_FAILED = 10023,

    ZERO_VULKAN_QUEUE_NOT_EXISTS = 10031,

    ZERO_VULKAN_CREATE_POOL_FAILED = 10041,

    ZERO_VULKAN_RENDERER_OUT_OF_DATE = 10051,
} ZEROResultEnum;

struct ZEROResult {
    ZEROResultEnum result_code;
    std::string result_string;
};

typedef enum ZEROPlatform {
    ZERO_PLATFORM_ANDROID,
    ZERO_PLATFORM_WIN32,
    ZERO_PLATFORM_WAYLAND,
    ZERO_PLATFORM_XLIB
} ZEROPlatform;

// Function name tracing

#if defined(_MSVC_VER)
    #define ZERO_FUNC_NAME std::string(__FUNCSIG__)
#elif defined(__GNUC__)
    #define ZERO_FUNC_NAME std::string(__PRETTY_FUNCTION__)
#elif defined(__clang__)
    #define ZERO_FUNC_NAME std::string(__PRETTY_FUNCTION__)
#endif

// helper macros
#define ZERO_EXCEPTION(err_num, exception_string) do { \
    std::string __err_string = std::string(exception_string); \
    uint32_t __err_num = static_cast<uint32_t>(err_num); \
    std::string __ret_string = ""; \
    __ret_string += std::string(ZERO_FUNC_NAME) + " failed, error(" + std::to_string(__err_num) + "): " + __err_string; \
    throw std::runtime_error(__ret_string); \
} while(0)

#define ZERO_CHECK_RESULT_RETURN(err) do { \
    ZEROResult __err = err; \
    if(__err.result_code) { \
        return __err; \
    } \
} while(0)

#define ZERO_CHECK_RESULT_EXCEPTION(err) do { \
    ZEROResult __err = err; \
    if(__err.result_code) { \
        ZERO_EXCEPTION(__err.result_code, __err.result_string); \
    } \
} while(0)

#define ZERO_CHECK_NULL_EXCEPTION(p) do { \
    if(!p) {\
        ZERO_EXCEPTION(ZERO_NULL_POINTER, "Null pointer"); \
    } \
} while (0)

#endif // #ifndef ZEROENGINE_ERRORS_H