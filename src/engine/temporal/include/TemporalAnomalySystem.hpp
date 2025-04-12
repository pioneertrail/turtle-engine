/**
 * @file TemporalAnomalySystem.hpp
 * @brief System for managing temporal anomalies and their effects on entities
 * @author TurtleEngine Team
 */
#pragma once

#include <vector>
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

namespace TurtleEngine {
namespace Temporal {

// Forward declare TemporalCollisionHandler
class TemporalCollisionHandler;

/**
 * @enum AnomalyType
 * @brief Enumeration of different types of temporal anomalies
 */
enum class AnomalyType {
    NONE = 0,     ///< No effect
    TIME_SLOW,    ///< Slows down time
    TIME_FAST,    ///< Speeds up time
    RIFT,         ///< Teleports entities
    STASIS,       ///< Freezes entities in time
    REVERSAL,     ///< Reverses time
    INSTABILITY   ///< Random effects
};

/**
 * @struct AnomalyEffect
 * @brief Structure representing the effect of an anomaly
 */
struct AnomalyEffect {
    AnomalyType type;       ///< Type of anomaly
    float timeDistortion;   ///< Time distortion factor (1.0 = normal time)
    glm::vec3 position;     ///< Center position of the anomaly
    float radius;           ///< Radius of effect
    float duration;         ///< Total duration of the anomaly
    int sourceId;           ///< ID of the source anomaly

    /**
     * @brief Default constructor
     */
    AnomalyEffect() 
        : type(AnomalyType::NONE), timeDistortion(1.0f), position(0.0f), 
          radius(0.0f), duration(0.0f), sourceId(-1) {}

    /**
     * @brief Parameterized constructor
     * @param type Type of anomaly
     * @param timeDistortion Time distortion factor
     * @param position Center position
     * @param radius Radius of effect
     * @param duration Total duration
     * @param sourceId ID of source anomaly
     */
    AnomalyEffect(AnomalyType type, float timeDistortion, const glm::vec3& position,
                 float radius, float duration, int sourceId)
        : type(type), timeDistortion(timeDistortion), position(position),
          radius(radius), duration(duration), sourceId(sourceId) {}
};

/**
 * @class AffectedEntity
 * @brief Interface for entities that can be affected by temporal anomalies
 */
class AffectedEntity {
public:
    /**
     * @brief Virtual destructor
     */
    virtual ~AffectedEntity() = default;
    
    /**
     * @brief Get the entity's unique identifier
     * @return Entity's unique ID
     */
    virtual int GetId() const = 0;
    
    /**
     * @brief Get the entity's current position
     * @return 3D position vector
     */
    virtual glm::vec3 GetPosition() const = 0;
    
    /**
     * @brief Apply a temporal effect to this entity
     * @param effect The effect to apply
     */
    virtual void ApplyTemporalEffect(const AnomalyEffect& effect) = 0;
};

/**
 * @class TemporalAnomaly
 * @brief Class representing a single temporal anomaly in the game world
 * 
 * A temporal anomaly is a localized distortion in time that affects entities
 * within its radius of influence. Different types of anomalies cause different
 * effects (slowing time, speeding it up, etc.).
 */
class TemporalAnomaly {
public:
    /**
     * @brief Constructor
     * @param id Unique identifier for this anomaly
     * @param type Type of anomaly
     * @param timeDistortion Time distortion factor (1.0 = normal time)
     * @param position Center position of the anomaly
     * @param radius Radius of effect
     * @param duration Total duration the anomaly will exist
     */
    TemporalAnomaly(int id, AnomalyType type, float timeDistortion, const glm::vec3& position,
                   float radius, float duration);

    /**
     * @brief Default destructor
     */
    ~TemporalAnomaly() = default;

    /**
     * @brief Update the anomaly's state
     * @param deltaTime Time elapsed since last update in seconds
     */
    void Update(float deltaTime);
    
    /**
     * @brief Check if a point is within this anomaly's area of effect
     * @param point The point to check
     * @return True if the point is inside the anomaly's radius
     */
    bool ContainsPoint(const glm::vec3& point) const;
    
    /**
     * @brief Check if this anomaly overlaps with another anomaly
     * @param other The other anomaly to check against
     * @return True if the anomalies' areas of effect overlap
     */
    bool Overlaps(const TemporalAnomaly& other) const;
    
    /**
     * @brief Check if the anomaly is still active
     * @return True if the anomaly is active, false if expired
     */
    bool IsActive() const;
    
    /**
     * @brief Forcibly deactivate the anomaly before its duration expires
     */
    void Deactivate();
    
    /**
     * @brief Get the unique identifier of this anomaly
     * @return The anomaly's ID
     */
    int GetIdentifier() const;
    
    /**
     * @brief Get the type of this anomaly
     * @return The anomaly's type
     */
    AnomalyType GetType() const;
    
    /**
     * @brief Get the time distortion factor of this anomaly
     * @return Time distortion factor (1.0 = normal time)
     */
    float GetTimeDistortion() const;
    
    /**
     * @brief Get the center position of this anomaly
     * @return 3D position vector
     */
    const glm::vec3& GetPosition() const;
    
    /**
     * @brief Get the radius of this anomaly
     * @return Radius of effect
     */
    float GetRadius() const;
    
    /**
     * @brief Get the total duration of this anomaly
     * @return Total duration in seconds
     */
    float GetDuration() const;
    
    /**
     * @brief Get the remaining time until this anomaly expires
     * @return Remaining time in seconds
     */
    float GetRemainingTime() const;
    
