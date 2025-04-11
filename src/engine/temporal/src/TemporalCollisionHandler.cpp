#include "../include/TemporalCollisionHandler.hpp"
#include <iostream>
#include <algorithm>
#include <cmath>

namespace TurtleEngine {
namespace Temporal {

TemporalCollisionHandler::TemporalCollisionHandler(std::shared_ptr<TemporalAnomalySystem> anomalySystem)
    : m_anomalySystem(anomalySystem) {
    
    if (!m_anomalySystem) {
        std::cerr << "Error: TemporalCollisionHandler created with null anomaly system!" << std::endl;
    }
}

TemporalCollisionHandler::~TemporalCollisionHandler() {
    // Clean up resources
    m_entityAnomalyMap.clear();
    m_combinationAreas.clear();
}

void TemporalCollisionHandler::initialize() {
    std::cout << "Temporal Collision Handler initialized" << std::endl;
}

void TemporalCollisionHandler::update(float deltaTime) {
    if (!m_anomalySystem) {
        return;
    }
    
    // Clear previous combination areas
    m_combinationAreas.clear();
    
    // Detect and process entity-anomaly collisions
    detectEntityAnomalyCollisions(deltaTime);
    
    // Detect and process anomaly-anomaly overlaps
    detectAnomalyOverlaps(deltaTime);
    
    // Resolve any anomaly combinations
    resolveAnomalyCombinations(deltaTime);
}

void TemporalCollisionHandler::detectEntityAnomalyCollisions(float deltaTime) {
    const auto& entities = m_anomalySystem->getEntities();
    const auto& anomalies = m_anomalySystem->getAnomalies();
    
    // For each entity, determine which anomalies it is currently in
    for (const auto& entityPair : entities) {
        const std::string& entityId = entityPair.first;
        const auto& entity = entityPair.second;
        
        // Get current position of entity
        const glm::vec3 entityPos = entity->getPosition();
        
        // Create a set of current anomalies affecting this entity
        std::unordered_set<std::string> currentAnomalies;
        
        for (const auto& anomaly : anomalies) {
            if (anomaly.containsPoint(entityPos)) {
                currentAnomalies.insert(anomaly.getIdentifier());
            }
        }
        
        // Check for entity entering new anomalies
        if (m_entityEnterCallback) {
            for (const auto& anomalyId : currentAnomalies) {
                auto previousIt = m_entityAnomalyMap[entityId].find(anomalyId);
                if (previousIt == m_entityAnomalyMap[entityId].end()) {
                    // Entity entered a new anomaly
                    const TemporalAnomaly* anomaly = m_anomalySystem->getAnomalyById(anomalyId);
                    if (anomaly) {
                        m_entityEnterCallback(entityId, *anomaly);
                    }
                }
            }
        }
        
        // Check for entity exiting anomalies
        if (m_entityExitCallback) {
            for (const auto& anomalyId : m_entityAnomalyMap[entityId]) {
                auto currentIt = currentAnomalies.find(anomalyId);
                if (currentIt == currentAnomalies.end()) {
                    // Entity exited an anomaly
                    const TemporalAnomaly* anomaly = m_anomalySystem->getAnomalyById(anomalyId);
                    if (anomaly) {
                        m_entityExitCallback(entityId, *anomaly);
                    }
                }
            }
        }
        
        // Update the map with current anomalies
        m_entityAnomalyMap[entityId] = std::move(currentAnomalies);
    }
}

void TemporalCollisionHandler::detectAnomalyOverlaps(float deltaTime) {
    const auto& anomalies = m_anomalySystem->getAnomalies();
    
    // Check for overlaps between anomalies
    for (size_t i = 0; i < anomalies.size(); ++i) {
        for (size_t j = i + 1; j < anomalies.size(); ++j) {
            if (anomalies[i].overlaps(anomalies[j])) {
                // Anomalies overlap
                if (m_anomaliesOverlapCallback) {
                    m_anomaliesOverlapCallback(anomalies[i], anomalies[j]);
                }
                
                // Calculate the center point of overlap
                const glm::vec3 center1 = anomalies[i].getPosition();
                const glm::vec3 center2 = anomalies[j].getPosition();
                const float radius1 = anomalies[i].getRadius();
                const float radius2 = anomalies[j].getRadius();
                
                // Calculate distance between centers
                const float distance = glm::distance(center1, center2);
                
                // Calculate overlap point - weighted toward the larger anomaly
                glm::vec3 overlapCenter;
                if (distance > 0.001f) {  // Avoid division by zero
                    const float weight1 = radius1 / (radius1 + radius2);
                    const float weight2 = radius2 / (radius1 + radius2);
                    overlapCenter = center1 * weight2 + center2 * weight1;
                } else {
                    // Centers are basically at the same point
                    overlapCenter = center1;
                }
                
                // Calculate combined effect
                CombinedAnomalyEffect combinedEffect = combineAnomalyEffects(anomalies[i], anomalies[j]);
                
                // Store this combination area
                m_combinationAreas.emplace_back(overlapCenter, combinedEffect);
            }
        }
    }
}

void TemporalCollisionHandler::resolveAnomalyCombinations(float deltaTime) {
    // Process the combination areas we've identified
    for (const auto& combination : m_combinationAreas) {
        const auto& center = combination.first;
        const auto& effect = combination.second;
        
        // Determine overlap radius (simplified as average of source anomalies)
        float radius = 0.0f;
        const auto& anomalies = m_anomalySystem->getAnomalies();
        
        // Find all anomalies at this point for radius calculation
        std::vector<const TemporalAnomaly*> overlappingAnomalies;
        for (const auto& anomaly : anomalies) {
            if (anomaly.containsPoint(center)) {
                overlappingAnomalies.push_back(&anomaly);
                radius += anomaly.getRadius();
            }
        }
        
        if (!overlappingAnomalies.empty()) {
            radius /= overlappingAnomalies.size();
        } else {
            // Fallback if we somehow don't have any anomalies here
            radius = 1.0f;
        }
        
        // Call the combination callback if set
        if (m_anomalyCombinationCallback) {
            m_anomalyCombinationCallback(effect, center, radius);
        }
        
        // Apply the combined effect to any entities in the overlap area
        const auto& entities = m_anomalySystem->getEntities();
        for (const auto& entityPair : entities) {
            const auto& entity = entityPair.second;
            const glm::vec3 entityPos = entity->getPosition();
            
            // Check if entity is in the combination area
            const float distanceSquared = glm::distance2(center, entityPos);
            if (distanceSquared <= (radius * radius)) {
                // Create a temporary effect structure
                AnomalyEffect combinedAnomalyEffect(
                    effect.resultType,
                    effect.timeDistortion,
                    center,
                    radius,
                    deltaTime,  // Just for this frame
                    "Combined_Effect"
                );
                
                // Apply the combined effect
                entity->applyTemporalEffect(combinedAnomalyEffect, deltaTime);
            }
        }
    }
}

CombinedAnomalyEffect TemporalCollisionHandler::combineAnomalyEffects(
    const TemporalAnomaly& a1, const TemporalAnomaly& a2) const {
    
    AnomalyType type1 = a1.getType();
    AnomalyType type2 = a2.getType();
    float distortion1 = a1.getTimeDistortion();
    float distortion2 = a2.getTimeDistortion();
    
    // Store source types
    std::vector<AnomalyType> sourceTypes = {type1, type2};
    
    // Determine the resulting anomaly type and distortion factor
    
    // Special case: If either anomaly is a rift, the result is a rift
    if (type1 == AnomalyType::RIFT || type2 == AnomalyType::RIFT) {
        // With a rift, effects become chaotic and unpredictable
        // Amplify the distortion
        float resultDistortion = distortion1 * distortion2;
        return CombinedAnomalyEffect(AnomalyType::RIFT, resultDistortion, sourceTypes);
    }
    
    // Special case: STASIS + any other type = STASIS
    if (type1 == AnomalyType::STASIS || type2 == AnomalyType::STASIS) {
        // Stasis dominates but is slightly weakened by the other effect
        float resultDistortion = std::min(distortion1, distortion2) * 0.8f;
        return CombinedAnomalyEffect(AnomalyType::STASIS, resultDistortion, sourceTypes);
    }
    
    // DILATION + ACCELERATION
    if ((type1 == AnomalyType::DILATION && type2 == AnomalyType::ACCELERATION) ||
        (type1 == AnomalyType::ACCELERATION && type2 == AnomalyType::DILATION)) {
        
        // These effects partially cancel each other, but not completely
        // The stronger effect determines the resulting type
        AnomalyType resultType;
        float resultDistortion;
        
        if (std::abs(1.0f - distortion1) > std::abs(1.0f - distortion2)) {
            // First effect is stronger
            resultType = type1;
            // Weakened by the opposing effect
            resultDistortion = 1.0f + (distortion1 - 1.0f) * 0.6f;
        } else {
            // Second effect is stronger
            resultType = type2;
            // Weakened by the opposing effect
            resultDistortion = 1.0f + (distortion2 - 1.0f) * 0.6f;
        }
        
        return CombinedAnomalyEffect(resultType, resultDistortion, sourceTypes);
    }
    
    // DILATION + REVERSION or ACCELERATION + REVERSION
    if ((type1 == AnomalyType::DILATION && type2 == AnomalyType::REVERSION) ||
        (type1 == AnomalyType::REVERSION && type2 == AnomalyType::DILATION) ||
        (type1 == AnomalyType::ACCELERATION && type2 == AnomalyType::REVERSION) ||
        (type1 == AnomalyType::REVERSION && type2 == AnomalyType::ACCELERATION)) {
        
        // Reversion combined with time speed changes creates a RIFT
        // The distortion is amplified
        float resultDistortion = distortion1 * distortion2 * -1.0f; // Negative to indicate reversal
        return CombinedAnomalyEffect(AnomalyType::RIFT, resultDistortion, sourceTypes);
    }
    
    // DILATION + DILATION or ACCELERATION + ACCELERATION
    if ((type1 == type2) && (type1 == AnomalyType::DILATION || type1 == AnomalyType::ACCELERATION)) {
        // Same type combines to strengthen the effect
        float resultDistortion;
        
        if (type1 == AnomalyType::DILATION) {
            // For dilation (slowing), effects multiply (becomes even slower)
            resultDistortion = distortion1 * distortion2;
        } else {
            // For acceleration (speeding up), effects add (becomes even faster)
            resultDistortion = distortion1 + distortion2 - 1.0f;
        }
        
        return CombinedAnomalyEffect(type1, resultDistortion, sourceTypes);
    }
    
    // REVERSION + REVERSION
    if (type1 == AnomalyType::REVERSION && type2 == AnomalyType::REVERSION) {
        // Double reversal cancels out, but creates instability
        // Result is acceleration but with some instability
        float resultDistortion = (distortion1 + distortion2) * 0.5f;
        return CombinedAnomalyEffect(AnomalyType::ACCELERATION, std::abs(resultDistortion), sourceTypes);
    }
    
    // Default case: average the effects
    float avgDistortion = (distortion1 + distortion2) * 0.5f;
    AnomalyType resultType = (std::abs(1.0f - distortion1) > std::abs(1.0f - distortion2)) ? type1 : type2;
    return CombinedAnomalyEffect(resultType, avgDistortion, sourceTypes);
}

CombinedAnomalyEffect TemporalCollisionHandler::combineMultipleAnomalyEffects(
    const std::vector<const TemporalAnomaly*>& anomalies) const {
    
    if (anomalies.empty()) {
        return CombinedAnomalyEffect(AnomalyType::DILATION, 1.0f);
    }
    
    if (anomalies.size() == 1) {
        return CombinedAnomalyEffect(
            anomalies[0]->getType(), 
            anomalies[0]->getTimeDistortion(),
            {anomalies[0]->getType()}
        );
    }
    
    // For 2 anomalies, use the dedicated combination function
    if (anomalies.size() == 2) {
        return combineAnomalyEffects(*anomalies[0], *anomalies[1]);
    }
    
    // For more than 2 anomalies, process them in pairs
    std::vector<AnomalyType> sourceTypes;
    for (const auto& anomaly : anomalies) {
        sourceTypes.push_back(anomaly->getType());
    }
    
    // Start with the first anomaly
    CombinedAnomalyEffect result(
        anomalies[0]->getType(), 
        anomalies[0]->getTimeDistortion(),
        {anomalies[0]->getType()}
    );
    
    // Combine with each subsequent anomaly
    for (size_t i = 1; i < anomalies.size(); ++i) {
        // Create a temporary anomaly to represent the current combined effect
        TemporalAnomaly tempAnomaly(
            AnomalyEffect(
                result.resultType,
                result.timeDistortion,
                glm::vec3(0.0f), // Position not used in combination logic
                1.0f,            // Radius not used in combination logic
                1.0f,            // Duration not used in combination logic
                "temp"
            )
        );
        
        // Combine with the next anomaly
        result = combineAnomalyEffects(tempAnomaly, *anomalies[i]);
    }
    
    // Set all the source types
    result.sourceTypes = sourceTypes;
    
    return result;
}

std::vector<TemporalAnomaly> TemporalCollisionHandler::getAnomaliesAffectingEntity(
    const std::string& entityId) const {
    
    std::vector<TemporalAnomaly> result;
    
    // Check if entity is in our map
    auto it = m_entityAnomalyMap.find(entityId);
    if (it == m_entityAnomalyMap.end()) {
        return result;
    }
    
    // Get all anomalies affecting this entity
    const auto& anomalyIds = it->second;
    const auto& allAnomalies = m_anomalySystem->getAnomalies();
    
    for (const auto& anomaly : allAnomalies) {
        if (anomalyIds.find(anomaly.getIdentifier()) != anomalyIds.end()) {
            result.push_back(anomaly);
        }
    }
    
    return result;
}

float TemporalCollisionHandler::getEntityTimeDistortion(const std::string& entityId) const {
    const auto affectingAnomalies = getAnomaliesAffectingEntity(entityId);
    
    if (affectingAnomalies.empty()) {
        return 1.0f; // Normal time flow
    }
    
    // Convert to pointers for combineMultipleAnomalyEffects
    std::vector<const TemporalAnomaly*> anomalyPtrs;
    for (const auto& anomaly : affectingAnomalies) {
        anomalyPtrs.push_back(&anomaly);
    }
    
    // Use our combination logic to determine the total time distortion
    CombinedAnomalyEffect combinedEffect = combineMultipleAnomalyEffects(anomalyPtrs);
    return combinedEffect.timeDistortion;
}

void TemporalCollisionHandler::setEntityEnterCallback(EntityEnterCallback callback) {
    m_entityEnterCallback = callback;
}

void TemporalCollisionHandler::setEntityExitCallback(EntityExitCallback callback) {
    m_entityExitCallback = callback;
}

void TemporalCollisionHandler::setAnomaliesOverlapCallback(AnomaliesOverlapCallback callback) {
    m_anomaliesOverlapCallback = callback;
}

void TemporalCollisionHandler::setAnomalyCombinationCallback(AnomalyCombinationCallback callback) {
    m_anomalyCombinationCallback = callback;
}

std::vector<std::pair<glm::vec3, CombinedAnomalyEffect>> 
TemporalCollisionHandler::getAnomalyCombinationAreas() const {
    return m_combinationAreas;
}

} // namespace Temporal
} // namespace TurtleEngine 