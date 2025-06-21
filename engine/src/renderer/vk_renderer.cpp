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

    createImageViews();
    createRenderPass();
    createFrameBuffer();

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

     createSwapChain();

    return true;
}

void granite::vkRenderer::createSwapChain() {
    granite::SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);

    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);
    
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) {
        imageCount = swapChainSupport.capabilities.maxImageCount;
    }

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT; // Directly draw to the framebuffer
    //VK_IMAGE_USAGE_TRANSFER_DST_BIT to draw into a image.
    createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    createInfo.queueFamilyIndexCount = 0; // Optional
    createInfo.pQueueFamilyIndices = nullptr; // Optional
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult res = vkCreateSwapchainKHR(logical_device, &createInfo, nullptr, &swapChain);
    if (res != VK_SUCCESS) {
        throw std::runtime_error("failed to create swap chain!");
    }    

    vkGetSwapchainImagesKHR(logical_device, swapChain, &imageCount, nullptr);
    swapChainImages.resize(imageCount);
    vkGetSwapchainImagesKHR(logical_device, swapChain, &imageCount, swapChainImages.data());
    swapChainImageFormat = surfaceFormat.format;
    swapChainExtent = extent;
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

void granite::vkRenderer::createImageViews() {
    swapChainImageViews.resize(swapChainImages.size());
    for (size_t i = 0; i < swapChainImages.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapChainImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapChainImageFormat;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        if (vkCreateImageView(logical_device, &createInfo, nullptr, &swapChainImageViews[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create image views!");
        }
    }
}

void granite::vkRenderer::createRenderPass() {
    
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;

    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;

    if (vkCreateRenderPass(logical_device, &renderPassInfo, nullptr, &renderPass) != VK_SUCCESS) {
        throw std::runtime_error("failed to create render pass!");
    }
}

void granite::vkRenderer::createFrameBuffer() {
    swapChainFramebuffers.resize(swapChainImageViews.size());
    for(size_t i = 0; i < swapChainImageViews.size(); i++) {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        VkFramebufferCreateInfo framebufferInfo{};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = renderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(logical_device, &framebufferInfo, nullptr, &swapChainFramebuffers[i]) != VK_SUCCESS) {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }  
}

void granite::vkRenderer::createGraphicsPipeline() {
    /*
    * The graphics pipeline in Vulkan is almost completely immutable, so you must recreate the pipeline 
    * from scratch if you want to change shaders, bind different framebuffers or change the blend function. 
    * The disadvantage is that you'll have to create a number of pipelines that represent all of the different 
    * combinations of states you want to use in your rendering operations. 
    */

    
}

void granite::vkRenderer::cleanup() {
    for (auto imageView : swapChainImageViews) {
        vkDestroyImageView(logical_device, imageView, nullptr);
    }

    for (auto framebuffer : swapChainFramebuffers) {
        vkDestroyFramebuffer(logical_device, framebuffer, nullptr);
    }

    vkDestroySwapchainKHR(logical_device, swapChain, nullptr);
    //TODO GLF WINDOW ETC...
}
