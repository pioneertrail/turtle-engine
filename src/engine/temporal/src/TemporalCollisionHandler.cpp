#include "TemporalCollisionHandler.hpp"
#include "../../include/utils/Logger.hpp"
#include <algorithm>
#include <cmath>
#include <unordered_set>

namespace TurtleEngine {
namespace Temporal {

TemporalCollisionHandler::TemporalCollisionHandler(TemporalAnomalySystem* anomalySystem)
    : m_anomalySystem(anomalySystem) {
    
    if (!m_anomalySystem) {
        Utils::Logger::getInstance().error("TemporalCollisionHandler created with null anomaly system");
    } else {
        Utils::Logger::getInstance().info("TemporalCollisionHandler initialized");
    }
}

TemporalCollisionHandler::~TemporalCollisionHandler() {
    // No resources to clean up
}

void TemporalCollisionHandler::Update(float deltaTime) {
    if (!m_anomalySystem) return;
    
    // Clear previous combination areas
    m_combinationAreas.clear();
    
    // Update spatial grid for efficient collision detection
    updateSpatialGrid();
    
    // Detect collisions using the spatial grid
    detectEntityAnomalyCollisions(deltaTime);
    detectAnomalyOverlaps(deltaTime);
    resolveAnomalyCombinations(deltaTime);
}

// Convert world position to grid coordinates
std::pair<int, int> TemporalCollisionHandler::worldToGrid(const glm::vec3& position) const {
    // Convert world coordinates to grid coordinates
    int gridX = static_cast<int>(std::floor(position.x / CELL_SIZE)) + GRID_SIZE / 2;
    int gridZ = static_cast<int>(std::floor(position.z / CELL_SIZE)) + GRID_SIZE / 2;
    
    // Clamp to grid bounds
    gridX = std::max(0, std::min(GRID_SIZE - 1, gridX));
    gridZ = std::max(0, std::min(GRID_SIZE - 1, gridZ));
    
    return {gridX, gridZ};
}

// Insert an anomaly into the spatial grid
void TemporalCollisionHandler::insertAnomalyIntoGrid(const TemporalAnomaly& anomaly) {
    const glm::vec3& position = anomaly.GetPosition();
    float radius = anomaly.GetRadius();
    int anomalyId = anomaly.GetIdentifier();
    
    // Calculate the grid cells this anomaly overlaps
    // We need to check all cells that the anomaly's sphere might touch
    float minX = position.x - radius;
    float maxX = position.x + radius;
    float minZ = position.z - radius;
    float maxZ = position.z + radius;
    
    auto minCell = worldToGrid(glm::vec3(minX, 0, minZ));
    auto maxCell = worldToGrid(glm::vec3(maxX, 0, maxZ));
    
    // Add anomaly to all cells it overlaps
    for (int x = minCell.first; x <= maxCell.first; ++x) {
        for (int z = minCell.second; z <= maxCell.second; ++z) {
            if (x >= 0 && x < GRID_SIZE && z >= 0 && z < GRID_SIZE) {
                m_spatialGrid[x][z].anomalyIds.push_back(anomalyId);
            }
        }
    }
}

// Insert an entity into the spatial grid
void TemporalCollisionHandler::insertEntityIntoGrid(const AffectedEntity& entity) {
    const glm::vec3& position = entity.GetPosition();
    std::string entityIdStr = entityIdToString(entity.GetId());
    
    // Get the cell containing this entity
    auto cell = worldToGrid(position);
    
    // Add entity to the cell
    m_spatialGrid[cell.first][cell.second].entityIds.push_back(entityIdStr);
}

// Update the spatial partitioning grid
void TemporalCollisionHandler::updateSpatialGrid() {
    // Clear the grid
    for (auto& row : m_spatialGrid) {
        for (auto& cell : row) {
            cell.clear();
        }
    }
    
    // Insert active anomalies into the grid
    const auto& anomalies = m_anomalySystem->GetAnomalies();
    for (const auto& anomalyPtr : anomalies) {
        if (!anomalyPtr || !anomalyPtr->IsActive()) continue;
        insertAnomalyIntoGrid(*anomalyPtr);
    }
    
    // Insert entities into the grid
    const auto& entities = m_anomalySystem->GetEntities();
    for (const auto& entity : entities) {
        if (!entity) continue;
        insertEntityIntoGrid(*entity);
    }
}

// Helper method for consistent entity ID conversion
std::string TemporalCollisionHandler::entityIdToString(int entityId) const {
    return std::to_string(entityId);
}

void TemporalCollisionHandler::detectEntityAnomalyCollisions(float deltaTime) {
    if (!m_anomalySystem) {
        Utils::Logger::getInstance().error("Cannot detect entity-anomaly collisions without anomaly system");
        return;
    }
    
    const auto& entities = m_anomalySystem->GetEntities();
    const auto& anomalies = m_anomalySystem->GetAnomalies();
    
    // Store current anomalies affecting each entity
    std::unordered_map<std::string, std::unordered_set<std::string>> currentEntityAnomalyMap;
    
    // Process each cell in the grid
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int z = 0; z < GRID_SIZE; ++z) {
            const auto& cell = m_spatialGrid[x][z];
            
            // Skip empty cells
            if (cell.entityIds.empty() || cell.anomalyIds.empty()) continue;
            
            // For each entity in this cell
            for (const auto& entityIdStr : cell.entityIds) {
                // Find the entity
                int entityId = std::stoi(entityIdStr);
                std::shared_ptr<AffectedEntity> entity = nullptr;
                
                for (const auto& e : entities) {
                    if (e && e->GetId() == entityId) {
                        entity = e;
                        break;
                    }
                }
                
                if (!entity) continue;
                
                const glm::vec3 entityPos = entity->GetPosition();
                
                // Check against anomalies in this cell
                for (int anomalyId : cell.anomalyIds) {
                    // Find the anomaly
                    std::shared_ptr<TemporalAnomaly> anomaly = m_anomalySystem->FindAnomaly(anomalyId);
                    if (!anomaly || !anomaly->IsActive()) continue;
                    
                    // Check if entity is inside anomaly
                    if (anomaly->ContainsPoint(entityPos)) {
                        // Record that this anomaly affects this entity
                        currentEntityAnomalyMap[entityIdStr].insert(entityIdToString(anomalyId));
                        
                        // Apply the anomaly effect
                        entity->ApplyTemporalEffect(anomaly->GetEffect());
                    }
                }
            }
        }
    }
    
