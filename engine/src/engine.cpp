#include <iostream>
#include <vulkan/vulkan.h>
#include "engine.hpp"

void Granite::init() {
    uint32_t version = 0;
    vkEnumerateInstanceVersion(&version);

    std::cout << "test" << std::endl;
    std::cout << "Vulkan version: " << VK_VERSION_MAJOR(version) << "." << VK_VERSION_MINOR(version) << std::endl;
}