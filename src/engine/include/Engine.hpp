#pragma once

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