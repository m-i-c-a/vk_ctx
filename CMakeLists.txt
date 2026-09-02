cmake_minimum_required(VERSION 3.16)

project(vk_ctx LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(CTest)

option(VK_CTX_BUILD_EXAMPLES "Build vk_ctx example executables" OFF)

include(FetchContent)

find_package(Vulkan REQUIRED)

add_library(vk_ctx_lib
    src/context.cpp
    src/device.cpp
    src/instance.cpp
    src/surface.cpp
)

target_include_directories(vk_ctx_lib
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(vk_ctx_lib PRIVATE Vulkan::Vulkan)

target_compile_features(vk_ctx_lib PUBLIC cxx_std_20)

target_compile_options(vk_ctx_lib PRIVATE
    $<$<CONFIG:Debug>:-Wall -Wextra -g>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

if (BUILD_TESTING)
    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )

    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    add_subdirectory(tests)
endif()

if (VK_CTX_BUILD_EXAMPLES)
    add_subdirectory(examples)
endif()
