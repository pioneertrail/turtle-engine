#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "Engine.hpp"
#include <iostream>
#include <chrono>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
#include "csl/CSLSystem.hpp"
#include "csl/GestureRecognizer.hpp"
#include "combat/Combo.hpp"
#include "ParticleSystem.hpp"
#include <random>

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

    // Correct initialization for raw pointer
    m_cslSystem = std::make_unique<CSL::CSLSystem>(); 

    // Define combos (can be loaded from config later)
    // Corrected ComboStep aggregate initialization
    ComboStep punch2 {"Punch2", std::chrono::milliseconds(400)}; 
    ComboStep punch1 {"Punch1", std::chrono::milliseconds(500), {punch2}}; // ID, Time, NextSteps
    m_definedCombos.push_back({"Basic_Punch_Combo", punch1});

    ComboStep kick1 {"Kick1", std::chrono::milliseconds(600)};
    m_definedCombos.push_back({"Basic_Kick", kick1});
    
    // Add CSL gesture names mapped to combo identifiers if needed
    // Make sure moveIdentifiers used here match names from onGestureRecognized
    ComboStep flammilSwipe {"Flammil", std::chrono::milliseconds(500)};
    m_definedCombos.push_back({"Flammil_Start", flammilSwipe});
    
    // Corrected ComboStep aggregate initialization
    ComboStep khargailCharge_Step2 {"Punch1", std::chrono::milliseconds(300)}; 
    ComboStep khargailCharge {"Khargail", std::chrono::milliseconds(600), {khargailCharge_Step2}}; // ID, Time, NextSteps
    m_definedCombos.push_back({"Khargail_Combo", khargailCharge});

    // Instantiate ComboManager with the defined combos
    m_comboManager = std::make_unique<ComboManager>(m_definedCombos);
    m_particleSystem = std::make_unique<ParticleSystem>(5000); // Allow up to 5000 particles
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

    // Initialize Particle System
    if (!m_particleSystem || !m_particleSystem->initialize()) {
        std::cerr << "Failed to initialize Particle System" << std::endl;
        // Optionally return false if particles are critical
    }

    // Set up input callbacks
    glfwSetWindowUserPointer(m_window, this);
    glfwSetKeyCallback(m_window, [](GLFWwindow* w, int key, int scancode, int action, int mods) {
        auto engine = static_cast<Engine*>(glfwGetWindowUserPointer(w));
        if (action == GLFW_PRESS) {
            // Record key press time for latency measurement
            auto keyPressTime = std::chrono::high_resolution_clock::now(); 

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
                // Quick Sign Keys
                case GLFW_KEY_F: // Trigger Flammil
                    std::cout << "[Input] F key pressed, triggering Flammil.\n";
                    if (engine->m_cslSystem) {
                        engine->m_cslSystem->triggerGesture(CSL::GestureType::FLAMMIL);
                    }
                    break;
                case GLFW_KEY_C: // Trigger Khargail
                    std::cout << "[Input] C key pressed, triggering Khargail.\n";
                    if (engine->m_cslSystem) {
                        engine->m_cslSystem->triggerGesture(CSL::GestureType::KHARGAIL);
                    }
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
              << std::endl;

    // Add debug logging for trajectory and velocities
    std::cout << "[Gesture Debug] Type: " << static_cast<int>(result.type) 
              << ", Trajectory Size: " << result.trajectory.size() 
              << ", Velocities Size: " << result.velocities.size() << std::endl;

    // --- Latency Check for Triggered Gestures --- 
    if (result.triggerTimestamp.has_value()) {
        auto callbackTime = std::chrono::high_resolution_clock::now();
        auto totalLatency = std::chrono::duration_cast<std::chrono::microseconds>(callbackTime - result.triggerTimestamp.value());
        std::cout << "    QuickSign Latency (Key Press -> Callback): " << totalLatency.count() << " us" << std::endl;
        // TODO: Log this to gesture_recognition_results.txt as required by task
        if (totalLatency.count() > 500000) { // 0.5 seconds
            std::cerr << "!!!! WARNING: QuickSign latency > 500ms !!!!" << std::endl;
        }
    }
    // --- End Latency Check ---

    // Process the move via ComboManager 
    bool moveProcessed = false;
    std::string currentComboMove;
    if (m_comboManager && gestureName != "UNKNOWN" && gestureName != "None") {
        // Timing for ComboManager processing itself
        auto start_time = std::chrono::high_resolution_clock::now();
        m_comboManager->ProcessMove(gestureName);
        currentComboMove = m_comboManager->getCurrentStateDebug(); // Get state *after* processing
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "    ComboManager::ProcessMove took: " << duration.count() << " us" << std::endl;
        
        if (duration.count() > 100000) {
            std::cerr << "!!!! WARNING: Combo processing took longer than 100ms !!!!" << std::endl;
        }
        if (!currentComboMove.empty()) { // Check if move resulted in a valid combo state
             moveProcessed = true;
        }
    }

    // --- Spawn FLAMMIL Ember Particles --- 
    // Note: result.trajectory and result.velocities may be empty if the gesture was
    // directly triggered (e.g., via keybind in CSLSystem::triggerGesture) instead of
    // recognized from tracked points. The logic here needs to account for this.
    // Updated CSLSystem::triggerGesture now provides minimal data for testing.
    if (result.type == CSL::GestureType::FLAMMIL && m_particleSystem && 
        !result.trajectory.empty() && result.trajectory.size() == result.velocities.size()) 
    {
        std::cout << "    Spawning FLAMMIL ember trail..." << std::endl;
        int windowWidth, windowHeight;
        glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);
        windowWidth = std::max(1, windowWidth); // Avoid division by zero
        windowHeight = std::max(1, windowHeight);

        const int spawnRate = 5; // Adjusted from 3 for demo clarity
        const float baseLifetime = 0.4f;
        const float lifetimeVariance = 0.2f;
        const float baseSpeed = 0.1f; // Low speed for embers
        const int burstCount = 10; // Boosted to 10 particles per point

        for (size_t i = 0; i < result.trajectory.size(); ++i) {
            if (i >= result.velocities.size()) break; 

            if (i % spawnRate == 0) { // Spawn every 5th point
                float velocityNormalized = result.velocities[i]; 
                float intensity = glm::clamp(0.5f + pow(velocityNormalized, 2.0f) * 0.5f, 0.5f, 1.0f); 
                float hotMix = glm::smoothstep(0.6f, 0.9f, velocityNormalized);
                glm::vec3 dynamicColorVec3 = glm::mix(glm::vec3(1.0, 0.4, 0.0), glm::vec3(1.0, 0.8, 0.2), hotMix) * intensity;
                glm::vec4 emberColor = glm::vec4(dynamicColorVec3, 0.8f); 

                float ndcX = (result.trajectory[i].x / static_cast<float>(windowWidth)) * 2.0f - 1.0f;
                float ndcY = 1.0f - (result.trajectory[i].y / static_cast<float>(windowHeight)) * 2.0f; 
                glm::vec3 spawnPos = glm::vec3(ndcX * 5.0f, ndcY * 5.0f, -1.0f); 
                
                float lifetime = baseLifetime + (static_cast<float>(rand()) / RAND_MAX - 0.5f) * lifetimeVariance * 2.0f;
                
                m_particleSystem->spawnBurst(burstCount, spawnPos, baseSpeed, lifetime, emberColor);
            }
        }
        // Dummy hit feedback
        glm::vec3 hitPos = glm::vec3(0.0f, 0.0f, -1.0f);  // Center screen
        glm::vec4 hitColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.8f);  // Red sparks
        m_particleSystem->spawnBurst(15, hitPos, 0.3f, 0.5f, hitColor);  // Hit burst
    }
    // --- End FLAMMIL Ember Particles --- 

    // --- Spawn particles based on SUCCESSFUL Combo Move processing --- 
    if (moveProcessed && m_particleSystem) {
         // If a move was processed AND resulted in a valid combo state 
         glm::vec3 spawnPos = glm::vec3(0.0f, 1.0f, -2.0f); // Fixed spawn point IN FRONT of origin
        // TODO: Replace with actual collision point from hitbox check
        glm::vec4 sparkColor = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f); // Yellow/orange spark
        float initialSpeed = 5.0f;
        float lifetime = 0.5f;
        int count = 20;
        std::cout << "    Spawning HIT particle burst for move: " << currentComboMove << std::endl;
        m_particleSystem->spawnBurst(count, spawnPos, initialSpeed, lifetime, sparkColor);
    }
    // --- End Hit Particle Spawn ---
}

