#pragma once

<<<<<<< HEAD
#include "Window.hpp"
#include "Renderer.hpp"
#include "InputManager.hpp"
#include <glm/glm.hpp>
#include <chrono>
#include <deque>

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
    bool initialize(const std::string& title, int width, int height);

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
    
private:
    /**
     * @brief Processes input and updates game state accordingly.
     * 
     * Handles keyboard input for:
     * - WASD: Triangle movement
     * - RGB: Color changes
     * - +/-: Size adjustment
     * - Tab: Toggle performance metrics
     */
    void handleInput();

    /**
     * @brief Updates the triangle's position, keeping it within screen bounds.
     */
    void updateTriangle();

    /**
     * @brief Renders the current frame using the renderer.
     */
    void render();

    /**
     * @brief Updates performance metrics for the current frame.
     * 
     * Calculates frame time and maintains a history of the last
     * MAX_FRAME_SAMPLES frames for statistical analysis.
     */
    void updatePerformanceMetrics();

    /**
     * @brief Displays current performance metrics in the console.
     * 
     * Shows FPS and frame time statistics (average, min, max).
     */
    void displayPerformanceMetrics();

    // Core components
    Window* window;           ///< Window management
    Renderer* renderer;       ///< OpenGL rendering
    InputManager* inputManager; ///< Input handling

    // Triangle state
    glm::vec2 trianglePos;    ///< Position in normalized device coordinates
    glm::vec4 triangleColor;  ///< RGBA color values
    float triangleSpeed;      ///< Movement speed per frame
    float triangleSize;       ///< Size in normalized device coordinates

    // Performance metrics
    std::chrono::high_resolution_clock::time_point lastFrameTime; ///< Timestamp of last frame
    std::deque<float> frameTimesMs;  ///< History of frame times in milliseconds
    int frameCount;                  ///< Total number of frames rendered
    float avgFrameTimeMs;           ///< Average frame time in milliseconds
    float minFrameTimeMs;           ///< Minimum frame time in milliseconds
    float maxFrameTimeMs;           ///< Maximum frame time in milliseconds
    float currentFPS;               ///< Current frames per second
    bool showMetrics;               ///< Whether to display performance metrics
    static constexpr int MAX_FRAME_SAMPLES = 120;  ///< Maximum number of frame time samples to keep
};

} // namespace TurtleEngine 
=======
#include <memory>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Grid.hpp"
#include "Shader.hpp"
#include "csl/CSLSystem.hpp"
#include "csl/GestureRecognizer.hpp"
#include "combat/Combo.hpp"
#include "ParticleSystem.hpp"
#include <mutex>
// Forward declare CSLSystem to avoid full include here if possible
// #include "csl/CSLSystem.hpp" 

namespace TurtleEngine {
    namespace CSL { class CSLSystem; } // Forward declaration
    struct GestureResult; // Forward declaration for callback type
    namespace Combat { // Assuming ComboManager is in Combat namespace based on file path
        class ComboManager;
        struct ComboSequence; // Needed for constructor/member type
    }
}

namespace TurtleEngine {

class Engine {
public:
    Engine();
    ~Engine();

    // Initialize the engine with hardware-aware settings
    bool initialize(const std::string& windowTitle = "TurtleEngine", 
                   int width = 800, 
                   int height = 600);

    // Main loop
    void run();

    // Cleanup
    void shutdown();

    // Set the CSL System instance
    void setCSLSystem(CSL::CSLSystem* sys);

private:
    // Callback handler for Flammyx gestures
    void handleFlammyxGesture(const CSL::GestureResult& result);

    // Hardware detection and configuration
    void detectHardwareCapabilities();
    void configureOpenGLContext();
    void processInput();
    void updateCamera();

    // Callback for CSL gesture results
    void onGestureRecognized(const CSL::GestureResult& result);

    // Core components
    GLFWwindow* m_window;
    bool m_isRunning;
    std::unique_ptr<CSL::CSLSystem> m_cslSystem;
    std::unique_ptr<ComboManager> m_comboManager;
    std::vector<ComboSequence> m_definedCombos;
    std::unique_ptr<ParticleSystem> m_particleSystem;
    
    // Camera
    struct {
        glm::vec3 position;
        glm::vec3 target;
        glm::vec3 up;
        float yaw;
        float pitch;
        float distance;
    } m_camera;

    // Grid
    std::unique_ptr<Grid> m_grid;

    // Flammyx Effect Rendering
    Shader m_flammyxShader;
    GLuint m_flammyxVao = 0;
    GLuint m_flammyxVbo = 0;
    size_t m_flammyxPointCount = 0;
    float m_flammyxDuration = 0.0f;
    std::mutex m_flammyxMutex;
    
    // Hardware info
    struct {
        int maxTextureSize;
        int maxUniformComponents;
        int maxVertexAttributes;
        std::string glVersion;
        std::string glVendor;
        std::string glRenderer;
    } m_hardwareInfo;

    // Performance monitoring
    struct {
        double frameTime;
        int fps;
        double lastFrameTime;
    } m_performance;
};
} 
>>>>>>> feature/step2-latency-opt
