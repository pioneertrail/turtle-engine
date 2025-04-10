#define GLEW_STATIC
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

    // --- Flammyx Setup Start ---
    // Load Flammyx shader
    if (!m_flammyxShader.loadFromFiles("shaders/flammyx.vert", "shaders/flammyx.frag")) {
        std::cerr << "Failed to load Flammyx shader!" << std::endl;
        // Optionally return false or handle error
    }

    // Generate VAO and VBO for Flammyx trail
    glGenVertexArrays(1, &m_flammyxVao);
    glGenBuffers(1, &m_flammyxVbo);

    glBindVertexArray(m_flammyxVao);
    glBindBuffer(GL_ARRAY_BUFFER, m_flammyxVbo);

    // Configure vertex attributes (position + velocity)
    // Position (vec3) at location 0
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Velocity (float) at location 1
    glVertexAttribPointer(1, 1, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
    // --- Flammyx Setup End ---

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
                    if (engine->m_cslSystem) {
                        std::cout << "[Input] F key pressed, triggering Flammil." << std::endl;
                        // Corrected: Call with only one argument
                        engine->m_cslSystem->triggerGesture(CSL::GestureType::FLAMMIL);
                    }
                    break;
                case GLFW_KEY_C: // Trigger Khargail
                    if (engine->m_cslSystem) {
                        std::cout << "[Input] C key pressed, triggering Khargail." << std::endl;
                        // Corrected: Call with only one argument
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

    // --- Latency Check for Triggered Gestures --- 
    if (result.triggerTimestamp.has_value()) {
        auto callbackTime = std::chrono::high_resolution_clock::now();
        auto totalLatency = std::chrono::duration_cast<std::chrono::microseconds>(callbackTime - result.triggerTimestamp.value());
        std::cout << "    QuickSign Latency (Key Press -> Callback): " << totalLatency.count() << " us" << std::endl;
        // TODO: Log this to gesture_recognition_results.txt as required by task
        // TODO: Check against <0.5s requirement from task list
        if (totalLatency.count() > 500000) { // 0.5 seconds
            std::cerr << "!!!! WARNING: QuickSign latency > 500ms !!!!" << std::endl;
        }
    }
    // --- End Latency Check ---

    // --- TEMPORARY: Spawn particles on gesture --- 
    if (m_particleSystem && result.type != CSL::GestureType::NONE) {
        glm::vec3 spawnPos = glm::vec3(0.0f, 1.0f, 0.0f); // Fixed spawn point for now
        // Later, this position should come from hitbox collision point
        glm::vec4 sparkColor = glm::vec4(1.0f, 0.8f, 0.2f, 1.0f); // Yellow/orange spark
        float initialSpeed = 5.0f;
        float lifetime = 0.5f;
        int count = 20; // Number of particles per spark
        std::cout << "    Spawning particle burst at (" << spawnPos.x << "," << spawnPos.y << "," << spawnPos.z << ")" << std::endl;
        m_particleSystem->spawnBurst(count, spawnPos, initialSpeed, lifetime, sparkColor);
    }
    // --- End Temporary Spawn ---

    // Process the move via ComboManager if a valid gesture name was found
    if (m_comboManager && gestureName != "UNKNOWN" && gestureName != "None") {
        // Timing for ComboManager processing itself
        auto start_time = std::chrono::high_resolution_clock::now();
        m_comboManager->ProcessMove(gestureName);
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
        std::cout << "    ComboManager::ProcessMove took: " << duration.count() << " us" << std::endl;
        
        if (duration.count() > 100000) {
            std::cerr << "!!!! WARNING: Combo processing took longer than 100ms !!!!" << std::endl;
        }
    }
}

void Engine::setCSLSystem(CSL::CSLSystem* sys) {
    // Correctly manage ownership with unique_ptr
    m_cslSystem.reset(sys); 
    if (m_cslSystem) {
        // Register the callback
        m_cslSystem->addPlasmaCallback([this](const CSL::GestureResult& result) {
            this->handleFlammyxGesture(result);
        });
        std::cout << "Engine: Registered Flammyx callback with CSLSystem." << std::endl;
    }
}

void Engine::handleFlammyxGesture(const CSL::GestureResult& result) {
    // Ensure this runs safely if called from CSL thread
    std::lock_guard<std::mutex> lock(m_flammyxMutex);

    if (result.type == CSL::GestureType::FLAMMIL && !result.trajectory.empty() && result.trajectory.size() == result.velocities.size()) {
        std::cout << "Engine: Received Flammil gesture for rendering. Points: " << result.trajectory.size() << std::endl;
        std::vector<float> vertexData;
        vertexData.reserve(result.trajectory.size() * 4); // 3 pos + 1 vel

        int windowWidth, windowHeight;
        glfwGetFramebufferSize(m_window, &windowWidth, &windowHeight);

        for (size_t i = 0; i < result.trajectory.size(); ++i) {
            float ndcX = (result.trajectory[i].x / static_cast<float>(windowWidth)) * 2.0f - 1.0f;
            float ndcY = 1.0f - (result.trajectory[i].y / static_cast<float>(windowHeight)) * 2.0f; // Invert Y for OpenGL coords

            // Clamp coordinates to NDC range [-1, 1]
            ndcX = glm::clamp(ndcX, -1.0f, 1.0f);
            ndcY = glm::clamp(ndcY, -1.0f, 1.0f);

            vertexData.push_back(ndcX); // Clamped X
            vertexData.push_back(ndcY); // Clamped Y
            vertexData.push_back(0.0f);                    // z (assuming 2D for now)
            vertexData.push_back(result.velocities[i]);    // velocity
        }

        // Update VBO
        glBindBuffer(GL_ARRAY_BUFFER, m_flammyxVbo);
        // Use glBufferData to resize and upload new data (GL_DYNAMIC_DRAW hints usage pattern)
        glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_DYNAMIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        m_flammyxPointCount = result.trajectory.size();
        // Set duration based on CSLSystem setting (if available and valid)
        if (m_cslSystem) {
             m_flammyxDuration = m_cslSystem->getPlasmaDuration();
        } else {
             m_flammyxDuration = 0.5f; // Default duration if CSL system not set
        }
    }
}

void Engine::run() {
    double lastTime = glfwGetTime(); // For delta time calculation

    while (m_isRunning && !glfwWindowShouldClose(m_window)) {
        // Calculate frame time / delta time
        double currentTime = glfwGetTime();
        double deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        
        m_performance.frameTime = deltaTime;
        m_performance.fps = static_cast<int>(1.0 / deltaTime);

        processInput();
        updateCamera();

        // Update CSL System (if set)
        if (m_cslSystem) {
            m_cslSystem->update();
        }

        // Update Particle System
        if (m_particleSystem) {
            m_particleSystem->update(static_cast<float>(deltaTime));
        }

        // Clear buffers
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f); // Dark grey background
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Create view and projection matrices
        glm::mat4 view = glm::lookAt(m_camera.position, m_camera.target, m_camera.up);
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f/600.0f, 0.1f, 100.0f);
        glm::mat4 mvp = projection * view; // Calculate MVP once

        // Render grid
        m_grid->render(view, projection);

        // Render particles
        if (m_particleSystem) {
            m_particleSystem->render(view, projection);
        }

        // --- Render Flammyx Trail Start ---
        {
            std::lock_guard<std::mutex> lock(m_flammyxMutex);
            if (m_flammyxPointCount > 0) {
                m_flammyxShader.use();
                m_flammyxShader.setMat4("MVP", mvp);
                
                glBindVertexArray(m_flammyxVao);
                glDrawArrays(GL_LINE_STRIP, 0, m_flammyxPointCount);
                glBindVertexArray(0);
                
                // Update duration and potentially clear points
                m_flammyxDuration -= static_cast<float>(deltaTime);
                if (m_flammyxDuration <= 0.0f) {
                    m_flammyxPointCount = 0; // Trail fades
                }
            }
        }
        // --- Render Flammyx Trail End ---

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

    // --- Flammyx Cleanup Start ---
    if (m_flammyxVao != 0) {
        glDeleteVertexArrays(1, &m_flammyxVao);
        m_flammyxVao = 0;
    }
    if (m_flammyxVbo != 0) {
        glDeleteBuffers(1, &m_flammyxVbo);
        m_flammyxVbo = 0;
    }
    // Shader object is cleaned up by its destructor
    // --- Flammyx Cleanup End ---

    if (m_window) {
        glfwDestroyWindow(m_window);
        m_window = nullptr;
    }
    glfwTerminate();
    m_isRunning = false;
}

} // namespace TurtleEngine 