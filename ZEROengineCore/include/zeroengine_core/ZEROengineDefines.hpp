#ifndef ZEROENGINE_DEFINES_H
#define ZEROENGINE_DEFINES_H

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
    ZERO_GRAPHICAL_ERROR = 1001,
    ZERO_GRAPHICAL_MOVE_OR_RESIZE = 1002,
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
#define ZERO_EXCEPT(err_num, exception_string) do { \
    std::string __ret_string = std::string(ZERO_FUNC_NAME) + \
    " failed, error(" + std::to_string(static_cast<uint32_t>(err_num)) + \
    "): " + std::string(exception_string); \
    throw std::runtime_error(__ret_string); \
} while(0)

#define ZERO_CHECK_RESULT_RETURN(err) do { \
    ZEROResult __err = err; \
    if(__err.result_code) { \
        return __err; \
    } \
} while(0)

#define ZERO_CHECK_RESULT_EXCEPT(err) do { \
    ZEROResult __err = err; \
    if(__err.result_code) { \
        ZERO_EXCEPT(__err.result_code, __err.result_string); \
    } \
} while(0)

#define ZERO_CHECK_NULL_EXCEPT(p) do { \
    if(!p) {\
        ZERO_EXCEPT(ZERO_NULL_POINTER, #p + "Null pointer"); \
    } \
} while (0)

#endif // #ifndef ZEROENGINE_ERRORS_H