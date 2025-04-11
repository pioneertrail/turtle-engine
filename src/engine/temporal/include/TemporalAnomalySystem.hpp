#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace TurtleEngine {
namespace Temporal {

/**
 * @enum AnomalyType
 * @brief Types of temporal anomalies that can be created
 */
enum class AnomalyType {
    RIFT,       // Spatial discontinuity in the timestream
    STASIS,     // Time freeze effect
    DILATION,   // Time slowdown effect
    ACCELERATION,// Time speedup effect
    REVERSION   // Time reversal effect
};

/**
 * @struct AnomalyEffect
 * @brief Detailed information about a temporal anomaly
 */
struct AnomalyEffect {
    AnomalyType type;            // Type of anomaly
    float timeDistortion;        // Time distortion factor (0-1 for slowdown, >1 for speedup, negative for reversal)
    glm::vec3 position;          // Center position of the anomaly
    float radius;                // Radius of effect
    float duration;              // Duration in seconds
    std::string sourceIdentifier; // Identifier of anomaly source
    
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

/**
 * @class AffectedEntity
 * @brief Interface for entities that can be affected by temporal anomalies
 */
class AffectedEntity {
public:
    virtual ~AffectedEntity() = default;
    
    // Apply temporal effect to this entity
    virtual void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) = 0;
    
    // Get entity position for spatial calculations
    virtual glm::vec3 getPosition() const = 0;
    
    // Get entity identifier
    virtual std::string getIdentifier() const = 0;
};

/**
 * @class TemporalAnomaly
 * @brief Manages temporal anomaly instances
 */
class TemporalAnomaly {
public:
    TemporalAnomaly(const AnomalyEffect& effect);
    ~TemporalAnomaly() = default;
    
    // Update the anomaly state
    void update(float deltaTime);
    
    // Check if entity is within effect radius
    bool affectsEntity(const AffectedEntity& entity) const;
    
    // Apply anomaly effect to an entity
    void applyToEntity(AffectedEntity& entity, float deltaTime);
    
    // Check if anomaly is still active
    bool isActive() const;
    
    // Get anomaly position
    glm::vec3 getPosition() const;
    
    // Get anomaly radius
    float getRadius() const;
    
    // Get anomaly type
    AnomalyType getType() const;
    
    // Get anomaly effect details
    const AnomalyEffect& getEffect() const;
    
    // Get remaining duration
    float getRemainingDuration() const;
    
    // Get intensity factor (based on remaining duration)
    float getIntensityFactor() const;
    
private:
    AnomalyEffect m_effect;
    float m_remainingDuration;
    float m_intensityFactor;
    bool m_isActive;
    float m_stabilityFactor; // How stable the anomaly is (affects visual effects)
};

/**
 * @class TemporalAnomalySystem
 * @brief Manages all temporal anomalies and their effects
 */
class TemporalAnomalySystem {
public:
    TemporalAnomalySystem();
    ~TemporalAnomalySystem() = default;
    
    // Create a new anomaly
    void createAnomaly(const AnomalyEffect& effect);
    
    // Update all anomalies
    void update(float deltaTime);
    
    // Register an entity to be affected by anomalies
    void registerEntity(std::shared_ptr<AffectedEntity> entity);
    
    // Unregister an entity
    void unregisterEntity(const std::string& entityId);
    
    // Get all active anomalies
    const std::vector<TemporalAnomaly>& getAnomalies() const;
    
    // Debug visualization callback
    using DebugDrawCallback = std::function<void(const TemporalAnomaly&)>;
    void setDebugDrawCallback(DebugDrawCallback callback);
    
    // Get count of active anomalies
    size_t getAnomalyCount() const;
    
    // Clear all anomalies
    void clearAnomalies();
    
    // Test if a point in space is affected by any anomaly
    bool isPointAffected(const glm::vec3& point) const;
    
    // Get combined time distortion factor at a point in space
    float getTimeDistortionAt(const glm::vec3& point) const;
    
private:
    std::vector<TemporalAnomaly> m_anomalies;
    std::unordered_map<std::string, std::shared_ptr<AffectedEntity>> m_entities;
    DebugDrawCallback m_debugDrawCallback;
    
    // Process anomaly-entity interactions
    void processInteractions(float deltaTime);
    
    // Remove expired anomalies
    void removeExpiredAnomalies();
};

} // namespace Temporal
} // namespace TurtleEngine 