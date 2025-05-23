#pragma once

#include <iostream>

#include "utils/Log.h"
#include "renderer/vk_renderer.h"

namespace granite {
    class Engine {
        public:
            Engine() {
                granite::Log::Init();
            }

            void init();

            inline vkRenderer& getRenderer() {
                return renderer;
            }

        private:
            vkRenderer renderer;
    };
}