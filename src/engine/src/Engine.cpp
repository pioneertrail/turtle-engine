#include <GL/glew.h>
#include "Engine.hpp"
#include <iostream>
#include <numeric>
#include <iomanip>
#include <sstream>

namespace TurtleEngine {

// Helper function for robust logging
void Engine::logToFile(const std::string& message) {
    if (!m_debugLog.is_open() || !m_debugLog.good()) {
        std::cerr << "Log stream bad, attempting reopen... Message: " << message << std::endl;
        m_debugLog.close();
        m_debugLog.open("mengin_debug.log", std::ios::app);
        if (!m_debugLog.is_open() || !m_debugLog.good()) {
            std::cerr << "Failed to reopen log file. Logging to stderr instead." << std::endl;
            std::cerr << message << std::endl; // Fallback to stderr
            return;
        }
        std::cerr << "Log stream reopened successfully." << std::endl;
    }
    m_debugLog << message << std::endl;
}

Engine::Engine() 
    : window(nullptr)
    , renderer(nullptr)
    , inputManager(nullptr)
    , trianglePos(0.0f)
    , triangleColor(1.0f, 0.0f, 0.0f, 1.0f)
    , triangleSpeed(0.01f)
    , triangleSize(0.3f)
    , frameCount(0)
    , avgFrameTimeMs(0.0f)
    , minFrameTimeMs(std::numeric_limits<float>::max())
    , maxFrameTimeMs(0.0f)
    , currentFPS(0.0f)
    , showMetrics(true) {
    lastFrameTime = std::chrono::high_resolution_clock::now();
    logToFile("=== Engine Start ===");
}

Engine::~Engine() {
    logToFile("Engine destructor starting.");
    shutdown();
    logToFile("=== Engine Stop ===");
    if (m_debugLog.is_open()) {
        m_debugLog.close();
    }
}

bool Engine::initialize(const std::string& title, int width, int height) {
    try {
        // Create window
        window = new Window(width, height, title);
        if (!window->isValid()) {
            std::cerr << "Failed to create window" << std::endl;
            return false;
        }
        
        // Create renderer
        renderer = new Renderer();
        renderer->init();
        
        // Create input manager
        inputManager = new InputManager(window->getHandle());
        
        logToFile("Engine Initialized successfully.");
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error initializing engine: " << e.what() << std::endl;
        return false;
    }
}

void Engine::run() {
    if (!window || !renderer || !inputManager) {
        std::cerr << "Engine not initialized" << std::endl;
        return;
    }
    
    while (!window->shouldClose()) {
        // Update performance metrics
        updatePerformanceMetrics();
        
        // Update input and game state
        inputManager->update();
        handleInput();
        updateTriangle();
        
        // Render frame
        render();
        
        if (showMetrics) {
            displayPerformanceMetrics();
        }
        
        // Swap buffers
        window->swapBuffers();
        window->pollEvents();
        
        frameCount++;
    }
}

void Engine::handleInput() {
    // Move triangle with WASD
    if (inputManager->isKeyPressed(GLFW_KEY_W)) {
        trianglePos.y += triangleSpeed;
    }
    if (inputManager->isKeyPressed(GLFW_KEY_S)) {
        trianglePos.y -= triangleSpeed;
    }
    if (inputManager->isKeyPressed(GLFW_KEY_A)) {
        trianglePos.x -= triangleSpeed;
    }
    if (inputManager->isKeyPressed(GLFW_KEY_D)) {
        trianglePos.x += triangleSpeed;
    }

    // Change color with RGB keys
    if (inputManager->isKeyPressed(GLFW_KEY_R)) {
        triangleColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
    }
    if (inputManager->isKeyPressed(GLFW_KEY_G)) {
        triangleColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);
    }
    if (inputManager->isKeyPressed(GLFW_KEY_B)) {
        triangleColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);
    }

    // Change size with + and -
    if (inputManager->isKeyPressed(GLFW_KEY_EQUAL)) {
        triangleSize = std::min(triangleSize + 0.01f, 1.0f);
    }
    if (inputManager->isKeyPressed(GLFW_KEY_MINUS)) {
        triangleSize = std::max(triangleSize - 0.01f, 0.1f);
    }

    // Toggle performance metrics display with Tab
    if (inputManager->isKeyPressed(GLFW_KEY_TAB)) {
        showMetrics = !showMetrics;
    }

    // Add STASAI gesture trigger
    if (inputManager->isKeyPressed(GLFW_KEY_A)) {
        logToFile("A key pressed, triggering Stasai gesture.");
        m_gestureRecognizer->triggerGesture(CSL::GestureType::STASAI);
    }

    // Add ANNIHLAT gesture trigger
    if (inputManager->isKeyPressed(GLFW_KEY_S)) {
        logToFile("S key pressed, triggering Annihlat gesture.");
        m_gestureRecognizer->triggerGesture(CSL::GestureType::ANNIHLAT);
    }

    // Add NONE gesture trigger (for testing or specific scenarios)
    if (inputManager->isKeyPressed(GLFW_KEY_N)) {
        logToFile("N key pressed, triggering None gesture.");
        m_gestureRecognizer->triggerGesture(CSL::GestureType::NONE);
    }
}

void Engine::updateTriangle() {
    // Keep triangle within screen bounds
    trianglePos = glm::clamp(trianglePos, glm::vec2(-1.0f + triangleSize), glm::vec2(1.0f - triangleSize));
}

void Engine::render() {
    renderer->clear();
    renderer->drawTriangle(trianglePos, 0.0f, glm::vec2(triangleSize), triangleColor);
}

void Engine::updatePerformanceMetrics() {
    auto currentTime = std::chrono::high_resolution_clock::now();
    float frameTime = std::chrono::duration<float, std::milli>(currentTime - lastFrameTime).count();
    lastFrameTime = currentTime;

    // Update frame time history
    frameTimesMs.push_back(frameTime);
    if (frameTimesMs.size() > MAX_FRAME_SAMPLES) {
        frameTimesMs.pop_front();
    }

    // Calculate statistics
    avgFrameTimeMs = std::accumulate(frameTimesMs.begin(), frameTimesMs.end(), 0.0f) / frameTimesMs.size();
    minFrameTimeMs = *std::min_element(frameTimesMs.begin(), frameTimesMs.end());
    maxFrameTimeMs = *std::max_element(frameTimesMs.begin(), frameTimesMs.end());
    currentFPS = 1000.0f / avgFrameTimeMs;
}

void Engine::displayPerformanceMetrics() {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(2)
       << "FPS: " << currentFPS << "\n"
       << "Frame time (ms) - Avg: " << avgFrameTimeMs 
       << " Min: " << minFrameTimeMs 
       << " Max: " << maxFrameTimeMs;
    
    std::cout << "\033[2J\033[H";  // Clear console and move cursor to top
    std::cout << ss.str() << std::endl;
}

void Engine::shutdown() {
    delete inputManager;
    delete renderer;
    delete window;
    
    inputManager = nullptr;
    renderer = nullptr;
    window = nullptr;
}

} // namespace TurtleEngine 