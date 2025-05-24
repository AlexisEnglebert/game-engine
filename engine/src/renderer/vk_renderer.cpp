#include "renderer/vk_renderer.h"


bool granite::vkRenderer::init() {

    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);

    createInstance();
    if(!pickPhysicalDevice()) {
        return false;
    }

    if(!createLogicalDevice()) {
        return false;
    }

    VkResult err = glfwCreateWindowSurface(instance, window, NULL, &surface);
    if (err) {
        granite::Log::GetLogger()->critical("Cannot create window surface : {0}", (int)err);
    }

    if(!initSwapchain()) {
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
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    granite::Log::GetLogger()->debug("Number of required extensions: {0}", (int)glfwExtensionCount);
    for(uint32_t i = 0; i < glfwExtensionCount; i++)
        granite::Log::GetLogger()->debug(glfwExtensions[i]);

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;
    createInfo.enabledExtensionCount = glfwExtensionCount;
	createInfo.ppEnabledExtensionNames = glfwExtensions;
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

    if (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU) {
        score += 1000;
    }

    score += deviceProperties.limits.maxImageDimension2D;

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

bool granite::vkRenderer::createLogicalDevice() {
    granite::Log::GetLogger()->debug("Creating Logical Device.");
    
    // Check for device queue family
    QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

    VkDeviceQueueCreateInfo queueCreateInfo{};
    queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queueCreateInfo.queueFamilyIndex = indices.graphicsFamily.value();
    queueCreateInfo.queueCount = 1;
    
    float queuePriority = 1.0f;
    queueCreateInfo.pQueuePriorities = &queuePriority;


    // Create logical device
    VkPhysicalDeviceFeatures deviceFeatures{};
    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pQueueCreateInfos = &queueCreateInfo;
    createInfo.queueCreateInfoCount = 1;
    createInfo.pEnabledFeatures = &deviceFeatures;
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();

    //TODO: validation layers

    if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &logical_device) != VK_SUCCESS) {
        granite::Log::GetLogger()->error("Failed creating Logical device :( ");
        return false;
    }

    granite::Log::GetLogger()->debug("Finished creating Logical Device ");
    return true;

}


granite::QueueFamilyIndices granite::vkRenderer::findQueueFamilies(VkPhysicalDevice device) {
    granite::Log::GetLogger()->debug("Finding Queue family");
    
    granite::QueueFamilyIndices indices;
    uint32_t queueFamilyCount = 0;
    
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    
    int i = 0;
    for(const auto& queueFamily : queueFamilies){
        if(indices.isComplete()){
            break;
        }
        if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT){
            indices.graphicsFamily = i;
        }
        granite::Log::GetLogger()->info("FamilyIdx has values : {0}", indices.graphicsFamily.has_value());
        i++;
    }

    return indices;
}


bool granite::vkRenderer::initSwapchain() {
     granite::Log::GetLogger()->debug("Init Vulkan Swapchain.");
    
     if(!isDeviceSuitable(physicalDevice)) {
        granite::Log::GetLogger()->critical("Device doesn't support swapchains.");
        return false;
     }


    return true;
}

bool granite::vkRenderer::isDeviceSuitable(VkPhysicalDevice device) {
    granite::QueueFamilyIndices indices = findQueueFamilies(device);

    bool extensionsSupported = checkDeviceExtensionSupport(device);

    bool swapChainAdequate = false;
    if (extensionsSupported) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
        swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
    }

    return indices.isComplete() && extensionsSupported && swapChainAdequate;
}

bool granite::vkRenderer::checkDeviceExtensionSupport(VkPhysicalDevice device) {
     uint32_t extensionCount;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

    std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

    for (const auto& extension : availableExtensions) {
        requiredExtensions.erase(extension.extensionName);
    }

    return requiredExtensions.empty();
}


granite::SwapChainSupportDetails granite::vkRenderer::querySwapChainSupport(VkPhysicalDevice device)
{
    //Init details about our device and window for the swapchain 
    // refresh rate, color space etc...
    granite::SwapChainSupportDetails details;

    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &details.capabilities);
    if(res != VK_SUCCESS) {
        throw std::runtime_error("vkGetPhysicalDeviceSurfaceCapabilitiesKHR failed.");
    }

    uint32_t formatCount;

    res = vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);
    if(res != VK_SUCCESS) {
        throw std::runtime_error("vkGetPhysicalDeviceSurfaceFormatsKHR failed.");
    }

    granite::Log::GetLogger()->info("FormatCount: {0}",(int)formatCount);
    if (formatCount != 0) {
        details.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, details.formats.data());
    }

    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0) {
        details.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, details.presentModes.data());
    }
    
    return details;
}

VkSurfaceFormatKHR granite::vkRenderer::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR granite::vkRenderer::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes) {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            return availablePresentMode;
        }
    }

    return VK_PRESENT_MODE_FIFO_KHR;
}

VkExtent2D granite::vkRenderer::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) {
        return capabilities.currentExtent;
    } else {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {
            static_cast<uint32_t>(width),
            static_cast<uint32_t>(height)
        };

        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);

        return actualExtent;
    }
}