    /**
     * @brief Get the effect structure for this anomaly
     * @return Constant reference to the anomaly effect
     */
    const AnomalyEffect& GetEffect() const;

private:
    int m_id;                  ///< Unique identifier
    AnomalyType m_type;        ///< Type of anomaly
    float m_timeDistortion;    ///< Time distortion factor
    glm::vec3 m_position;      ///< Center position
    float m_radius;            ///< Radius of effect
    float m_duration;          ///< Total duration
    float m_elapsedTime = 0.0f; ///< Time elapsed since creation
    bool m_active;             ///< Whether the anomaly is active
    AnomalyEffect m_effect;    ///< The effect structure for this anomaly
};

/**
 * @class TemporalAnomalySystem
 * @brief System for managing temporal anomalies and their effects on entities
 * 
 * The TemporalAnomalySystem is responsible for:
 * - Creating and tracking all temporal anomalies
 * - Registering entities that can be affected by anomalies
 * - Updating anomaly state and applying effects to entities
 * - Providing access to active anomalies
 */
class TemporalAnomalySystem {
public:
    using AnomalyRemovedCallback = std::function<void(int)>;

    /**
     * @brief Constructor
     */
    TemporalAnomalySystem();
    
    /**
     * @brief Destructor
     */
    ~TemporalAnomalySystem();
    
    /**
     * @brief Initialize the temporal anomaly system
     * @return True if initialization was successful
     */
    bool initialize();

    /**
     * @brief Register an entity to be affected by temporal anomalies
     * @param entity Shared pointer to the entity
     */
    void RegisterEntity(std::shared_ptr<AffectedEntity> entity);
    
    /**
     * @brief Unregister an entity from being affected by anomalies
     * @param entityId ID of the entity to unregister
     */
    void UnregisterEntity(int entityId);
    
    /**
     * @brief Create a new temporal anomaly with the specified parameters
     * @param type Type of anomaly
     * @param timeDistortion Time distortion factor
     * @param position Center position
     * @param radius Radius of effect
     * @param duration Total duration
     * @return Unique ID of the created anomaly
     */
    int CreateAnomaly(
        AnomalyType type, float timeDistortion, const glm::vec3& position,
        float radius, float duration);
    
    /**
     * @brief Alternative overload for creating a temporal anomaly
     * @param position Center position
     * @param radius Radius of effect
     * @param type Type of anomaly
     * @param timeDistortion Time distortion factor
     * @param duration Total duration
     * @return Unique ID of the created anomaly
     */
    int CreateAnomaly(
        const glm::vec3& position, float radius, AnomalyType type, 
        float timeDistortion, float duration);
    
    /**
     * @brief Update all anomalies and process their effects
     * @param deltaTime Time elapsed since last update in seconds
     */
    void Update(float deltaTime);
    
    /**
     * @brief Get all anomalies (active and inactive)
     * @return Constant reference to vector of all anomalies
     */
    const std::vector<std::shared_ptr<TemporalAnomaly>>& GetAnomalies() const;
    
    /**
     * @brief Get only active anomalies
     * @return Vector of active anomalies
     */
    std::vector<std::shared_ptr<TemporalAnomaly>> GetActiveAnomalies() const;
    
    /**
     * @brief Get the count of active anomalies
     * @return Number of active anomalies
     */
    size_t GetActiveAnomalyCount() const;
    
    /**
     * @brief Find an anomaly by its ID
     * @param anomalyId ID of the anomaly to find
     * @return Shared pointer to the anomaly, or nullptr if not found
     */
    std::shared_ptr<TemporalAnomaly> FindAnomaly(int anomalyId) const;
    
    /**
     * @brief Forcibly deactivate an anomaly
     * @param anomalyId ID of the anomaly to deactivate
     */
    void DeactivateAnomaly(int anomalyId);
    
    /**
     * @brief Remove all anomalies and reset the system
     */
    void Clear();

    /**
     * @brief Set callback for when a new anomaly is created
     * @param callback Function to call when an anomaly is created
     */
    void SetAnomalyCreatedCallback(std::function<void(std::shared_ptr<TemporalAnomaly>)> callback);
    
    /**
     * @brief Set callback for when an anomaly is removed
     * @param callback Function to call when an anomaly is removed
     */
    void SetAnomalyRemovedCallback(AnomalyRemovedCallback callback);

    /**
     * @brief Get all registered entities
     * @return Constant reference to vector of entities
     */
    const std::vector<std::shared_ptr<AffectedEntity>>& GetEntities() const { return m_entities; }

    /**
     * @brief Creates an anomaly with the specified parameters
     * 
     * This is a simplified version for the demo that automatically
     * sets appropriate time distortion based on the anomaly type.
     * 
     * @param position Center position of the anomaly
     * @param radius Radius of the anomaly's effect
     * @param intensity Intensity of the temporal effect (0.0-1.0)
     * @param duration Duration of the anomaly in seconds
     * @param type Type of anomaly
     * @return ID of the created anomaly
     */
    int createAnomaly(
        const glm::vec3& position,
        float radius,
        float intensity,
        float duration,
        AnomalyType type);
    
    /**
     * @brief Get a list of all active anomalies
     * 
     * @return Vector of active anomaly effects
     */
    std::vector<AnomalyEffect> getActiveAnomalies() const;

    bool RemoveAnomaly(int id);

private:
    std::vector<std::shared_ptr<AffectedEntity>> m_entities;  ///< Registered entities
    std::vector<std::shared_ptr<TemporalAnomaly>> m_anomalies; ///< All anomalies
    std::function<void(std::shared_ptr<TemporalAnomaly>)> m_anomalyCreatedCallback; ///< Callback for anomaly creation
    AnomalyRemovedCallback m_anomalyRemovedCallback; ///< Callback for anomaly removal
    int m_nextAnomalyId = 0; ///< Counter for generating unique anomaly IDs
};

} // namespace Temporal
} // namespace TurtleEngine 