#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <glm/glm.hpp>
#include "../engine/temporal/include/TemporalAnomalySystem.hpp"
#include "../engine/temporal/include/TemporalCollisionHandler.hpp"

using namespace TurtleEngine::Temporal;

// Test entity implementation - similar to the one used in TemporalAnomalyTest
class TestEntity : public AffectedEntity {
public:
    TestEntity(const std::string& id, const glm::vec3& position)
        : m_id(id), m_position(position), m_timeScale(1.0f), m_velocity(0.0f), m_inAnomaly(false) {
        std::cout << "Created entity " << m_id << " at " << 
            m_position.x << ", " << m_position.y << ", " << m_position.z << std::endl;
    }
    
    void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) override {
        m_timeScale = effect.timeDistortion;
        m_inAnomaly = true;
        
        // Apply effect based on anomaly type
        switch (effect.type) {
            case AnomalyType::STASIS:
                m_velocity *= 0.1f; // Almost stopped
                break;
                
            case AnomalyType::DILATION:
                m_velocity *= 0.5f; // Slowed
                break;
                
            case AnomalyType::ACCELERATION:
                m_velocity *= 2.0f; // Accelerated
                break;
                
            case AnomalyType::REVERSION:
                m_velocity *= -1.0f; // Reversed
                break;
                
            case AnomalyType::RIFT:
                // Random effect for rifts using C++ random functionality
                static std::random_device rd;
                static std::mt19937 gen(rd());
                static std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
                
                m_velocity = glm::vec3(
                    dist(gen),
                    dist(gen),
                    dist(gen)
                );
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
        
        // Reset for next frame
        m_timeScale = 1.0f;
        m_inAnomaly = false;
    }
    
    void setVelocity(const glm::vec3& velocity) {
        m_velocity = velocity;
    }
    
    void setPosition(const glm::vec3& position) {
        m_position = position;
    }
    
    bool isInAnomaly() const {
        return m_inAnomaly;
    }
    
private:
    std::string m_id;
    glm::vec3 m_position;
    float m_timeScale;
    glm::vec3 m_velocity;
    bool m_inAnomaly;
};

int main() {
    std::cout << "=== Temporal Collision Handler Test ===" << std::endl;
    
    // Create the temporal anomaly system
    auto anomalySystem = std::make_shared<TemporalAnomalySystem>();
    
    // Create the collision handler
    auto collisionHandler = std::make_shared<TemporalCollisionHandler>(anomalySystem);
    collisionHandler->initialize();
    
    // Set up collision event callbacks
    collisionHandler->setEntityEnterCallback(
        [](const std::string& entityId, const TemporalAnomaly& anomaly) {
            std::cout << "COLLISION EVENT: Entity " << entityId << " entered a " 
                      << static_cast<int>(anomaly.getType()) << " anomaly!" << std::endl;
        }
    );
    
    collisionHandler->setEntityExitCallback(
        [](const std::string& entityId, const TemporalAnomaly& anomaly) {
            std::cout << "COLLISION EVENT: Entity " << entityId << " exited a " 
                      << static_cast<int>(anomaly.getType()) << " anomaly!" << std::endl;
        }
    );
    
    collisionHandler->setAnomaliesOverlapCallback(
        [](const TemporalAnomaly& anomaly1, const TemporalAnomaly& anomaly2) {
            std::cout << "COLLISION EVENT: Anomaly type " << static_cast<int>(anomaly1.getType()) 
                      << " overlapped with anomaly type " << static_cast<int>(anomaly2.getType()) 
                      << std::endl;
        }
    );
    
    // Create test entities
    std::vector<std::shared_ptr<TestEntity>> entities;
    
    entities.push_back(std::make_shared<TestEntity>("Player", glm::vec3(0.0f, 0.0f, 0.0f)));
    entities.push_back(std::make_shared<TestEntity>("Enemy1", glm::vec3(5.0f, 0.0f, 0.0f)));
    entities.push_back(std::make_shared<TestEntity>("Enemy2", glm::vec3(-5.0f, 0.0f, 0.0f)));
    entities.push_back(std::make_shared<TestEntity>("Enemy3", glm::vec3(0.0f, 0.0f, 5.0f)));
    
    // Register entities with the anomaly system
    for (auto& entity : entities) {
        anomalySystem->registerEntity(entity);
    }
    
    // Create initial anomalies
    std::cout << "\n=== Creating initial anomalies ===" << std::endl;
    
    // Create a stasis field
    AnomalyEffect stasisEffect(
        AnomalyType::STASIS,
        0.1f,                // Strong time slowdown
        glm::vec3(3.0f, 0.0f, 0.0f), // Near Enemy1
        2.0f,                // Small radius
        10.0f,               // 10 second duration
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
    
    // Run simulation
    std::cout << "\n=== Running simulation ===" << std::endl;
    
    const float deltaTime = 0.5f; // Half-second time steps for demonstration
    const float totalTime = 15.0f; // 15 second simulation
    float currentTime = 0.0f;
    
    while (currentTime < totalTime) {
        // Print current time
        std::cout << "\n--- Time: " << currentTime << "s ---" << std::endl;
        
        // Update anomaly system and collision handler
        anomalySystem->update(deltaTime);
        collisionHandler->update(deltaTime);
        
        // Update entities
        for (auto& entity : entities) {
            entity->update(deltaTime);
        }
        
        // Test collision handler queries
        for (auto& entity : entities) {
            auto affectingAnomalies = collisionHandler->getAnomaliesAffectingEntity(entity->getIdentifier());
            std::cout << entity->getIdentifier() << " is affected by " 
                      << affectingAnomalies.size() << " anomalies" << std::endl;
            
            float distortion = collisionHandler->getEntityTimeDistortion(entity->getIdentifier());
            std::cout << entity->getIdentifier() << " time distortion: " << distortion << std::endl;
        }
        
        // Create testing scenarios
        if (currentTime == 2.0f) {
            // Move Player entity toward stasis field
            std::cout << "\n=== Moving Player toward Stasis Field ===" << std::endl;
            entities[0]->setPosition(glm::vec3(2.0f, 0.0f, 0.0f));
        }
        else if (currentTime == 5.0f) {
            // Move Player entity away from stasis field
            std::cout << "\n=== Moving Player away from Stasis Field ===" << std::endl;
            entities[0]->setPosition(glm::vec3(0.0f, 0.0f, 0.0f));
            
            // Create overlapping anomaly with the stasis field
            std::cout << "\n=== Creating overlapping anomaly ===" << std::endl;
            AnomalyEffect overlappingEffect(
                AnomalyType::ACCELERATION,
                2.0f,                 // Speedup
                glm::vec3(4.0f, 0.0f, 0.0f), // Near stasis field
                2.5f,                 // Radius to create overlap
                5.0f,                 // 5 second duration
                "OverlapTest"
            );
            anomalySystem->createAnomaly(overlappingEffect);
        }
        else if (currentTime == 10.0f) {
            // Create a moving entity that will pass through anomalies
            std::cout << "\n=== Creating moving entity to pass through anomalies ===" << std::endl;
            auto movingEntity = std::make_shared<TestEntity>("MovingProbe", glm::vec3(-10.0f, 0.0f, 0.0f));
            movingEntity->setVelocity(glm::vec3(2.0f, 0.0f, 0.0f)); // Move along X axis
            entities.push_back(movingEntity);
            anomalySystem->registerEntity(movingEntity);
        }
        
        // Add time
        currentTime += deltaTime;
        
        // Pause for clarity in output
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "\n=== Test complete ===" << std::endl;
    
    return 0;
} 