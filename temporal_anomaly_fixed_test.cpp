#include <iostream>
#include <memory>
#include <glm/glm.hpp>

namespace TurtleEngine {
namespace Temporal {

// Forward declarations to create a standalone test
enum class AnomalyType {
    RIFT,       // Spatial discontinuity in the timestream
    STASIS,     // Time freeze effect
    DILATION,   // Time slowdown effect
    ACCELERATION,// Time speedup effect
    REVERSION   // Time reversal effect
};

struct AnomalyEffect {
    AnomalyType type;
    float timeDistortion;
    glm::vec3 position;
    float radius;
    float duration;
    std::string sourceIdentifier;
    
    AnomalyEffect(
        AnomalyType anomalyType = AnomalyType::RIFT,
        float distortionFactor = 0.5f,
        const glm::vec3& pos = glm::vec3(0.0f),
        float rad = 5.0f,
        float dur = 10.0f,
        const std::string& identifier = ""
    ) : type(anomalyType), timeDistortion(distortionFactor), position(pos), 
        radius(rad), duration(dur), sourceIdentifier(identifier) {}
};

// Interface for entities affected by temporal anomalies
class AffectedEntity {
public:
    virtual ~AffectedEntity() = default;
    virtual void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) = 0;
    virtual glm::vec3 getPosition() const = 0;
    virtual std::string getIdentifier() const = 0;
};

// Test entity implementation
class TestEntity : public AffectedEntity {
public:
    TestEntity(const std::string& id, const glm::vec3& position)
        : m_id(id), m_position(position), m_timeScale(1.0f) {
        std::cout << "Created entity " << m_id << " at position (" 
                 << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }

    void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) override {
        // Critical fix: directly apply the time distortion factor based on anomaly type
        switch (effect.type) {
            case AnomalyType::DILATION:
                // For dilation, timeScale should be reduced
                m_timeScale = effect.timeDistortion;
                break;
            case AnomalyType::STASIS:
                // For stasis, time almost stops
                m_timeScale = effect.timeDistortion;
                break;
            case AnomalyType::ACCELERATION:
                // For acceleration, timeScale should increase
                m_timeScale = effect.timeDistortion;
                break;
            case AnomalyType::REVERSION:
                // For reversion, timeScale becomes negative
                m_timeScale = -effect.timeDistortion;
                break;
            case AnomalyType::RIFT:
                // For rift, unpredictable time distortion
                m_timeScale = effect.timeDistortion * (0.5f + std::sin(deltaTime * 10.0f) * 0.5f);
                break;
        }
        
        std::cout << "Entity " << m_id << " affected by temporal anomaly type " 
                 << static_cast<int>(effect.type) << ". Time scale now: " << m_timeScale << std::endl;
    }

    void update(float deltaTime) {
        // Simulate entity update with time scale applied
        float adjustedDelta = deltaTime * m_timeScale;
        std::cout << "Entity " << m_id << " updated with time scale " << m_timeScale 
                 << " (adjusted delta: " << adjustedDelta << ")" << std::endl;
    }

    glm::vec3 getPosition() const override { return m_position; }
    std::string getIdentifier() const override { return m_id; }
    float getTimeScale() const { return m_timeScale; }
    
    void setPosition(const glm::vec3& position) { m_position = position; }

private:
    std::string m_id;
    glm::vec3 m_position;
    float m_timeScale;
};

// Simplified temporal anomaly class for testing
class TemporalAnomaly {
public:
    TemporalAnomaly(const AnomalyEffect& effect)
        : m_effect(effect), 
          m_remainingDuration(effect.duration),
          m_intensityFactor(1.0f),
          m_isActive(true) {
        std::cout << "Created anomaly of type " << static_cast<int>(effect.type) 
                 << " at position (" << effect.position.x << ", " 
                 << effect.position.y << ", " << effect.position.z 
                 << ") with radius " << effect.radius << std::endl;
    }
    
    void update(float deltaTime) {
        if (!m_isActive) return;
        
        m_remainingDuration -= deltaTime;
        if (m_remainingDuration <= 0.0f) {
            m_isActive = false;
            m_remainingDuration = 0.0f;
            std::cout << "Anomaly dissipated" << std::endl;
            return;
        }
        
        float lifetimeProgress = 1.0f - (m_remainingDuration / m_effect.duration);
        m_intensityFactor = 1.0f - std::abs(2.0f * lifetimeProgress - 1.0f);
    }
    
    bool affectsEntity(const AffectedEntity& entity) const {
        if (!m_isActive) return false;
        
        float distance = glm::distance(entity.getPosition(), m_effect.position);
        return distance <= m_effect.radius;
    }
    