void Engine::setCSLSystem(CSL::CSLSystem* sys) {
    // Correctly manage ownership with unique_ptr
    m_cslSystem.reset(sys); 
}

void Engine::run() {
    while (m_isRunning && !glfwWindowShouldClose(m_window)) {
        auto frameStart = std::chrono::high_resolution_clock::now();

        // Frame time calculation
        double currentTime = glfwGetTime();
        m_performance.deltaTime = currentTime - m_performance.lastFrameTime;
        m_performance.lastFrameTime = currentTime;
        double currentFrameTime = m_performance.deltaTime; // Use calculated delta time for updates

        processInput();

        // --- Start CSL Update Timing ---
        auto cslStart = std::chrono::high_resolution_clock::now();
        if (m_cslSystem) {
            m_cslSystem->update(); // Assuming this handles frame grabbing internally or uses m_currentFrame
        }
        auto cslEnd = std::chrono::high_resolution_clock::now();
        std::cout << "CSL Update: " << std::chrono::duration_cast<std::chrono::microseconds>(cslEnd - cslStart).count() << " us" << std::endl;
        // --- End CSL Update Timing ---

        // --- Start Particle Update Timing ---
        auto particleUpdateStart = std::chrono::high_resolution_clock::now();
        if (m_particleSystem) {
            m_particleSystem->update(static_cast<float>(currentFrameTime)); // Use calculated delta time
        }
        auto particleUpdateEnd = std::chrono::high_resolution_clock::now();
        std::cout << "Particle Update: " << std::chrono::duration_cast<std::chrono::microseconds>(particleUpdateEnd - particleUpdateStart).count() << " us" << std::endl;
        // --- End Particle Update Timing ---

        updateCamera(); // Assuming this uses member variables, not deltaTime directly

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark grey background (Restored)
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        std::cout << "Cleared to dark grey" << std::endl; // Debug log for clear

        // Calculate view and projection matrices
        glm::mat4 view = glm::lookAt(m_camera.position, m_camera.target, m_camera.up);
        int fbWidth, fbHeight;
        glfwGetFramebufferSize(m_window, &fbWidth, &fbHeight);
        float aspectRatio = (fbHeight > 0) ? static_cast<float>(fbWidth) / fbHeight : 1.0f;
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), aspectRatio, 0.1f, 100.0f);

        // Render grid if available
        if (m_grid) {
            m_grid->render(view, projection);
        }

        // --- Start Particle Render Timing ---
        auto particleRenderStart = std::chrono::high_resolution_clock::now();
        if (m_particleSystem) {
            m_particleSystem->render(view, projection);
        }
        auto particleRenderEnd = std::chrono::high_resolution_clock::now();
        std::cout << "Particle Render: " << std::chrono::duration_cast<std::chrono::microseconds>(particleRenderEnd - particleRenderStart).count() << " us" << std::endl;
        // --- End Particle Render Timing ---

        auto frameEnd = std::chrono::high_resolution_clock::now();
        std::cout << "Frame Total: " << std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count() << " us" << std::endl;

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