    // Check for entities entering new anomalies
    if (m_entityEnterCallback) {
        for (const auto& [entityIdStr, anomalyIds] : currentEntityAnomalyMap) {
            auto& previousAnomalies = m_entityAnomalyMap[entityIdStr]; // Get or create entry
            
            for (const auto& anomalyIdStr : anomalyIds) {
                // If this anomaly wasn't affecting the entity before
                if (previousAnomalies.find(anomalyIdStr) == previousAnomalies.end()) {
                    // Entity entered a new anomaly
                    int anomalyId = std::stoi(anomalyIdStr);
                    std::shared_ptr<TemporalAnomaly> anomaly = m_anomalySystem->FindAnomaly(anomalyId);
                    if (anomaly) {
                        m_entityEnterCallback(entityIdStr, *anomaly);
                    }
                }
            }
        }
    }
    
    // Check for entities exiting anomalies
    if (m_entityExitCallback) {
        for (const auto& [entityIdStr, previousAnomalyIds] : m_entityAnomalyMap) {
            auto currentIt = currentEntityAnomalyMap.find(entityIdStr);
            const auto& currentAnomalyIds = (currentIt != currentEntityAnomalyMap.end()) 
                                         ? currentIt->second 
                                         : std::unordered_set<std::string>();
            
            for (const auto& anomalyIdStr : previousAnomalyIds) {
                // If this anomaly was affecting the entity before but not now
                if (currentAnomalyIds.find(anomalyIdStr) == currentAnomalyIds.end()) {
                    // Entity exited an anomaly
                    int anomalyId = std::stoi(anomalyIdStr);
                    std::shared_ptr<TemporalAnomaly> anomaly = m_anomalySystem->FindAnomaly(anomalyId);
                    if (anomaly) {
                        m_entityExitCallback(entityIdStr, *anomaly);
                    }
                }
            }
        }
    }
    