    void applyToEntity(AffectedEntity& entity, float deltaTime) {
        if (!m_isActive || !affectsEntity(entity)) return;
        
        float distance = glm::distance(entity.getPosition(), m_effect.position);
        float distanceFactor = 1.0f - (distance / m_effect.radius);
        
        // Apply falloff with smooth transition at edge
        distanceFactor = glm::smoothstep(0.0f, 1.0f, distanceFactor);
        
        // Adjust effect based on intensity and distance
        AnomalyEffect adjustedEffect = m_effect;
        adjustedEffect.timeDistortion *= m_intensityFactor * distanceFactor;
        
        entity.applyTemporalEffect(adjustedEffect, deltaTime);
    }
    
    bool isActive() const { return m_isActive; }
    glm::vec3 getPosition() const { return m_effect.position; }
    float getRadius() const { return m_effect.radius; }
    AnomalyType getType() const { return m_effect.type; }
    const AnomalyEffect& getEffect() const { return m_effect; }
    float getRemainingDuration() const { return m_remainingDuration; }
    float getIntensityFactor() const { return m_intensityFactor; }
    
private:
    AnomalyEffect m_effect;
    float m_remainingDuration;
    float m_intensityFactor;
    bool m_isActive;
};

// Simplified temporal anomaly system for testing
class TemporalAnomalySystem {
public:
    TemporalAnomalySystem() {
        std::cout << "TemporalAnomalySystem initialized" << std::endl;
    }
    
    void createAnomaly(const AnomalyEffect& effect) {
        m_anomalies.emplace_back(effect);
        std::cout << "Created new anomaly, total: " << m_anomalies.size() << std::endl;
    }
    
    void update(float deltaTime) {
        // Update all anomalies
        for (auto& anomaly : m_anomalies) {
            anomaly.update(deltaTime);
        }
        
        // Process anomaly-entity interactions
        for (auto& anomaly : m_anomalies) {
            if (!anomaly.isActive()) continue;
            
            for (auto& [id, entity] : m_entities) {
                if (anomaly.affectsEntity(*entity)) {
                    anomaly.applyToEntity(*entity, deltaTime);
                }
            }
        }
        
        // Update all entities with their current time scale
        for (auto& [id, entity] : m_entities) {
            entity->update(deltaTime);
        }
        
        // Remove expired anomalies
        size_t initialCount = m_anomalies.size();
        m_anomalies.erase(
            std::remove_if(
                m_anomalies.begin(), 
                m_anomalies.end(),
                [](const TemporalAnomaly& a) { return !a.isActive(); }
            ),
            m_anomalies.end()
        );
        
        size_t removedCount = initialCount - m_anomalies.size();
        if (removedCount > 0) {
            std::cout << "Removed " << removedCount << " expired anomalies, remaining: " 
                     << m_anomalies.size() << std::endl;
        }
    }
    
    void registerEntity(std::shared_ptr<AffectedEntity> entity) {
        if (!entity) return;
        
        const std::string& id = entity->getIdentifier();
        m_entities[id] = entity;
        
        std::cout << "Registered entity: " << id << ", total entities: " 
                 << m_entities.size() << std::endl;
    }
    
    void unregisterEntity(const std::string& entityId) {
        auto it = m_entities.find(entityId);
        if (it != m_entities.end()) {
            m_entities.erase(it);
            std::cout << "Unregistered entity: " << entityId << ", remaining entities: " 
                     << m_entities.size() << std::endl;
        }
    }
    
    const std::vector<TemporalAnomaly>& getAnomalies() const { return m_anomalies; }
    size_t getAnomalyCount() const { return m_anomalies.size(); }
    
    bool isPointAffected(const glm::vec3& point) const {
        for (const auto& anomaly : m_anomalies) {
            if (!anomaly.isActive()) continue;
            
            float distance = glm::distance(point, anomaly.getPosition());
            if (distance <= anomaly.getRadius()) {
                return true;
            }
        }
        return false;
    }
    
