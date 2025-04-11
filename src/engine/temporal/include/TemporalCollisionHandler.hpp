#pragma once

#include "TemporalAnomalySystem.hpp"
#include <memory>
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string>

namespace TurtleEngine {
namespace Temporal {

// Result of anomaly combination
struct CombinedAnomalyEffect {
    AnomalyType resultType;     // The resulting type of the combination
    float timeDistortion;       // Combined time distortion factor
    std::vector<AnomalyType> sourceTypes; // Types that created this combination
    
    CombinedAnomalyEffect(AnomalyType type, float distortion) 
        : resultType(type), timeDistortion(distortion) {}
    
    CombinedAnomalyEffect(AnomalyType type, float distortion, const std::vector<AnomalyType>& sources) 
        : resultType(type), timeDistortion(distortion), sourceTypes(sources) {}
};

// Class that handles interactions between temporal anomalies and entities
class TemporalCollisionHandler {
public:
    // Callback definitions for collision events
    using EntityEnterCallback = std::function<void(const std::string&, const TemporalAnomaly&)>;
    using EntityExitCallback = std::function<void(const std::string&, const TemporalAnomaly&)>;
    using AnomaliesOverlapCallback = std::function<void(const TemporalAnomaly&, const TemporalAnomaly&)>;
    using AnomalyCombinationCallback = std::function<void(const CombinedAnomalyEffect&, const glm::vec3&, float)>;
    
    TemporalCollisionHandler(std::shared_ptr<TemporalAnomalySystem> anomalySystem);
    ~TemporalCollisionHandler();
    
    // Initialize the collision handler
    void initialize();
    
    // Update collision detection and resolution
    void update(float deltaTime);
    
    // Get a list of anomalies currently affecting an entity
    std::vector<TemporalAnomaly> getAnomaliesAffectingEntity(const std::string& entityId) const;
    
    // Get the total time distortion factor for an entity (combining all anomalies)
    float getEntityTimeDistortion(const std::string& entityId) const;
    
    // Set callback for when entities enter an anomaly
    void setEntityEnterCallback(EntityEnterCallback callback);
    
    // Set callback for when entities exit an anomaly
    void setEntityExitCallback(EntityExitCallback callback);
    
    // Set callback for when anomalies overlap
    void setAnomaliesOverlapCallback(AnomaliesOverlapCallback callback);
    
    // Set callback for when anomalies combine to form a new effect
    void setAnomalyCombinationCallback(AnomalyCombinationCallback callback);
    
    // Get areas where anomalies are combining/overlapping
    std::vector<std::pair<glm::vec3, CombinedAnomalyEffect>> getAnomalyCombinationAreas() const;
    
private:
    // Reference to the temporal anomaly system
    std::shared_ptr<TemporalAnomalySystem> m_anomalySystem;
    
    // Tracking data structures
    std::unordered_map<std::string, std::unordered_set<std::string>> m_entityAnomalyMap;
    std::vector<std::pair<glm::vec3, CombinedAnomalyEffect>> m_combinationAreas;
    
    // Event callbacks
    EntityEnterCallback m_entityEnterCallback;
    EntityExitCallback m_entityExitCallback;
    AnomaliesOverlapCallback m_anomaliesOverlapCallback;
    AnomalyCombinationCallback m_anomalyCombinationCallback;
    
    // Private helper methods
    void detectEntityAnomalyCollisions(float deltaTime);
    void detectAnomalyOverlaps(float deltaTime);
    void resolveAnomalyCombinations(float deltaTime);
    
    // Determines the combined effect of two anomalies
    CombinedAnomalyEffect combineAnomalyEffects(const TemporalAnomaly& a1, const TemporalAnomaly& a2) const;
    
    // Determines the combined effect of multiple anomalies
    CombinedAnomalyEffect combineMultipleAnomalyEffects(
        const std::vector<const TemporalAnomaly*>& anomalies) const;
};

} // namespace Temporal
} // namespace TurtleEngine 