#pragma once

#include <set>
#include <map>
#include <vector>
#include <format>
#include <string>
#include <iostream>
#include <optional>
#include <stdexcept>
#include <vulkan/vulkan.h>

#include "utils/Log.h"
#include "GLFW/glfw3.h"

namespace granite {

    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;

        bool isComplete() {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
        std::vector<VkSurfaceFormatKHR> formats;
        std::vector<VkPresentModeKHR> presentModes;
    };

    class vkRenderer {
        public:
            vkRenderer() = default;
            bool init();
            bool initSwapchain();

            inline VkInstance& getInstance() {
                return instance;
            }

            inline VkSurfaceKHR& getSurface() {
                return surface;
            }

            inline GLFWwindow* getWindow() {
                return window;
            }

            void Draw();

        private:
            bool createInstance();
            bool pickPhysicalDevice();
            bool createLogicalDevice();
            bool isDeviceSuitable(VkPhysicalDevice device);
            bool checkDeviceExtensionSupport(VkPhysicalDevice device);
            void createSwapChain();
            void createGraphicsPipeline();
            void cleanup();
            void createRenderPass();
            void createFrameBuffer();
            void createCommandPool();
            void createCommandBuffer();
            void createImageViews();
            void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
			void createSemaphoresAndFences();

            VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
            VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
            VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);


            QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
            SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);

            VkQueue graphicsQueue;
            VkQueue presentQueue;

            VkInstance instance;
            VkSurfaceKHR surface;    
            VkPhysicalDevice physicalDevice;
            VkDevice logical_device;
            VkSwapchainKHR swapChain;
            VkRenderPass renderPass;
            VkCommandPool commandPool;
            VkCommandBuffer commandBuffer;

            VkSemaphore imageAvailableSemaphore;
            VkSemaphore renderFinishedSemaphore;
            VkFence inFlightFence;

            std::vector<VkImage> swapChainImages;
            std::vector<VkImageView> swapChainImageViews;
            std::vector<VkFramebuffer> swapChainFramebuffers;
            VkFormat swapChainImageFormat;
            VkExtent2D swapChainExtent;

            GLFWwindow* window;

            const std::vector<const char*> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
    };
}