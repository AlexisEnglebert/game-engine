#include "renderer/vk_renderer.h"
#include <iostream>


bool granite::vkRenderer::init() {
    createInstance();
    if(!pickPhysicalDevice()) {
        return false;
    }

    granite::Log::GetLogger()->debug("Vulkan initialization sucess.");
    return true;
}

bool granite::vkRenderer::createInstance() {
    granite::Log::GetLogger()->debug("Creating vulkan instance");
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Engine";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "Granite";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount); // use gltf exetensions for vulkan 

    granite::Log::GetLogger()->debug("Number of required extensions: {0}", (int)glfwExtensionCount);
    for(int i = 0; i < glfwExtensionCount; i++)
        granite::Log::GetLogger()->debug(glfwExtensions[i]);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount; // Need extension to handle every platforms
	createInfo.ppEnabledExtensionNames = glfwExtensions;// Need extension to handle every platforms
	createInfo.enabledLayerCount = 0; // reste obscur pour l'instant

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to create instance!");
    }

    return true;

}

int rateDeviceSuitability(VkPhysicalDevice device) {
    int score = 0;

    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    // Discrete GPUs have a significant performance advantage
    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    // Maximum possible size of textures affects graphics quality
    score += deviceProperties.limits.maxImageDimension2D;

    // Application can't function without geometry shaders
    if (!deviceFeatures.geometryShader) {
        return 0;
    }

    return score;
}

bool granite::vkRenderer::pickPhysicalDevice() {
    granite::Log::GetLogger()->debug("Choosing physical device");

    uint32_t deviceCount = 0;
    VkResult res = vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);
    if (res != VK_SUCCESS) {
         throw std::runtime_error("failed to enumerate physical devices.");
    }

    std::vector<VkPhysicalDevice> devices(deviceCount);
    res = vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());
    if (res != VK_SUCCESS) {
         throw std::runtime_error("failed to enumerate physical devices.");
    }

    if (deviceCount <= 0){
        granite::Log::GetLogger()->error("No suitable device found.");
        return false;
    }

    granite::Log::GetLogger()->info("Found {0} device",deviceCount);

    std::multimap<int, VkPhysicalDevice> candidates;
    for(const auto& device : devices) {

        int score = rateDeviceSuitability(device);
        candidates.insert(std::make_pair(score, device));
    }

    // Check if the best candidate is suitable at all
    if (candidates.rbegin()->first > 0) {
        this->physicalDevice = candidates.rbegin()->second;
    } else {
        throw std::runtime_error("failed to find a suitable GPU!");
    }

    VkPhysicalDeviceProperties deviceProperty;
    vkGetPhysicalDeviceProperties(devices[0], &deviceProperty);
    granite::Log::GetLogger()->info("Device : {0}", deviceProperty.deviceName);
    granite::Log::GetLogger()->info("VendorID : {0}", deviceProperty.vendorID);
    granite::Log::GetLogger()->info("DeviceID : {0}", deviceProperty.deviceID);
    granite::Log::GetLogger()->info("DeviceType : {0}", (int)deviceProperty.deviceType);

    return true;
}
