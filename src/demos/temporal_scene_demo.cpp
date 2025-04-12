#include <iostream>
#include "engine/include/Engine.hpp"
#include "engine/include/Window.hpp"
#include "engine/include/Renderer.hpp"
#include "engine/include/ParticleSystem.hpp"
#include "engine/include/Shader.hpp"
#include "engine/include/InputManager.hpp"
#include "engine/include/utils/Logger.hpp"
#include "engine/temporal/include/TemporalAnomalySystem.hpp"

using namespace TurtleEngine;

// Custom scene demo using TurtleEngine
class TemporalSceneDemo {
private:
    Engine* engine;
    TemporalAnomalySystem temporalSystem;
    bool running;
    
    // Scene objects
    struct SceneObject {
        glm::vec3 position;
        glm::vec3 velocity;
        glm::vec4 color;
        float temporalCharge;
        bool affectedByTime;
    };
    
    std::vector<SceneObject> objects;
    
public:
    TemporalSceneDemo() : engine(nullptr), running(false) {
        // Initialize logger first
        Utils::Logger::init(Utils::LogLevel::DEBUG);
        LOG_INFO("Initializing Temporal Scene Demo");
    }
    
    ~TemporalSceneDemo() {
        if (engine) {
            delete engine;
        }
    }
    
    bool initialize() {
        // Create and initialize the engine
        engine = new Engine();
        if (!engine->initialize("Temporal Scene Demo", 1280, 720)) {
            LOG_ERROR("Failed to initialize engine");
            return false;
        }
        
        // Initialize temporal system
        temporalSystem.initialize();
        
        // Create scene objects
        createScene();
        
        running = true;
        return true;
    }
    
    void createScene() {
        LOG_INFO("Creating scene objects");
        
        // Regular objects (affected by time)
        for (int i = 0; i < 10; i++) {
            float angle = (i / 10.0f) * 3.14159f * 2.0f;
            SceneObject obj;
            obj.position = glm::vec3(
                cosf(angle) * 3.0f,
                0.0f,
                sinf(angle) * 3.0f
            );
            obj.velocity = glm::vec3(
                cosf(angle + 1.57f) * 0.5f,
                0.0f,
                sinf(angle + 1.57f) * 0.5f
            );
            obj.color = glm::vec4(0.7f, 0.2f, 0.2f, 1.0f); // Red
            obj.temporalCharge = 0.0f;
            obj.affectedByTime = true;
            objects.push_back(obj);
        }
        
        // Temporal anomaly objects (not affected by normal time)
        for (int i = 0; i < 5; i++) {
            float angle = (i / 5.0f) * 3.14159f * 2.0f;
            SceneObject obj;
            obj.position = glm::vec3(
                cosf(angle) * 5.0f,
                1.0f,
                sinf(angle) * 5.0f
            );
            obj.velocity = glm::vec3(
                cosf(angle + 1.57f) * 0.2f,
                0.0f,
                sinf(angle + 1.57f) * 0.2f
            );
            obj.color = glm::vec4(0.2f, 0.2f, 0.8f, 1.0f); // Blue
            obj.temporalCharge = 1.0f;
            obj.affectedByTime = false;
            objects.push_back(obj);
        }
        
        // Add a temporal anomaly in the center
        temporalSystem.createAnomaly(
            glm::vec3(0.0f, 0.0f, 0.0f), // position
            2.0f,                         // radius
            0.8f,                         // intensity
            5.0f,                         // duration
            AnomalyType::TIME_SLOW        // Updated to use the correct namespace
        );
        
        // Set up particle effects
        ParticleSystem* particleSystem = engine->getParticleSystem();
        particleSystem->setGravity(glm::vec3(0.0f, -0.1f, 0.0f));
        
        // Create particle emitter at the center for the anomaly
        particleSystem->createEmitter(
            glm::vec3(0.0f, 0.0f, 0.0f),  // position
            glm::vec3(0.0f, 1.0f, 0.0f),  // direction
            30.0f,                         // spread angle
            50,                           // particles per second
            1.5f,                         // particle life
            0.5f,                         // size
            glm::vec4(0.0f, 0.7f, 1.0f, 0.5f), // start color
            glm::vec4(0.0f, 0.3f, 0.8f, 0.0f)  // end color
        );
    }
    
    void run() {
        LOG_INFO("Starting main loop");
        
        while (running) {
            // Get delta time
            float deltaTime = 0.016f; // Approx 60 FPS
            
            // Update temporal system
            temporalSystem.update(deltaTime);
            
            // Update all objects
            updateObjects(deltaTime);
            
            // Process input
            processInput();
            
            // Render the scene
            render();
            
            // Check if window should close
            if (engine->getWindow()->shouldClose()) {
                running = false;
            }
        }
    }
    
