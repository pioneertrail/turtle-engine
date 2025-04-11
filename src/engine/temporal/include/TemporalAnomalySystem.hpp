#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <glm/glm.hpp>

namespace TurtleEngine {
namespace Temporal {

// Types of temporal anomalies that can exist in the game world
enum class AnomalyType {
    STASIS,       // Complete time stop or near-stop
    DILATION,     // Time slowdown
    ACCELERATION, // Time speedup
    REVERSION,    // Time reversal
    RIFT          // Chaotic/random temporal effects
};

// Helper function to get string representation of anomaly types
std::string getAnomalyTypeName(AnomalyType type);

// Forward declaration of the AffectedEntity interface
class AffectedEntity;

// Structure defining the properties of a temporal anomaly effect
struct AnomalyEffect {
    AnomalyType type;          // Type of anomaly
    float timeDistortion;      // Time scale factor (0.0 = stopped, 0.5 = half speed, 2.0 = double speed)
    glm::vec3 position;        // World position of anomaly center
    float radius;              // Radius of effect
    float duration;            // How long the anomaly lasts
    std::string identifier;    // Unique identifier for this anomaly instance

    AnomalyEffect(
        AnomalyType type,
        float timeDistortion,
        const glm::vec3& position,
        float radius,
        float duration,
        const std::string& identifier
    ) : type(type), timeDistortion(timeDistortion), position(position),
        radius(radius), duration(duration), identifier(identifier) {}
};

// Interface for entities that can be affected by temporal anomalies
class AffectedEntity {
public:
    virtual ~AffectedEntity() = default;
    
    // Called when the entity is affected by a temporal anomaly
    virtual void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) = 0;
    
    // Returns the entity's current position for spatial queries
    virtual glm::vec3 getPosition() const = 0;
    
    // Returns a unique identifier for this entity
    virtual std::string getIdentifier() const = 0;
};

// Represents an active temporal anomaly in the game world
class TemporalAnomaly {
public:
    TemporalAnomaly(const AnomalyEffect& effect)
        : m_effect(effect), m_remainingDuration(effect.duration) {}
    
    // Updates the anomaly, returns false if the anomaly should be removed
    bool update(float deltaTime) {
        m_remainingDuration -= deltaTime;
        return m_remainingDuration > 0.0f;
    }
    
    // Accessors for anomaly properties
    AnomalyType getType() const { return m_effect.type; }
    float getTimeDistortion() const { return m_effect.timeDistortion; }
    glm::vec3 getPosition() const { return m_effect.position; }
    float getRadius() const { return m_effect.radius; }
    float getRemainingDuration() const { return m_remainingDuration; }
    std::string getIdentifier() const { return m_effect.identifier; }
    
    // Returns the complete effect structure
    const AnomalyEffect& getEffect() const { return m_effect; }
    
    // Checks if the specified point is within this anomaly's radius
    bool containsPoint(const glm::vec3& point) const {
        float distanceSquared = glm::distance2(m_effect.position, point);
        return distanceSquared <= (m_effect.radius * m_effect.radius);
    }
    
    // Checks if this anomaly overlaps with another
    bool overlaps(const TemporalAnomaly& other) const {
        float distance = glm::distance(m_effect.position, other.getPosition());
        return distance < (m_effect.radius + other.getRadius());
    }
    
private:
    AnomalyEffect m_effect;
    float m_remainingDuration;
};

// Main system for managing temporal anomalies and their effects
class TemporalAnomalySystem {
public:
    TemporalAnomalySystem();
    ~TemporalAnomalySystem();
    
    // Creates a new temporal anomaly with the specified effect
    void createAnomaly(const AnomalyEffect& effect);
    
    // Removes an anomaly by identifier
    void removeAnomaly(const std::string& identifier);
    
    // Registers an entity to be affected by temporal anomalies
    void registerEntity(std::shared_ptr<AffectedEntity> entity);
    
    // Unregisters an entity
    void unregisterEntity(const std::string& identifier);
    
    // Updates all anomalies and applies their effects to entities
    void update(float deltaTime);
    
    // Returns a reference to all active anomalies
    const std::vector<TemporalAnomaly>& getAnomalies() const { return m_anomalies; }
    
    // Returns a specific anomaly by identifier, or nullptr if not found
    const TemporalAnomaly* getAnomalyById(const std::string& identifier) const;
    
    // Get all entities currently registered with the system
    const std::unordered_map<std::string, std::shared_ptr<AffectedEntity>>& getEntities() const { 
        return m_entities; 
    }
    
private:
    std::vector<TemporalAnomaly> m_anomalies;
    std::unordered_map<std::string, std::shared_ptr<AffectedEntity>> m_entities;
};

} // namespace Temporal
} // namespace TurtleEngine 