    // Update the map with current state
    m_entityAnomalyMap = std::move(currentEntityAnomalyMap);
}

void TemporalCollisionHandler::detectAnomalyOverlaps(float deltaTime) {
    if (!m_anomalySystem) {
        Utils::Logger::getInstance().error("Cannot detect anomaly overlaps without anomaly system");
        return;
    }
    
    // Process each cell in the grid
    for (int x = 0; x < GRID_SIZE; ++x) {
        for (int z = 0; z < GRID_SIZE; ++z) {
            const auto& cell = m_spatialGrid[x][z];
            
            // Skip cells with fewer than 2 anomalies
            if (cell.anomalyIds.size() < 2) continue;
            
            // Check for overlaps between anomalies in this cell
            for (size_t i = 0; i < cell.anomalyIds.size(); ++i) {
                int anomalyId1 = cell.anomalyIds[i];
                std::shared_ptr<TemporalAnomaly> anomaly1 = m_anomalySystem->FindAnomaly(anomalyId1);
                if (!anomaly1 || !anomaly1->IsActive()) continue;
                
                for (size_t j = i + 1; j < cell.anomalyIds.size(); ++j) {
                    int anomalyId2 = cell.anomalyIds[j];
                    
                    // Skip comparing an anomaly with itself
                    if (anomalyId1 == anomalyId2) continue;
                    
                    std::shared_ptr<TemporalAnomaly> anomaly2 = m_anomalySystem->FindAnomaly(anomalyId2);
                    if (!anomaly2 || !anomaly2->IsActive()) continue;
                    
                    // Check for overlap
                    if (anomaly1->Overlaps(*anomaly2)) {
                        // Anomalies overlap
                        if (m_anomaliesOverlapCallback) {
                            m_anomaliesOverlapCallback(*anomaly1, *anomaly2);
                        }
                        
                        // Calculate the center point of overlap
                        const glm::vec3 center1 = anomaly1->GetPosition();
                        const glm::vec3 center2 = anomaly2->GetPosition();
                        const float radius1 = anomaly1->GetRadius();
                        const float radius2 = anomaly2->GetRadius();
                        
                        // Calculate distance between centers
                        const float distance = glm::distance(center1, center2);
                        
                        // Calculate overlap point - weighted toward the larger anomaly
                        glm::vec3 overlapCenter;
                        if (distance > 0.001f && (radius1 + radius2) > 0.001f) {
                            const float weight1 = radius1 / (radius1 + radius2);
                            const float weight2 = radius2 / (radius1 + radius2);
                            overlapCenter = center1 * weight2 + center2 * weight1;
                        } else {
                            // Centers are basically at the same point or radii are zero
                            overlapCenter = center1;
                        }
                        
                        // Calculate combined effect
                        CombinedAnomalyEffect combinedEffect = combineAnomalyEffects(*anomaly1, *anomaly2);
                        
                        // Store this combination area
                        m_combinationAreas.emplace_back(overlapCenter, combinedEffect);
                    }
                }
            }
        }
    }
}

