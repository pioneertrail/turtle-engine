#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include <iostream>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include "csl/CSLSystem.hpp"
#include "csl/GestureRecognizer.hpp"
#include "combat/Combo.hpp"

namespace TurtleEngine {

Engine::Engine() : m_window(nullptr), m_isRunning(false) {
    m_performance.frameTime = 0.0;
    m_performance.fps = 0;
    m_performance.lastFrameTime = 0.0;

    // Initialize camera
    m_camera.position = glm::vec3(0.0f, 10.0f, 10.0f);
    m_camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    m_camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    m_camera.yaw = -45.0f;
    m_camera.pitch = -45.0f;
    m_camera.distance = 20.0f;

    m_cslSystem = std::make_unique<CSL::CSLSystem>();

    // Define combos (can be loaded from config later)
    ComboStep punch2 {"Punch2", std::chrono::milliseconds(400)}; // Example: Punch2 needs to follow within 400ms
    ComboStep punch1 {"Punch1", std::chrono::milliseconds(500), {}, {punch2}};
    m_definedCombos.push_back({"Basic_Punch_Combo", punch1});

    ComboStep kick1 {"Kick1", std::chrono::milliseconds(600)};
    m_definedCombos.push_back({"Basic_Kick", kick1});
    
    // Add CSL gesture names mapped to combo identifiers if needed
    // Make sure moveIdentifiers used here match names from onGestureRecognized
    ComboStep flammilSwipe {"Flammil", std::chrono::milliseconds(500)};
    m_definedCombos.push_back({"Flammil_Start", flammilSwipe});
    
    ComboStep khargailCharge_Step2 {"Punch1", std::chrono::milliseconds(300)}; // Khargail -> Punch1 ?
    ComboStep khargailCharge {"Khargail", std::chrono::milliseconds(600), {}, {khargailCharge_Step2}};
    m_definedCombos.push_back({"Khargail_Combo", khargailCharge});

    // Instantiate ComboManager with the defined combos
    m_comboManager = std::make_unique<ComboManager>(m_definedCombos);
}

Engine::~Engine() {
    shutdown();
}

bool Engine::initialize(const std::string& windowTitle, int width, int height) {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure GLFW for OpenGL 4.0
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    
    // Create window
    m_window = glfwCreateWindow(width, height, windowTitle.c_str(), nullptr, nullptr);
    if (!m_window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(m_window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return false;
    }

    // Detect hardware capabilities
    detectHardwareCapabilities();
    
    // Configure OpenGL context based on hardware
    configureOpenGLContext();

    // Create grid
    m_grid = std::make_unique<Grid>(20, 20, 1.0f);

    // Initialize and start CSL System
    if (!m_cslSystem || !m_cslSystem->initialize()) {
        std::cerr << "Failed to initialize CSL System" << std::endl;
        // Consider if this should be fatal - depends on game requirements
        // return false; 
    } else {
        // Register gesture callback
        m_cslSystem->registerGestureCallback(
            [this](const CSL::GestureResult& result) {
                this->onGestureRecognized(result);
            }
        );
        // Optionally register plasma callback if needed separately
        // m_cslSystem->addPlasmaCallback(...);

        if (!m_cslSystem->start()) {
             std::cerr << "Failed to start CSL System" << std::endl;
             // return false;
        }
    }

    // Set up input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
        if (action == GLFW_PRESS) {
            switch (key) {
                case GLFW_KEY_ESCAPE:
                    glfwSetWindowShouldClose(w, GLFW_TRUE);
                    break;
                case GLFW_KEY_W:
                    engine->m_camera.distance = std::max(5.0f, engine->m_camera.distance - 1.0f);
                    break;
                case GLFW_KEY_S:
                    engine->m_camera.distance = std::min(30.0f, engine->m_camera.distance + 1.0f);
                    break;
            }
        }
    });

    m_isRunning = true;
    return true;
}

