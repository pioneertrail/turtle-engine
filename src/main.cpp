<<<<<<< HEAD
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "Engine.hpp"

using namespace TurtleEngine;

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // Create engine instance
    Engine engine;
    if (!engine.initialize("Turtle Engine", 1280, 720)) {
        std::cerr << "Failed to initialize engine" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    // Run the engine
    engine.run();

    // Cleanup
    glfwTerminate();
=======
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

>>>>>>> feature/step2-latency-opt
    return 0;
} 