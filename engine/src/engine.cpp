#include "engine.h"

granite::Engine* granite::Engine::instance = NULL;

void granite::Engine::init() {
    granite::Log::Init();
    renderer.init();
}
