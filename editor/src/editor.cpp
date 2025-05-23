#include "engine.h"
#include "utils/Log.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main() {
    granite::Engine engine;
    glfwInit();

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    GLFWwindow* window = glfwCreateWindow(640, 480, "My Title", NULL, NULL);
    
    if (window == NULL) {
        granite::Log::GetLogger()->critical("Cannot create editor window.");
    }

    engine.init();

    VkSurfaceKHR surface;    
    VkResult err = glfwCreateWindowSurface(engine.getRenderer().getInstance(), window, NULL, &surface);
    if (err) {
        granite::Log::GetLogger()->critical("Cannot create window surface : {0}", (int)err);
    }

    while (!glfwWindowShouldClose(window)){
    
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    return 0;
}
