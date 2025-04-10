#pragma once

#include <memory>
#include <string>
#include <vector>
#include <chrono> 
#include <fstream> // For file logging
#include <stdexcept> // For runtime_error in getter

struct GLFWwindow; // Forward declare
namespace glm { struct vec3; struct mat4; } // Forward declare

// Forward declarations for engine components
namespace TurtleEngine {
    class Grid;
    class Shader;
    class ParticleSystem;
    namespace CSL { class CSLSystem; struct GestureResult; }
    namespace Combat { class ComboManager; struct ComboSequence; }
}

namespace TurtleEngine {

/**
 * @brief Main engine class that manages the game loop, rendering, and input handling.
 * 
 * The Engine class serves as the core of the application, coordinating between
 * the window system, renderer, and input manager. It maintains the game loop
 * and handles performance monitoring.
 */
class Engine {
public:
    /**
     * @brief Constructs a new Engine instance.
     * 
     * Initializes all pointers to nullptr and sets default values for
     * triangle properties and performance metrics.
     */
    Engine();

    /**
     * @brief Destroys the Engine instance.
     * 
     * Calls shutdown() to ensure proper cleanup of all resources.
     */
    ~Engine();
    
    /**
     * @brief Initializes the engine with the specified window parameters.
     * 
     * Creates and initializes the window, renderer, and input manager.
     * 
     * @param title The window title
     * @param width The window width in pixels
     * @param height The window height in pixels
     * @return true if initialization was successful, false otherwise
     */
    bool initialize(const std::string& windowTitle = "TurtleEngine", 
                   int width = 800, 
                   int height = 600);

    /**
     * @brief Starts the main game loop.
     * 
     * Handles input processing, updates game state, and renders frames
     * until the window is closed.
     */
    void run();

    /**
     * @brief Performs cleanup and releases all resources.
     * 
     * Deletes the window, renderer, and input manager instances.
     */
    void shutdown();

    void setCSLSystem(CSL::CSLSystem* sys);

    // --- Methods for test automation --- 
    void simulateKeyPress(char key);
    ParticleSystem& getParticleSystem() { 
        if (!m_particleSystem) { 
            throw std::runtime_error("ParticleSystem not initialized!"); 
        }
        return *m_particleSystem; 
    }
    void update(float deltaTime);
    // --- End Test Methods --- 

private:
    // Automated Test State
    bool m_automatedTestMode = false;
    int m_automatedTestPhase = 0;
    int m_testFrameCounter = 0;

    void onGestureRecognized(const CSL::GestureResult& result);
    bool initGL(); 
    void detectHardwareCapabilities();
    void configureOpenGLContext();
    void processInput();
    void updateCamera(); 

    // Helper for robust file logging
    void logToFile(const std::string& message);

    // Core components
    GLFWwindow* m_window = nullptr;
    bool m_isRunning = false;
    std::unique_ptr<CSL::CSLSystem> m_cslSystem;
    std::unique_ptr<Combat::ComboManager> m_comboManager;
    std::vector<Combat::ComboSequence> m_definedCombos;
    std::unique_ptr<ParticleSystem> m_particleSystem;
    std::unique_ptr<Grid> m_grid;
    
    struct Camera { // Simplified definition
        glm::vec3 position {0.0f, 5.0f, 15.0f};
        glm::vec3 target {0.0f, 0.0f, 0.0f}; 
        glm::vec3 up {0.0f, 1.0f, 0.0f};
        float yaw {-90.0f};
        float pitch { -20.0f};
        float distance {15.0f};
    } m_camera;
    
    struct HardwareInfo { // Simplified definition
        int maxTextureSize = 0;
        int maxUniformComponents = 0;
        int maxVertexAttributes = 0;
        std::string glVersion;
        std::string glVendor;
        std::string glRenderer;
    } m_hardwareInfo;

    struct PerformanceMetrics { // Simplified definition
        double lastFrameTime = 0.0;
        double deltaTime = 0.0;
    } m_performance;

    std::ofstream m_debugLog; // Debug log file stream

    // Method declarations that were modified or added
    void processInput(); // Renamed from handleInput

};

} // namespace TurtleEngine 
