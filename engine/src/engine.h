#pragma once

#include <iostream>

#include "utils/Log.h"
#include "renderer/vk_renderer.h"

namespace granite {
    class Engine {
        public:
            void init();

            inline vkRenderer& getRenderer() {
                return renderer;
            }

            inline std::string getApplicationName() {
                return application_name;
            }

            static inline Engine* createInstance(std::string app) {
                if(instance == NULL) {
                    instance = new Engine(app);
                }
                return instance;
            }

            static inline Engine* getInstance() {
                return instance;
            }

        private:
            Engine(){}
            Engine(std::string app) : application_name(app) {}
            Engine& operator=(const Engine&) = delete;
            Engine(const Engine&) = delete;

            vkRenderer renderer;
            std::string application_name;
            static Engine* instance;
    };

}