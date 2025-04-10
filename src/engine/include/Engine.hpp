#pragma once

#include <memory>
#include <string>
#include <vector>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Grid.hpp"
#include "csl/CSLSystem.hpp"
#include "csl/GestureRecognizer.hpp"
#include "combat/Combo.hpp"
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

private:
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