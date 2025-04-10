#pragma once

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
#include <fstream> // For file logging
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
        double deltaTime;
    } m_performance;

    std::ofstream m_debugLog; // Debug log file stream
};
} 