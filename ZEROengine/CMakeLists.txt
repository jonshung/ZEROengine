cmake_minimum_required(VERSION 3.25)

project(ZEROengine VERSION 0.0.1 LANGUAGES CXX)
set(CMAKE_VERBOSE_MAKEFILE ON)

add_subdirectory(ZEROcore)

add_subdirectory(ZEROgraphical)

# module declaration
add_library(ZEROengine SHARED ${ZEROengineCore_Sources} ${ZEROengineGraphical_Sources})
add_library(ZEROengine::ZEROengine ALIAS ZEROengine)

target_include_directories(ZEROengine
PUBLIC
    ${ZEROengineCore_Includes}
    ${ZEROengineGraphical_Includes}
)

# requiring atleast C++17
target_compile_features(ZEROengine PRIVATE cxx_std_17)
target_compile_options(ZEROengine PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)