void TemporalCollisionHandler::resolveAnomalyCombinations(float deltaTime) {
    if (!m_anomalySystem) {
        Utils::Logger::getInstance().error("Cannot resolve combinations without anomaly system");
        return;
    }

    // Process the combination areas we've identified
    for (const auto& combination : m_combinationAreas) {
        const auto& center = combination.first;
        const auto& effect = combination.second;
        
        // Determine overlap radius (simplified as average of source anomalies)
        float radius = 0.0f;
        const auto& anomalies = m_anomalySystem->GetAnomalies();
        std::vector<std::shared_ptr<TemporalAnomaly>> overlappingAnomalies;
        
        // Find all active anomalies at this point for radius calculation
        for (const auto& anomalyPtr : anomalies) {
            if (!anomalyPtr || !anomalyPtr->IsActive()) continue;

            if (anomalyPtr->ContainsPoint(center)) {
                radius += anomalyPtr->GetRadius();
                overlappingAnomalies.push_back(anomalyPtr);
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
        const auto& entities = m_anomalySystem->GetEntities();
        for (const auto& entity : entities) {
            if (!entity) continue;
            
            const glm::vec3 entityPos = entity->GetPosition();
            
            // Check if entity is in the combination area
            const float distanceSquared = glm::distance2(center, entityPos);
            if (distanceSquared <= (radius * radius)) {
                // Create a temporary effect structure with an integer sourceId
                AnomalyEffect combinedAnomalyEffect(
                    effect.resultType,
                    effect.timeDistortion,
                    center,
                    radius,
                    deltaTime,  // Just for this frame
                    -1          // Use -1 as a special ID for combined effects
                );
                
                // Apply the combined effect
                entity->ApplyTemporalEffect(combinedAnomalyEffect);
            }
        }
    }
}

CombinedAnomalyEffect TemporalCollisionHandler::combineAnomalyEffects(
    const TemporalAnomaly& a1, const TemporalAnomaly& a2) const {
    
    AnomalyType type1 = a1.GetType();
    AnomalyType type2 = a2.GetType();
    float distortion1 = a1.GetTimeDistortion();
    float distortion2 = a2.GetTimeDistortion();
    
    // Store source types
    std::vector<AnomalyType> sourceTypes = {type1, type2};
    
    // Determine the resulting anomaly type and distortion factor
    
    // Special case: If either anomaly is a rift, the result is a rift
    if (type1 == AnomalyType::RIFT || type2 == AnomalyType::RIFT) {
        float combinedDistortion = std::max(distortion1, distortion2) * 1.2f; // Amplify the effect
        return CombinedAnomalyEffect(AnomalyType::RIFT, combinedDistortion, sourceTypes);
    }
    
    // Special case: If either anomaly is stasis, weakened stasis dominates
    if (type1 == AnomalyType::STASIS || type2 == AnomalyType::STASIS) {
        // Stasis dominates but is weakened (80% of original distortion)
        float stasisDistortion = (type1 == AnomalyType::STASIS) ? distortion1 : distortion2;
        return CombinedAnomalyEffect(AnomalyType::STASIS, stasisDistortion * 0.8f, sourceTypes);
    }
    
    // Dilation and Acceleration partially cancel each other
    if ((type1 == AnomalyType::DILATION && type2 == AnomalyType::ACCELERATION) ||
        (type1 == AnomalyType::ACCELERATION && type2 == AnomalyType::DILATION)) {
        
        // Calculate net distortion (one slows, the other speeds up)
        float netDistortion = std::abs(distortion1 - distortion2);
        
        // Determine dominant type
        AnomalyType dominantType = (std::abs(distortion1) > std::abs(distortion2)) ? type1 : type2;
        
        // If they're very close to equal, they mostly cancel out, leaving mild instability
        if (netDistortion < 0.2f) {
            return CombinedAnomalyEffect(AnomalyType::INSTABILITY, 0.3f, sourceTypes);
        }
        
        return CombinedAnomalyEffect(dominantType, netDistortion * 0.7f, sourceTypes);
    }
    
    // Dilation/Acceleration + Reversal creates a rift due to time flow conflict
    if (((type1 == AnomalyType::DILATION || type1 == AnomalyType::ACCELERATION) && type2 == AnomalyType::REVERSAL) ||
        ((type2 == AnomalyType::DILATION || type2 == AnomalyType::ACCELERATION) && type1 == AnomalyType::REVERSAL)) {
        
        // Creates a more powerful rift when time flow is completely opposite
        float combinedDistortion = std::max(distortion1, distortion2) * 1.3f;
        return CombinedAnomalyEffect(AnomalyType::RIFT, combinedDistortion, sourceTypes);
    }
    
    // Same types intensify the effect
    if (type1 == type2) {
        float combinedDistortion;
        
        if (type1 == AnomalyType::DILATION) {
            // Multiplicative effect for dilation
            combinedDistortion = distortion1 * distortion2;
        } else if (type1 == AnomalyType::ACCELERATION) {
            // Additive effect for acceleration
            combinedDistortion = distortion1 + distortion2;
        } else if (type1 == AnomalyType::REVERSAL) {
            // Reversals cancel each other, become unstable acceleration
            return CombinedAnomalyEffect(AnomalyType::ACCELERATION, 1.5f, sourceTypes);
        } else {
            // Default intensification for other types
            combinedDistortion = std::max(distortion1, distortion2) * 1.2f;
        }
        
        return CombinedAnomalyEffect(type1, combinedDistortion, sourceTypes);
    }
    
    // Default: Average distortions, select dominant type
    float avgDistortion = (distortion1 + distortion2) / 2.0f;
    AnomalyType dominantType = GetCombinedType(type1, type2);
    
    return CombinedAnomalyEffect(dominantType, avgDistortion, sourceTypes);
}

CombinedAnomalyEffect TemporalCollisionHandler::combineMultipleAnomalyEffects(
    const std::vector<const TemporalAnomaly*>& anomalies) const {
    
    if (anomalies.empty()) {
        return CombinedAnomalyEffect(AnomalyType::DILATION, 1.0f);
    }
    
    if (anomalies.size() == 1) {
        return CombinedAnomalyEffect(
            anomalies[0]->GetType(), 
            anomalies[0]->GetTimeDistortion(),
            {anomalies[0]->GetType()}
        );
    }
    
    // For 2 anomalies, use the dedicated combination function
    if (anomalies.size() == 2) {
        return combineAnomalyEffects(*anomalies[0], *anomalies[1]);
    }
    
    // For more than 2 anomalies, process them in pairs
    std::vector<AnomalyType> sourceTypes;
    for (const auto& anomaly : anomalies) {
        sourceTypes.push_back(anomaly->GetType());
    }
    
    // Start with the first anomaly
    CombinedAnomalyEffect result(
        anomalies[0]->GetType(), 
        anomalies[0]->GetTimeDistortion(),
        {anomalies[0]->GetType()}
    );
    
    // Combine with each subsequent anomaly
    for (size_t i = 1; i < anomalies.size(); ++i) {
        // Create a temporary anomaly to represent the current combined effect
        TemporalAnomaly tempAnomaly(
            -1, // Use -1 as special ID for temporary anomalies
            result.resultType,
            result.timeDistortion,
            glm::vec3(0.0f), // Position not used in combination logic
            1.0f,            // Radius not used in combination logic
            1.0f             // Duration not used in combination logic
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
    const auto& allAnomalies = m_anomalySystem->GetAnomalies();
    
    for (const auto& anomalyPtr : allAnomalies) {
        if (!anomalyPtr) continue;
        
        std::string anomalyIdStr = entityIdToString(anomalyPtr->GetIdentifier());
        if (anomalyIds.find(anomalyIdStr) != anomalyIds.end()) {
            // Create a temporary TemporalAnomaly copy from pointer
            int id = anomalyPtr->GetIdentifier();
            AnomalyType type = anomalyPtr->GetType();
            float distortion = anomalyPtr->GetTimeDistortion();
            glm::vec3 position = anomalyPtr->GetPosition();
            float radius = anomalyPtr->GetRadius();
            float duration = anomalyPtr->GetDuration();
            
            // Create a new anomaly with the same properties
            TemporalAnomaly anomaly(id, type, distortion, position, radius, duration);
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

void TemporalCollisionHandler::ProcessCollisions(const std::vector<std::shared_ptr<TemporalAnomaly>>& anomalies) {
    // Implementation for handling collisions between anomalies
    // This would create new anomalies, modify existing ones, etc.
    
    // For demonstration purposes, just log the anomalies
    Utils::Logger::getInstance().info("Processing collisions among " + std::to_string(anomalies.size()) + " anomalies");
    
    // In a real implementation, you would:
    // 1. Find all overlapping anomalies
    // 2. Calculate combined effects
    // 3. Apply new effects to entities in the overlap region
}

AnomalyEffect TemporalCollisionHandler::CalculateCombinedEffect(const std::vector<AnomalyEffect>& effects) const {
    if (effects.empty()) {
        return AnomalyEffect();
    }
    
    if (effects.size() == 1) {
        return effects[0];
    }
    
    // Start with the first effect as a base
    AnomalyEffect combined = effects[0];
    
    // Special case: if any effect is STASIS, it dominates
    for (const auto& effect : effects) {
        if (effect.type == AnomalyType::STASIS) {
            combined.type = AnomalyType::STASIS;
            combined.timeDistortion = 0.0f;
            break;
        }
    }
    
    // For other effects, combine them logically
    float combinedDistortion = 0.0f;
    int distortionCount = 0;
    
    for (const auto& effect : effects) {
        // Skip STASIS since we already handled it
        if (effect.type == AnomalyType::STASIS) {
            continue;
        }
        
        // For time distortion effects, average them
        if (effect.type == AnomalyType::DILATION || effect.type == AnomalyType::ACCELERATION) {
            combinedDistortion += effect.timeDistortion;
            distortionCount++;
        }
        
        // For REVERSAL, it toggles the direction (simplified)
        if (effect.type == AnomalyType::REVERSAL) {
            combined.timeDistortion *= -1.0f;
        }
        
        // RIFT and INSTABILITY are more complex, but we'll simplify for the demo
    }
    
    // Apply the averaged distortion if applicable
    if (distortionCount > 0 && combined.type != AnomalyType::STASIS) {
        combined.timeDistortion = combinedDistortion / distortionCount;
    }
    
    // Position is the average of all effects
    glm::vec3 totalPos(0.0f);
    for (const auto& effect : effects) {
        totalPos += effect.position;
    }
    combined.position = totalPos / static_cast<float>(effects.size());
    
    // Radius is the maximum
    float maxRadius = 0.0f;
    for (const auto& effect : effects) {
        maxRadius = std::max(maxRadius, effect.radius);
    }
    combined.radius = maxRadius;
    
    // Duration is the minimum
    float minDuration = combined.duration;
    for (const auto& effect : effects) {
        minDuration = std::min(minDuration, effect.duration);
    }
    combined.duration = minDuration;
    
    return combined;
}

bool TemporalCollisionHandler::AreColliding(const TemporalAnomaly& a, const TemporalAnomaly& b) const {
    const auto& effectA = a.GetEffect();
    const auto& effectB = b.GetEffect();
    
    float distance = glm::distance(effectA.position, effectB.position);
    return distance < (effectA.radius + effectB.radius);
}

AnomalyType TemporalCollisionHandler::GetCombinedType(AnomalyType type1, AnomalyType type2) const {
    // Time dilating anomalies
    if (type1 == AnomalyType::TIME_SLOW && type2 == AnomalyType::TIME_SLOW) {
        return AnomalyType::TIME_SLOW; // Enhanced slowing
    }
    
    if (type1 == AnomalyType::TIME_FAST && type2 == AnomalyType::TIME_FAST) {
        return AnomalyType::TIME_FAST; // Enhanced acceleration
    }
    
    if ((type1 == AnomalyType::TIME_SLOW && type2 == AnomalyType::TIME_FAST) || 
        (type1 == AnomalyType::TIME_FAST && type2 == AnomalyType::TIME_SLOW)) {
        // Opposing effects partially cancel, but create instability
        return AnomalyType::INSTABILITY;
    }
    
    // ... other combinations ...
    
    // Default: return the more "powerful" effect
    if (type1 == AnomalyType::STASIS || type2 == AnomalyType::STASIS) {
        return AnomalyType::STASIS;
    }
    
    if (type1 == AnomalyType::RIFT || type2 == AnomalyType::RIFT) {
        return AnomalyType::RIFT;
    }
    
    // Fallback
    return (type1 > type2) ? type1 : type2;
}

float TemporalCollisionHandler::CalculateCombinedTimeDistortion(const std::vector<AnomalyEffect>& effects) const {
    // STASIS always results in zero distortion
    for (const auto& effect : effects) {
        if (effect.type == AnomalyType::STASIS) {
            return 0.0f;
        }
    }
    
    // Calculate average distortion
    float totalDistortion = 0.0f;
    for (const auto& effect : effects) {
        // For REVERSAL, flip the sign
        if (effect.type == AnomalyType::REVERSAL) {
            totalDistortion -= effect.timeDistortion;
        } else {
            totalDistortion += effect.timeDistortion;
        }
    }
    
    return totalDistortion / static_cast<float>(effects.size());
}

} // namespace Temporal
} // namespace TurtleEngine 