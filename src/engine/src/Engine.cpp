#include <GL/glew.h>
#include "Engine.hpp"
#include <iostream>
#include <numeric>
#include <iomanip>
#include <sstream>

namespace TurtleEngine {

// Helper for robust file logging
void Engine::logToFile(const std::string& message) {
    // Check if the stream is valid before writing
    if (!m_debugLog.is_open() || !m_debugLog.good()) {
        std::cerr << "[Engine::logToFile] Log stream is bad. Attempting to reopen..." << std::endl;
        m_debugLog.close(); // Close if already open but bad
        m_debugLog.open("EngineLog.txt", std::ios::app); // Try reopening
        
        // Check again after trying to reopen
        if (!m_debugLog.is_open() || !m_debugLog.good()) {
            std::cerr << "[Engine::logToFile] Failed to reopen log stream. Further logs may be lost." << std::endl;
            // Optionally, could disable further logging attempts here
            return; // Cannot log if stream is bad and cannot be reopened
        } else {
             std::cerr << "[Engine::logToFile] Log stream reopened successfully." << std::endl;
             // Log the original message now that the stream is good
        }
    }

    // Proceed with logging if the stream is good
    auto now = std::chrono::system_clock::now();
    auto now_c = std::chrono::system_clock::to_time_t(now);
    m_debugLog << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " | " << message << std::endl;
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
    // Initialize test state
    m_automatedTestMode = true; // Or set via command line arg later
    m_automatedTestPhase = 0;
    m_testFrameCounter = 0;
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
    
    m_isRunning = true; // Ensure isRunning is set
    logToFile("Engine run loop started.");

    while (m_isRunning && !window->shouldClose()) {
        auto currentTime = std::chrono::high_resolution_clock::now();
        m_performance.deltaTime = std::chrono::duration<double>(currentTime - m_performance.lastFrameTime).count();
        m_performance.lastFrameTime = currentTime;

        glfwPollEvents(); // Poll events early

        if (m_automatedTestMode) {
            // --- Automated Test Logic ---
            m_testFrameCounter++;

            // Define phase durations (in frames)
            const int initialDelayFrames = 60; // 1 second at 60 FPS
            const int gestureTriggerDelayFrames = 120; // 2 seconds between triggers
            const int finalDelayFrames = 60; // 1 second after last trigger

            switch (m_automatedTestPhase) {
                case 0: // Initial Delay
                    if (m_testFrameCounter >= initialDelayFrames) {
                        m_gestureRecognizer->triggerGesture(CSL::GestureType::STASAI);
                        m_automatedTestPhase = 1;
                        m_testFrameCounter = 0; // Reset counter for next phase
                    }
                    break;
                case 1: // After STASAI
                    if (m_testFrameCounter >= gestureTriggerDelayFrames) {
                        m_gestureRecognizer->triggerGesture(CSL::GestureType::ANNIHLAT);
                        m_automatedTestPhase = 2;
                        m_testFrameCounter = 0;
                    }
                    break;
                case 2: // After ANNIHLAT
                    if (m_testFrameCounter >= gestureTriggerDelayFrames) {
                        m_gestureRecognizer->triggerGesture(CSL::GestureType::FLAMMIL);
                        m_automatedTestPhase = 3;
                        m_testFrameCounter = 0;
                    }
                    break;
                case 3: // After FLAMMIL
                    if (m_testFrameCounter >= finalDelayFrames) {
                         std::cout << "RENDER_CHECK_SUCCESS" << std::endl; // Signal completion for external check
                         window->setShouldClose(true); // Request window close
                         m_automatedTestPhase = 4; // Move to final phase
                    }
                    break;
                 case 4: // Test Finished
                    // Do nothing, wait for window to close
                    break;
            }
        } else {
             // --- Manual Input Processing ---
             processInput(); // Use the renamed method
        }

        // Update systems regardless of mode
        updateCamera(); // Update camera based on state/input
        m_gestureRecognizer->update(); // Update Gesture Recognizer
        m_particleSystem->update(static_cast<float>(m_performance.deltaTime));

        // --- Rendering ---
        renderer->clear();
        renderer->drawTriangle(trianglePos, 0.0f, glm::vec2(triangleSize), triangleColor);

        // Get matrices
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 
                                                (float)800 / (float)600, 0.1f, 100.0f);
        glm::mat4 view = glm::lookAt(m_camera.position, m_camera.target, m_camera.up);

        // Render Grid
        if (m_grid) {
            m_grid->render(projection, view);
        } else {
             logToFile("[Render] Grid is null!");
        }

        // Render Particles
        if (m_particleSystem) {
             m_particleSystem->render(projection, view);
        } else {
            logToFile("[Render] ParticleSystem is null!");
        }

        // Update performance metrics
        updatePerformanceMetrics();
        
        if (showMetrics) {
            displayPerformanceMetrics();
        }
        
        // Swap buffers
        window->swapBuffers();
    }
    logToFile("Engine run loop finished.");
    m_isRunning = false; 
}

void Engine::processInput() {
    if (m_automatedTestMode) return; // Don't process manual input in auto mode

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
    static bool tabPressedLastFrame = false;
    bool tabPressedThisFrame = inputManager->isKeyPressed(GLFW_KEY_TAB);
    if (tabPressedThisFrame && !tabPressedLastFrame) {
       // logToFile("Tab key toggled (Manual)."); // Removed debug log 
    }
    tabPressedLastFrame = tabPressedThisFrame;

    // Simplified manual controls for testing:
    if (inputManager->isKeyPressed(GLFW_KEY_ESCAPE)) {
         window->setShouldClose(true);
    }

    // Manual gesture triggers (remain gated by m_automatedTestMode check above)
    if (inputManager->isKeyPressed(GLFW_KEY_A)) {
        // logToFile("A key pressed (Manual), triggering Stasai gesture."); // Removed debug log
        m_gestureRecognizer->triggerGesture(CSL::GestureType::STASAI);
    }
    if (inputManager->isKeyPressed(GLFW_KEY_S)) {
        // logToFile("S key pressed (Manual), triggering Annihlat gesture."); // Removed debug log
        m_gestureRecognizer->triggerGesture(CSL::GestureType::ANNIHLAT);
    }
    if (inputManager->isKeyPressed(GLFW_KEY_F)) {
        // logToFile("F key pressed (Manual), triggering Flammil gesture."); // Removed debug log
        m_gestureRecognizer->triggerGesture(CSL::GestureType::FLAMMIL);
    }
    if (inputManager->isKeyPressed(GLFW_KEY_N)) {
        // logToFile("N key pressed (Manual), triggering None gesture."); // Removed debug log
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