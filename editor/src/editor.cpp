#include "engine.h"
#include "utils/Log.h"
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

int main() {
    granite::Engine* engine = granite::Engine::createInstance("Editor");
    engine->init();

    granite::Log::GetLogger()->info("Application name: {0}", engine->getApplicationName());
    while (!glfwWindowShouldClose(engine->getRenderer().getWindow())){
        glfwPollEvents();
		engine->getRenderer().Draw();
    }

    glfwDestroyWindow(engine->getRenderer().getWindow());
    return 0;
}