void Engine::detectHardwareCapabilities() {
    // Get OpenGL information
    m_hardwareInfo.glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));
    m_hardwareInfo.glVendor = reinterpret_cast<const char*>(glGetString(GL_VENDOR));
    m_hardwareInfo.glRenderer = reinterpret_cast<const char*>(glGetString(GL_RENDERER));

    // Get hardware limits
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, &m_hardwareInfo.maxTextureSize);
    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &m_hardwareInfo.maxVertexAttributes);
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_COMPONENTS, &m_hardwareInfo.maxUniformComponents);

    // Log hardware information
    std::cout << "OpenGL Version: " << m_hardwareInfo.glVersion << std::endl;
    std::cout << "Vendor: " << m_hardwareInfo.glVendor << std::endl;
    std::cout << "Renderer: " << m_hardwareInfo.glRenderer << std::endl;
    std::cout << "Max Texture Size: " << m_hardwareInfo.maxTextureSize << std::endl;
    std::cout << "Max Vertex Attributes: " << m_hardwareInfo.maxVertexAttributes << std::endl;
    std::cout << "Max Uniform Components: " << m_hardwareInfo.maxUniformComponents << std::endl;
}

void Engine::configureOpenGLContext() {
    // Enable features based on hardware capabilities
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    
    // Set viewport
    int width, height;
    glfwGetFramebufferSize(m_window, &width, &height);
    glViewport(0, 0, width, height);
}

void Engine::processInput() {
    if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS) {
        m_camera.yaw -= 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS) {
        m_camera.yaw += 1.0f;
    }
    if (glfwGetKey(m_window, GLFW_KEY_Q) == GLFW_PRESS) {
        m_camera.pitch = std::min(0.0f, m_camera.pitch + 1.0f);
    }
    if (glfwGetKey(m_window, GLFW_KEY_E) == GLFW_PRESS) {
        m_camera.pitch = std::max(-89.0f, m_camera.pitch - 1.0f);
    }
}

void Engine::updateCamera() {
    float x = m_camera.distance * cos(glm::radians(m_camera.pitch)) * cos(glm::radians(m_camera.yaw));
    float y = m_camera.distance * sin(glm::radians(m_camera.pitch));
    float z = m_camera.distance * cos(glm::radians(m_camera.pitch)) * sin(glm::radians(m_camera.yaw));
    
    m_camera.position = glm::vec3(x, y, z);
}

void Engine::onGestureRecognized(const CSL::GestureResult& result) {
    // Convert GestureType to string/identifier
    std::string gestureName = "UNKNOWN"; 
    switch (result.type) {
        case CSL::GestureType::KHARGAIL: gestureName = "Khargail"; break;
        case CSL::GestureType::FLAMMIL: gestureName = "Flammil"; break;
        case CSL::GestureType::STASAI: gestureName = "Stasai"; break;
        case CSL::GestureType::ANNIHLAT: gestureName = "Annihlat"; break;
        case CSL::GestureType::NONE: gestureName = "None"; break;
        default: break;
    }

    std::cout << "[Engine Callback] Gesture Recognized: " << gestureName 
              << " | Confidence: " << result.confidence 
              // Note: GestureResult struct has transitionLatency, but CSLSystem doesn't seem to populate it yet.
              // << " | Latency: " << result.transitionLatency << " ms"
              << std::endl;

    // Process the move via ComboManager if a valid gesture name was found
    if (m_comboManager && gestureName != "UNKNOWN" && gestureName != "None") {
        // Add timing measurement here for latency check (Task 2.4)
        auto start_time = std::chrono::high_resolution_clock::now();
        
        m_comboManager->ProcessMove(gestureName);
        
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "    ComboManager::ProcessMove took: " << duration.count() << " us" << std::endl;
        
        // TODO: Check if duration meets < 0.1s (100,000 us) requirement and log/assert if not.
        if (duration.count() > 100000) {
            std::cerr << "!!!! WARNING: Combo processing took longer than 100ms !!!!" << std::endl;
        }
    }
}

void Engine::run() {
    while (m_isRunning && !glfwWindowShouldClose(m_window)) {
        // Calculate frame time
        auto currentTime = glfwGetTime();
        m_performance.frameTime = currentTime - m_performance.lastFrameTime;
        m_performance.lastFrameTime = currentTime;
        m_performance.fps = static_cast<int>(1.0 / m_performance.frameTime);

        processInput();
        
        // Update CSL System
        if (m_cslSystem) {
            m_cslSystem->update();
        }

        updateCamera();

        // Clear buffers
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create view and projection matrices
        glm::mat4 view = glm::lookAt(m_camera.position, m_camera.target, m_camera.up);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);

        // Render grid
        m_grid->render(view, projection);

        // Swap buffers and poll events
        glfwSwapBuffers(m_window);
        glfwPollEvents();
    }
}

void Engine::shutdown() {
    // Stop CSL System first
    if (m_cslSystem) {
        m_cslSystem->stop();
    }

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    m_isRunning = false;
}

} // namespace TurtleEngine 