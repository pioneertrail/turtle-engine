#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include "../engine/temporal/include/TemporalAnomalySystem.hpp"

using namespace TurtleEngine::Temporal;

// Simple test entity implementation
class TestEntity : public AffectedEntity {
public:
    TestEntity(const std::string& id, const glm::vec3& position)
        : m_id(id), m_position(position), m_timeScale(1.0f), m_velocity(0.0f) {
        std::cout << "Created entity " << m_id << " at " << 
            m_position.x << ", " << m_position.y << ", " << m_position.z << std::endl;
    }
    
    void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) override {
        m_timeScale = effect.timeDistortion;
        
        std::cout << "Entity " << m_id << " affected by temporal anomaly. " <<
            "Time scale: " << m_timeScale << std::endl;
        
        // Apply effect based on anomaly type
        switch (effect.type) {
            case AnomalyType::STASIS:
                m_velocity *= 0.1f; // Almost stopped
                std::cout << "  Stasis effect: Movement slowed to near-stop" << std::endl;
                break;
                
            case AnomalyType::DILATION:
                m_velocity *= 0.5f; // Slowed
                std::cout << "  Dilation effect: Movement slowed" << std::endl;
                break;
                
            case AnomalyType::ACCELERATION:
                m_velocity *= 2.0f; // Accelerated
                std::cout << "  Acceleration effect: Movement accelerated" << std::endl;
                break;
                
            case AnomalyType::REVERSION:
                m_velocity *= -1.0f; // Reversed
                std::cout << "  Reversion effect: Movement reversed" << std::endl;
                break;
                
            case AnomalyType::RIFT:
                // Random effect for rifts
                m_velocity = glm::vec3(
                    glm::linearRand(-1.0f, 1.0f),
                    glm::linearRand(-1.0f, 1.0f),
                    glm::linearRand(-1.0f, 1.0f)
                );
                std::cout << "  Rift effect: Movement randomized" << std::endl;
                break;
        }
    }
    
    glm::vec3 getPosition() const override {
        return m_position;
    }
    
    std::string getIdentifier() const override {
        return m_id;
    }
    
    void update(float deltaTime) {
        // Apply velocity (modified by temporal effects)
        m_position += m_velocity * deltaTime * m_timeScale;
        
        // Reset time scale for next frame
        m_timeScale = 1.0f;
    }
    
    void setVelocity(const glm::vec3& velocity) {
        m_velocity = velocity;
    }
    
private:
    std::string m_id;
    glm::vec3 m_position;
    float m_timeScale;
    glm::vec3 m_velocity;
};

int main() {
    std::cout << "=== Temporal Anomaly Framework Test ===" << std::endl;
    
    // Create the temporal anomaly system
    auto anomalySystem = std::make_shared<TemporalAnomalySystem>();
    
    // Create some test entities
    std::vector<std::shared_ptr<TestEntity>> entities;
    
    entities.push_back(std::make_shared<TestEntity>("Player", glm::vec3(0.0f, 0.0f, 0.0f)));
    entities.push_back(std::make_shared<TestEntity>("Enemy1", glm::vec3(5.0f, 0.0f, 0.0f)));
    entities.push_back(std::make_shared<TestEntity>("Enemy2", glm::vec3(-5.0f, 0.0f, 0.0f)));
    entities.push_back(std::make_shared<TestEntity>("Enemy3", glm::vec3(0.0f, 0.0f, 5.0f)));
    
    // Set initial velocities
    entities[0]->setVelocity(glm::vec3(0.0f, 0.0f, 1.0f)); // Player moving forward
    entities[1]->setVelocity(glm::vec3(-1.0f, 0.0f, 0.0f)); // Enemy1 moving left
    entities[2]->setVelocity(glm::vec3(1.0f, 0.0f, 0.0f));  // Enemy2 moving right
    entities[3]->setVelocity(glm::vec3(0.0f, 0.0f, -1.0f)); // Enemy3 moving backward
    
    // Register entities with the anomaly system
    for (auto& entity : entities) {
        anomalySystem->registerEntity(entity);
    }
    
    // Test different anomaly types
    std::cout << "\n=== Creating temporal anomalies ===" << std::endl;
    
    // Create a stasis field
    AnomalyEffect stasisEffect(
        AnomalyType::STASIS,
        0.1f,                // Strong time slowdown
        glm::vec3(3.0f, 0.0f, 0.0f), // Near Enemy1
        2.0f,                // Small radius
        5.0f,                // 5 second duration
        "StasisTest"
    );
    anomalySystem->createAnomaly(stasisEffect);
    
    // Create a time dilation field
    AnomalyEffect dilationEffect(
        AnomalyType::DILATION,
        0.5f,                // Moderate time slowdown
        glm::vec3(-3.0f, 0.0f, 0.0f), // Near Enemy2
        3.0f,                // Medium radius
        10.0f,               // 10 second duration
        "DilationTest"
    );
    anomalySystem->createAnomaly(dilationEffect);
    
    // Create an acceleration field
    AnomalyEffect accelerationEffect(
        AnomalyType::ACCELERATION,
        2.0f,                // Moderate time speedup
        glm::vec3(0.0f, 0.0f, 3.0f), // Near Enemy3
        2.5f,                // Medium radius
        7.0f,                // 7 second duration
        "AccelerationTest"
    );
    anomalySystem->createAnomaly(accelerationEffect);
    
    // Create a rift (near player)
    AnomalyEffect riftEffect(
        AnomalyType::RIFT,
        1.5f,                // Chaotic time distortion
        glm::vec3(0.0f, 0.0f, 0.0f), // At player position
        4.0f,                // Large radius
        15.0f,               // 15 second duration
        "RiftTest"
    );
    anomalySystem->createAnomaly(riftEffect);
    
    // Simulation loop
    std::cout << "\n=== Running simulation ===" << std::endl;
    
    const float deltaTime = 0.5f; // Half-second time steps for demonstration
    const float totalTime = 10.0f; // 10 second simulation
    float currentTime = 0.0f;
    
    while (currentTime < totalTime) {
        // Print current time
        std::cout << "\n--- Time: " << currentTime << "s ---" << std::endl;
        
        // Update anomaly system
        anomalySystem->update(deltaTime);
        
        // Update entities
        for (auto& entity : entities) {
            entity->update(deltaTime);
        }
        
        // Show entity positions
        for (auto& entity : entities) {
            glm::vec3 pos = entity->getPosition();
            std::cout << entity->getIdentifier() << " position: " 
                      << pos.x << ", " << pos.y << ", " << pos.z << std::endl;
        }
        
        // Add time
        currentTime += deltaTime;
        
        // Create a reversion field halfway through
        if (std::abs(currentTime - totalTime / 2.0f) < deltaTime / 2.0f) {
            std::cout << "\n=== Creating reversion field ===" << std::endl;
            AnomalyEffect reversionEffect(
                AnomalyType::REVERSION,
                -0.5f,               // Time reversal
                entities[0]->getPosition(), // At player's current position
                5.0f,                // Large radius
                5.0f,                // 5 second duration
                "ReversionTest"
            );
            anomalySystem->createAnomaly(reversionEffect);
        }
        
        // Pause for clarity in output
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "\n=== Test complete ===" << std::endl;
    
    return 0;
} 