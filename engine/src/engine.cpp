#include "engine.h"

void granite::init() {
    granite::Log::Init();
    vkRenderer renderer;
    renderer.init();
}