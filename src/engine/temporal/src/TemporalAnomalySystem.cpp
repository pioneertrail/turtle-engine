#include "../include/TemporalAnomalySystem.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <glm/gtx/string_cast.hpp>

namespace TurtleEngine {
namespace Temporal {

// TemporalAnomaly Implementation
TemporalAnomaly::TemporalAnomaly(const AnomalyEffect& effect)
    : m_effect(effect), 
      m_remainingDuration(effect.duration),
      m_intensityFactor(1.0f),
      m_isActive(true),
      m_stabilityFactor(1.0f) {
    std::cout << "[TemporalAnomaly] Created anomaly of type " 
              << static_cast<int>(effect.type) << " at " 
              << glm::to_string(effect.position) << " with radius " 
              << effect.radius << std::endl;
}

void TemporalAnomaly::update(float deltaTime) {
    if (!m_isActive) return;
    
    // Decrease remaining duration
    m_remainingDuration -= deltaTime;
    
    // Check if expired
    if (m_remainingDuration <= 0.0f) {
        m_isActive = false;
        m_remainingDuration = 0.0f;
        std::cout << "[TemporalAnomaly] Anomaly dissipated" << std::endl;
        return;
    }
    
    // Update intensity factor based on remaining duration
    // Anomaly is strongest in the middle of its lifespan
    float lifetimeProgress = 1.0f - (m_remainingDuration / m_effect.duration);
    
    // Parabolic curve: starts low, peaks in middle, ends low
    m_intensityFactor = 1.0f - std::abs(2.0f * lifetimeProgress - 1.0f);
    
    // Fluctuate stability slightly for visual effects
    m_stabilityFactor = 0.8f + 0.2f * sin(lifetimeProgress * 6.28f);
    
    // Debug output
    if (fmod(m_remainingDuration, 1.0f) < deltaTime) {
        std::cout << "[TemporalAnomaly] Remaining: " << std::fixed << std::setprecision(1) 
                  << m_remainingDuration << "s, Intensity: " << m_intensityFactor 
                  << ", Stability: " << m_stabilityFactor << std::endl;
    }
}

bool TemporalAnomaly::affectsEntity(const AffectedEntity& entity) const {
    if (!m_isActive) return false;
    
    // Check if entity is within radius
    float distance = glm::distance(entity.getPosition(), m_effect.position);
    return distance <= m_effect.radius;
}

void TemporalAnomaly::applyToEntity(AffectedEntity& entity, float deltaTime) {
    if (!m_isActive || !affectsEntity(entity)) return;
    
    // Calculate distance factor: closer = stronger effect
    float distance = glm::distance(entity.getPosition(), m_effect.position);
    float distanceFactor = 1.0f - (distance / m_effect.radius);
    
    // Apply falloff with smooth transition at edge
    distanceFactor = glm::smoothstep(0.0f, 1.0f, distanceFactor);
    
    // Adjust effect based on intensity and distance
    AnomalyEffect adjustedEffect = m_effect;
    adjustedEffect.timeDistortion *= m_intensityFactor * distanceFactor;
    
    // Apply to entity
    entity.applyTemporalEffect(adjustedEffect, deltaTime);
}

bool TemporalAnomaly::isActive() const {
    return m_isActive;
}

glm::vec3 TemporalAnomaly::getPosition() const {
    return m_effect.position;
}

float TemporalAnomaly::getRadius() const {
    return m_effect.radius;
}

AnomalyType TemporalAnomaly::getType() const {
    return m_effect.type;
}

const AnomalyEffect& TemporalAnomaly::getEffect() const {
    return m_effect;
}

float TemporalAnomaly::getRemainingDuration() const {
    return m_remainingDuration;
}

float TemporalAnomaly::getIntensityFactor() const {
    return m_intensityFactor;
}

// TemporalAnomalySystem Implementation
TemporalAnomalySystem::TemporalAnomalySystem() {
    std::cout << "[TemporalAnomalySystem] Initialized" << std::endl;
}

void TemporalAnomalySystem::createAnomaly(const AnomalyEffect& effect) {
    m_anomalies.emplace_back(effect);
    std::cout << "[TemporalAnomalySystem] Created new anomaly, total: " 
              << m_anomalies.size() << std::endl;
    
    // Call debug visualization if registered
    if (m_debugDrawCallback) {
        m_debugDrawCallback(m_anomalies.back());
    }
}

void TemporalAnomalySystem::update(float deltaTime) {
    // Update all anomalies
    for (auto& anomaly : m_anomalies) {
        anomaly.update(deltaTime);
    }
    
    // Process anomaly-entity interactions
    processInteractions(deltaTime);
    
    // Remove expired anomalies
    removeExpiredAnomalies();
}

void TemporalAnomalySystem::registerEntity(std::shared_ptr<AffectedEntity> entity) {
    if (!entity) return;
    
    const std::string& id = entity->getIdentifier();
    m_entities[id] = entity;
    
    std::cout << "[TemporalAnomalySystem] Registered entity: " << id 
              << ", total entities: " << m_entities.size() << std::endl;
}

void TemporalAnomalySystem::unregisterEntity(const std::string& entityId) {
    auto it = m_entities.find(entityId);
    if (it != m_entities.end()) {
        m_entities.erase(it);
        std::cout << "[TemporalAnomalySystem] Unregistered entity: " << entityId 
                  << ", remaining entities: " << m_entities.size() << std::endl;
    }
}

const std::vector<TemporalAnomaly>& TemporalAnomalySystem::getAnomalies() const {
    return m_anomalies;
}

void TemporalAnomalySystem::setDebugDrawCallback(DebugDrawCallback callback) {
    m_debugDrawCallback = callback;
}

size_t TemporalAnomalySystem::getAnomalyCount() const {
    return m_anomalies.size();
}

void TemporalAnomalySystem::clearAnomalies() {
    m_anomalies.clear();
    std::cout << "[TemporalAnomalySystem] Cleared all anomalies" << std::endl;
}

bool TemporalAnomalySystem::isPointAffected(const glm::vec3& point) const {
    for (const auto& anomaly : m_anomalies) {
        if (!anomaly.isActive()) continue;
        
        float distance = glm::distance(point, anomaly.getPosition());
        if (distance <= anomaly.getRadius()) {
            return true;
        }
    }
    return false;
}

float TemporalAnomalySystem::getTimeDistortionAt(const glm::vec3& point) const {
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
                    // Multiple stasis effects stack multiplicatively
                    combinedDistortion *= std::max(0.01f, 1.0f - effectStrength);
                    break;
                    
                case AnomalyType::DILATION:
                    // Dilations slow time
                    combinedDistortion *= std::max(0.01f, 1.0f - effectStrength);
                    break;
                    
                case AnomalyType::ACCELERATION:
                    // Accelerations speed up time
                    combinedDistortion *= (1.0f + effectStrength);
                    break;
                    
                case AnomalyType::REVERSION:
                    // Reversions make time negative
                    combinedDistortion *= -effectStrength;
                    break;
                    
                case AnomalyType::RIFT:
                    // Rifts cause unpredictable time distortion
                    if (distanceFactor > 0.8f) {
                        // Near the center, very unstable
                        combinedDistortion *= std::sin(distanceFactor * 10.0f) * effectStrength;
                    } else {
                        // Further out, general slowdown
                        combinedDistortion *= std::max(0.01f, 1.0f - effectStrength * 0.5f);
                    }
                    break;
            }
            
            pointAffected = true;
        }
    }
    
    return pointAffected ? combinedDistortion : 1.0f;
}

void TemporalAnomalySystem::processInteractions(float deltaTime) {
    for (auto& anomaly : m_anomalies) {
        if (!anomaly.isActive()) continue;
        
        for (auto& [id, entity] : m_entities) {
            if (anomaly.affectsEntity(*entity)) {
                anomaly.applyToEntity(*entity, deltaTime);
            }
        }
    }
}

void TemporalAnomalySystem::removeExpiredAnomalies() {
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
        std::cout << "[TemporalAnomalySystem] Removed " << removedCount 
                  << " expired anomalies, remaining: " << m_anomalies.size() << std::endl;
    }
}

} // namespace Temporal
} // namespace TurtleEngine 