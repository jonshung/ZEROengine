#include "zeroengine_core/ZEROengine.hpp"

#include <vector>
#include <array>
#include <iostream>

void ZEROengine::init() {
    this->graphical_module->initGraphicalModule();
}

void ZEROengine::run() {
    mainLoop();
}

void ZEROengine::mainLoop() {
    
    while(!this->quitting_signal) {
        this->graphical_module->drawFrame();
        this->graphical_module->getWindowContext()->poll();
        this->quitting_signal = this->graphical_module->getWindowContext()->isClosing();
    }
}

void ZEROengine::cleanup() {
    this->graphical_module->cleanup();
}

ZEROengine::~ZEROengine() {
    cleanup();
}