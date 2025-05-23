cmake_minimum_required(VERSION 3.20)
project(Granite VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if (MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -pedantic)
endif()

add_subdirectory(external/glfw)
add_subdirectory(external/glm)
add_subdirectory(external/spdlog)

find_package(Vulkan REQUIRED)

set(ENGINE_SOURCES
    engine/src/utils/Log.cpp
    engine/src/engine.cpp
    engine/src/renderer/vk_renderer.cpp
)

add_library(Granite STATIC ${ENGINE_SOURCES})
target_include_directories(Granite PUBLIC engine/src)
target_link_libraries(Granite
    Vulkan::Vulkan
    glfw
    glm
    spdlog::spdlog
)

add_executable(GraniteEditor editor/src/editor.cpp)
target_link_libraries(GraniteEditor Granite glfw spdlog::spdlog)
