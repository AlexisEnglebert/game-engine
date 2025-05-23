#pragma once

#include <map>
#include <vector>
#include <format>
#include <string>
#include <stdexcept>
#include <vulkan/vulkan.h>
#include "utils/Log.h"
#include "GLFW/glfw3.h"

namespace granite {
    class vkRenderer {
        public:
            vkRenderer() = default;
            bool init();
            inline VkInstance& getInstance() {
                return instance;
            }

        private:
            bool createInstance();
            bool pickPhysicalDevice();

            VkInstance instance;
            VkPhysicalDevice physicalDevice;
            VkDevice logical_device;
    };
}