    void updateObjects(float deltaTime) {
        // Get anomalies from the temporal system
        auto anomalies = temporalSystem.getActiveAnomalies();
        
        // Update each object
        for (auto& obj : objects) {
            // Calculate time dilation factor based on anomalies
            float timeFactor = 1.0f;
            float strongestInfluence = 0.0f;
            if (obj.affectedByTime) {
                for (const auto& anomaly : anomalies) {
                    float distance = glm::distance(obj.position, anomaly.position);
                    if (distance < anomaly.radius) {
                        float influence = 1.0f - (distance / anomaly.radius);
                        strongestInfluence = std::max(strongestInfluence, influence);
                        
                        // Apply different effects based on anomaly type
                        if (anomaly.type == Temporal::AnomalyType::TIME_SLOW) {
                            timeFactor *= (1.0f - influence * anomaly.intensity);
                        } else if (anomaly.type == Temporal::AnomalyType::TIME_FAST) {
                            timeFactor *= (1.0f + influence * anomaly.intensity);
                        }
                    }
                }
            }
            
            // Apply velocity with time dilation
            obj.position += obj.velocity * deltaTime * timeFactor;
            
            // Visualize time dilation through color
            if (obj.affectedByTime) {
                // Shift color based on time factor (redder when slower, bluer when faster)
                obj.color.r = glm::mix(0.2f, 0.8f, 1.0f - timeFactor);
                obj.color.b = glm::mix(0.2f, 0.8f, timeFactor);
            }
            
            // Add particles for objects affected by time anomalies
            if (obj.affectedByTime && timeFactor < 0.5f) {
                // Calculate particle parameters based on time distortion intensity
                float intensity = std::abs(timeFactor - 1.0f); // Proxy for effect strength
                int count = static_cast<int>(5 + intensity * 10);
                float life = 0.5f + intensity * 0.2f;
                
                // Emit temporal particles with dynamic parameters
                engine->getParticleSystem()->emitParticles(
                    obj.position,                      // position
                    count,                             // count - now scales with intensity
                    life,                              // life - now scales with intensity
                    glm::vec4(0.0f, 0.5f, 1.0f, 0.3f)  // color
                );
            }
        }
    }
    
    void processInput() {
        InputManager* input = engine->getInputManager();
        
        // Camera controls
        if (input->isKeyPressed(GLFW_KEY_W)) {
            // Move camera forward
        }
        if (input->isKeyPressed(GLFW_KEY_S)) {
            // Move camera backward
        }
        if (input->isKeyPressed(GLFW_KEY_A)) {
            // Move camera left
        }
        if (input->isKeyPressed(GLFW_KEY_D)) {
            // Move camera right
        }
        
        // Create new temporal anomaly on mouse click
        if (input->isMouseButtonJustPressed(GLFW_MOUSE_BUTTON_LEFT)) {
            // Get mouse position and convert to world position
            glm::vec2 mousePos = engine->getWindow()->getMousePosition();
            
            // TODO: Convert mouse position to world position
            // For now, just create at a random position
            float x = (rand() % 100) / 10.0f - 5.0f;
            float z = (rand() % 100) / 10.0f - 5.0f;
            
            temporalSystem.createAnomaly(
                glm::vec3(x, 0.0f, z),
                1.5f,
                0.7f,
                3.0f,
                AnomalyType::TIME_FAST  // Updated to use the correct namespace
            );
            
            LOG_INFO("Created new temporal anomaly at ({}, 0, {})", x, z);
        }
        
        // Exit on escape
        if (input->isKeyJustPressed(GLFW_KEY_ESCAPE)) {
            running = false;
        }
    }
    
    void render() {
        Renderer* renderer = engine->getRenderer();
        
        // Clear the screen
        renderer->clear();
        
        // Draw ground grid
        renderer->drawGrid(10, 1.0f, glm::vec4(0.3f, 0.3f, 0.3f, 1.0f));
        
        // Draw objects
        for (const auto& obj : objects) {
            renderer->drawSphere(
                obj.position,     // position
                0.3f,             // radius
                obj.color         // color
            );
        }
        
        // Draw temporal anomalies
        auto anomalies = temporalSystem.getActiveAnomalies();
        for (const auto& anomaly : anomalies) {
            glm::vec4 anomalyColor;
            
            // Color based on anomaly type
            if (anomaly.type == AnomalyType::TIME_SLOW) {  // Updated to use the correct namespace
                anomalyColor = glm::vec4(0.0f, 0.5f, 1.0f, 0.3f); // Blue for slow
            } else {
                anomalyColor = glm::vec4(1.0f, 0.5f, 0.0f, 0.3f); // Orange for fast
            }
            
            // Draw sphere representing the anomaly
            renderer->drawTransparentSphere(
                anomaly.position,
                anomaly.radius,
                anomalyColor
            );
        }
        
        // Update and draw particles
        engine->getParticleSystem()->render(renderer);
        
        // Swap buffers
        engine->getWindow()->swapBuffers();
        engine->getWindow()->pollEvents();
    }
};

int main() {
    TemporalSceneDemo demo;
    
    if (demo.initialize()) {
        demo.run();
    } else {
        std::cerr << "Failed to initialize demo" << std::endl;
        return -1;
    }
    
    return 0;
} 