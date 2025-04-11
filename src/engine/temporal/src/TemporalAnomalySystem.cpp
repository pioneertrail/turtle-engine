#include "../include/TemporalAnomalySystem.hpp"
#include <algorithm>
#include <iostream>

namespace TurtleEngine {
namespace Temporal {

TemporalAnomalySystem::TemporalAnomalySystem() {
    std::cout << "Temporal Anomaly System initialized" << std::endl;
}

TemporalAnomalySystem::~TemporalAnomalySystem() {
    // Clean up resources
    m_anomalies.clear();
    m_entities.clear();
}

void TemporalAnomalySystem::createAnomaly(const AnomalyEffect& effect) {
    // Check if anomaly with this identifier already exists
    auto it = std::find_if(m_anomalies.begin(), m_anomalies.end(), 
        [&](const TemporalAnomaly& a) { return a.getIdentifier() == effect.identifier; });
    
    if (it != m_anomalies.end()) {
        std::cout << "Warning: Anomaly with ID " << effect.identifier << " already exists. Updating properties." << std::endl;
        m_anomalies.erase(it);
    }
    
    // Create new anomaly
    m_anomalies.emplace_back(effect);
    
    std::cout << "Created " << getAnomalyTypeName(effect.type) << " anomaly at position "
              << effect.position.x << ", " << effect.position.y << ", " << effect.position.z
              << " with radius " << effect.radius << " and time distortion " << effect.timeDistortion << std::endl;
}

void TemporalAnomalySystem::removeAnomaly(const std::string& identifier) {
    auto it = std::find_if(m_anomalies.begin(), m_anomalies.end(), 
        [&](const TemporalAnomaly& a) { return a.getIdentifier() == identifier; });
    
    if (it != m_anomalies.end()) {
        std::cout << "Removed anomaly " << identifier << std::endl;
        m_anomalies.erase(it);
    }
}

void TemporalAnomalySystem::registerEntity(std::shared_ptr<AffectedEntity> entity) {
    if (!entity) {
        std::cout << "Error: Cannot register null entity" << std::endl;
        return;
    }
    
    const std::string& id = entity->getIdentifier();
    
    if (m_entities.find(id) != m_entities.end()) {
        std::cout << "Warning: Entity " << id << " already registered. Updating reference." << std::endl;
    }
    
    m_entities[id] = entity;
    std::cout << "Registered entity " << id << std::endl;
}

void TemporalAnomalySystem::unregisterEntity(const std::string& identifier) {
    auto it = m_entities.find(identifier);
    
    if (it != m_entities.end()) {
        std::cout << "Unregistered entity " << identifier << std::endl;
        m_entities.erase(it);
    }
}

void TemporalAnomalySystem::update(float deltaTime) {
    // First, update all anomalies and remove expired ones
    m_anomalies.erase(
        std::remove_if(m_anomalies.begin(), m_anomalies.end(),
            [&](TemporalAnomaly& anomaly) {
                return !anomaly.update(deltaTime);
            }),
        m_anomalies.end()
    );
    
    // Then, apply anomaly effects to all entities
    for (auto& entityPair : m_entities) {
        const auto& entityId = entityPair.first;
        auto& entity = entityPair.second;
        
        const glm::vec3 entityPos = entity->getPosition();
        
        // Apply effects from all anomalies that contain this entity
        for (const auto& anomaly : m_anomalies) {
            if (anomaly.containsPoint(entityPos)) {
                entity->applyTemporalEffect(anomaly.getEffect(), deltaTime);
            }
        }
    }
}

const TemporalAnomaly* TemporalAnomalySystem::getAnomalyById(const std::string& identifier) const {
    auto it = std::find_if(m_anomalies.begin(), m_anomalies.end(), 
        [&](const TemporalAnomaly& a) { return a.getIdentifier() == identifier; });
    
    if (it != m_anomalies.end()) {
        return &(*it);
    }
    
    return nullptr;
}

// Helper function to get string representation of anomaly types
std::string getAnomalyTypeName(AnomalyType type) {
    switch (type) {
        case AnomalyType::STASIS: return "STASIS";
        case AnomalyType::DILATION: return "DILATION";
        case AnomalyType::ACCELERATION: return "ACCELERATION";
        case AnomalyType::REVERSION: return "REVERSION";
        case AnomalyType::RIFT: return "RIFT";
        default: return "UNKNOWN";
    }
}

} // namespace Temporal
} // namespace TurtleEngine 