    float getTimeDistortionAt(const glm::vec3& point) const {
        float combinedDistortion = 1.0f; // Default: no distortion
        bool pointAffected = false;
        
        for (const auto& anomaly : m_anomalies) {
            if (!anomaly.isActive()) continue;
            
            float distance = glm::distance(point, anomaly.getPosition());
            if (distance <= anomaly.getRadius()) {
                // Calculate falloff
                float distanceFactor = 1.0f - (distance / anomaly.getRadius());
                distanceFactor = glm::smoothstep(0.0f, 1.0f, distanceFactor);
                
                // Apply effect based on type
                float effectStrength = anomaly.getEffect().timeDistortion * 
                                      anomaly.getIntensityFactor() * 
                                      distanceFactor;
                
                // Combine effects based on type
                switch (anomaly.getType()) {
                    case AnomalyType::STASIS:
                    case AnomalyType::DILATION:
                        // Dilations slow time - fixed: directly apply the factor
                        combinedDistortion = effectStrength;
                        break;
                        
                    case AnomalyType::ACCELERATION:
                        // Accelerations speed up time
                        combinedDistortion = effectStrength;
                        break;
                        
                    case AnomalyType::REVERSION:
                        // Reversions make time negative
                        combinedDistortion = -effectStrength;
                        break;
                        
                    case AnomalyType::RIFT:
                        // Rifts cause unpredictable time distortion
                        combinedDistortion = effectStrength;
                        break;
                }
                
                pointAffected = true;
                // We're just using the most recent effect for simplicity in this test
            }
        }
        
        return pointAffected ? combinedDistortion : 1.0f;
    }
    
private:
    std::vector<TemporalAnomaly> m_anomalies;
    std::unordered_map<std::string, std::shared_ptr<TestEntity>> m_entities;
};

} // namespace Temporal
} // namespace TurtleEngine

// Main test function
int main() {
    std::cout << "=== TemporalAnomalySystem Standalone Test ===\n";

    // Create a TemporalAnomalySystem instance
    auto anomalySystem = std::make_shared<TurtleEngine::Temporal::TemporalAnomalySystem>();

    // Create a test entity positioned at (1,0,0), which should be within the 2m radius
    // of the anomaly that will be created at (0,0,0)
    auto entity = std::make_shared<TurtleEngine::Temporal::TestEntity>(
        "TestEntity", glm::vec3(1.0f, 0.0f, 0.0f));
    anomalySystem->registerEntity(entity);

    // Test 1: Create and apply a Dilation anomaly
    std::cout << "\nTest 1: Create and apply Dilation anomaly\n";
    TurtleEngine::Temporal::AnomalyEffect dilationEffect(
        TurtleEngine::Temporal::AnomalyType::DILATION,
        0.5f, // Slowdown to 50%
        glm::vec3(0.0f),
        2.0f, // 2m radius
        5.0f, // 5s duration
        "DilationTest"
    );
    
    // Initial time scale check
    std::cout << "Entity initial time scale: " << entity->getTimeScale() << std::endl;
    
    // Create the anomaly and update the system
    anomalySystem->createAnomaly(dilationEffect);
    anomalySystem->update(0.1f);
    
    // Check the entity's time scale - should be 0.5 now because of the dilation
    std::cout << "Entity time scale after dilation: " << entity->getTimeScale() << std::endl;
    bool dilationSuccess = (entity->getTimeScale() < 0.9f); // Should be close to 0.5
    std::cout << "Dilation Test: " << (dilationSuccess ? "PASSED" : "FAILED") 
              << " (Time Scale: " << entity->getTimeScale() << ")\n";

    // Test 2: Create and apply a Rift anomaly
    std::cout << "\nTest 2: Create and apply Rift anomaly\n";
    TurtleEngine::Temporal::AnomalyEffect riftEffect(
        TurtleEngine::Temporal::AnomalyType::RIFT,
        0.75f, // Chaotic distortion
        glm::vec3(0.0f),
        2.0f,
        5.0f,
        "RiftTest"
    );
    
    // Create the anomaly and update the system
    anomalySystem->createAnomaly(riftEffect);
    anomalySystem->update(0.1f);
    
    // Check the entity's time scale - should be affected by the rift
    std::cout << "Entity time scale after rift: " << entity->getTimeScale() << std::endl;
    bool riftSuccess = (entity->getTimeScale() != 1.0f); // Should be changed by the rift
    std::cout << "Rift Test: " << (riftSuccess ? "PASSED" : "FAILED") 
              << " (Time Scale: " << entity->getTimeScale() << ")\n";

    // Test 3: Entity moves out of anomaly range
    std::cout << "\nTest 3: Entity moves out of anomaly range\n";
    entity->setPosition(glm::vec3(5.0f, 0.0f, 0.0f)); // Move outside the 2m radius
    
    // Update again and check time scale
    anomalySystem->update(0.1f);
    std::cout << "Entity time scale after moving out of range: " << entity->getTimeScale() << std::endl;
    bool rangeSuccess = (entity->getTimeScale() == 1.0f); // Should return to normal
    std::cout << "Range Test: " << (rangeSuccess ? "PASSED" : "FAILED") 
              << " (Time Scale: " << entity->getTimeScale() << ")\n";

    std::cout << "\n=== TemporalAnomalySystem Test Complete ===\n";
    return (dilationSuccess && riftSuccess && rangeSuccess) ? 0 : 1;
} 