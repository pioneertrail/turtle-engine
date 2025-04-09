#include "engine/include/Engine.hpp"
#include <iostream>

int main() {
    TurtleEngine::Engine engine;

    // Initialize the engine
    if (!engine.initialize("TurtleEngine - OpenGL 4.0")) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return -1;
    }

    // Run the main loop
    engine.run();

    return 0;
} 