#include "../include/HealthSystem.hpp"
// Remove the include for the engine's ParticleSystem
// #include "../../include/ParticleSystem.hpp"
#include "../../graphics/include/ParticleSystem.hpp" // Include the full definition for implementation
#include <iostream>
#include <sstream>
#include <iomanip>
#include <algorithm>

#ifndef STANDALONE_TEST // Keep this for now, might be needed elsewhere
#include <glm/gtc/random.hpp>
#endif

namespace TurtleEngine {
namespace Combat {

// Constants for Resilience
namespace ResilienceConstants {
    constexpr float CRITICAL_HIT_RESISTANCE_IGNORE = 0.5f; // Critical hits ignore 50% of resistance
}

// ---- Resilience Implementation ----

Resilience::Resilience() : m_flatReduction(0.0f), m_shield(0.0f) {
    // Initialize resistances to 0 (no effect)
    for (int i = 0; i < static_cast<int>(DamageType::COUNT); ++i) {
        m_resistances[static_cast<DamageType>(i)] = 0.0f;
    }
}

float Resilience::processDamage(const DamageInfo& damage) {
    // Apply shield first if available
    float remainingDamage = damage.amount;
    if (m_shield > 0.0f) {
        remainingDamage = processShieldDamage(remainingDamage);
    }
    
    // Apply flat reduction
    remainingDamage = std::max(0.0f, remainingDamage - m_flatReduction);
    
    // Apply type-specific resistance
    float resistance = getResistance(damage.type);
    remainingDamage *= (1.0f - resistance);
    
    // Critical hits ignore a portion of resistance
    if (damage.isCritical) {
        float resistanceDamage = damage.amount * resistance * ResilienceConstants::CRITICAL_HIT_RESISTANCE_IGNORE;
        remainingDamage += resistanceDamage;
    }
    
    // Ensure damage doesn't go negative
    return std::max(0.0f, remainingDamage);
}

void Resilience::setResistance(DamageType type, float value) {
    // Clamp resistance to valid range
    m_resistances[type] = std::clamp(value, 0.0f, 1.0f);
    
    std::cout << "[Resilience] Set " << static_cast<int>(type) 
              << " resistance to " << m_resistances[type] << std::endl;
}

float Resilience::getResistance(DamageType type) const {
    auto it = m_resistances.find(type);
    if (it != m_resistances.end()) {
        return it->second;
    }
    return 0.0f; // Default to no resistance if type not found
}

void Resilience::setFlatReduction(float value) {
    m_flatReduction = std::max(0.0f, value);
    
    std::cout << "[Resilience] Set flat reduction to " 
              << m_flatReduction << std::endl;
}

float Resilience::getFlatReduction() const {
    return m_flatReduction;
}

void Resilience::setShield(float value) {
    m_shield = std::max(0.0f, value);
    
    std::cout << "[Resilience] Set shield to " << m_shield << std::endl;
}

float Resilience::getShield() const {
    return m_shield;
}

float Resilience::processShieldDamage(float amount) {
    if (m_shield <= 0.0f) {
        return amount; // No shield, full damage passes through
    }
    
    if (amount <= m_shield) {
        // Shield absorbs all damage
        m_shield -= amount;
        return 0.0f;
    } else {
        // Shield is depleted, remaining damage passes through
        float remainingDamage = amount - m_shield;
        m_shield = 0.0f;
        return remainingDamage;
    }
}

// ---- HealthComponent Implementation ----

HealthComponent::HealthComponent(float maxHealth, std::shared_ptr<Graphics::ParticleSystem> particleSystem)
    : m_currentHealth(maxHealth),
      m_maxHealth(maxHealth),
      m_resilience(std::make_unique<Resilience>()),
      m_position(glm::vec3(0.0f)),
      m_debugVisualizationEnabled(false),
      m_particleSystem(particleSystem),
      m_isVisible(true),
      m_debugInfo("")
{
    // Initialize callbacks with empty functions
    m_damageCallback = [](const DamageInfo&, float) {};
    m_healingCallback = [](float, const glm::vec3&) {};
    m_deathCallback = []() {};
}

float HealthComponent::applyDamage(const DamageInfo& damage) {
    if (!isAlive()) {
        return 0.0f; // Already dead, no damage to apply
    }
    
    // Process damage through resilience system
    float actualDamage = m_resilience->processDamage(damage);
    
    // Apply damage to health
    m_currentHealth = std::max(0.0f, m_currentHealth - actualDamage);
    
    std::cout << "[HealthComponent] Took " << actualDamage 
              << " damage (" << damage.amount << " raw). Health: " 
              << m_currentHealth << "/" << m_maxHealth << std::endl;
    
    // Create visual effects
    createDamageParticles(damage, actualDamage);
    
    // Call damage callback if registered
    if (m_damageCallback) {
        m_damageCallback(damage, actualDamage);
    }
    
    // Check for death
    if (m_currentHealth <= 0.0f && m_deathCallback) {
        m_deathCallback();
    }
    
    // Update debug info
    updateDebugInfo();
    
    return actualDamage;
}

float HealthComponent::applyHealing(float amount, const glm::vec3& source) {
    if (!isAlive() || amount <= 0.0f) {
        return 0.0f; // Cannot heal if dead or invalid amount
    }
    
    float oldHealth = m_currentHealth;
    m_currentHealth = std::min(m_maxHealth, m_currentHealth + amount);
    float actualHealing = m_currentHealth - oldHealth;
    
    std::cout << "[HealthComponent] Healed " << actualHealing 
              << ". Health: " << m_currentHealth << "/" 
              << m_maxHealth << std::endl;
    
    // Create visual effects
    createHealingParticles(actualHealing, source);
    
    // Call healing callback if registered
    if (m_healingCallback) {
        m_healingCallback(actualHealing, source);
    }
    
    // Update debug info
    updateDebugInfo();
    
    return actualHealing;
}

float HealthComponent::getCurrentHealth() const {
    return m_currentHealth;
}

float HealthComponent::getMaxHealth() const {
    return m_maxHealth;
}

float HealthComponent::getHealthPercentage() const {
    if (m_maxHealth <= 0.0f) {
        return 0.0f;
    }
    return m_currentHealth / m_maxHealth;
}

bool HealthComponent::isAlive() const {
    return m_currentHealth > 0.0f;
}

Resilience& HealthComponent::getResilience() {
    return *m_resilience;
}

void HealthComponent::setMaxHealth(float newMaxHealth) {
    if (newMaxHealth <= 0.0f) {
        std::cerr << "[HealthComponent] Error: Cannot set max health to zero or negative." << std::endl;
        return;
    }
    
    float oldMaxHealth = m_maxHealth;
    float healthRatio = (oldMaxHealth > 0.0f) ? (m_currentHealth / oldMaxHealth) : 1.0f;
    
    m_maxHealth = newMaxHealth;
    m_currentHealth = m_maxHealth * healthRatio; // Adjust current health proportionally
    
    std::cout << "[HealthComponent] Max health set to " << m_maxHealth 
              << ". Current health adjusted to " << m_currentHealth << std::endl;
    
    updateDebugInfo();
}

void HealthComponent::setDamageCallback(std::function<void(const DamageInfo&, float)> callback) {
    m_damageCallback = callback;
}

void HealthComponent::setHealingCallback(std::function<void(float, const glm::vec3&)> callback) {
    m_healingCallback = callback;
}

void HealthComponent::setDeathCallback(std::function<void()> callback) {
    m_deathCallback = callback;
}

void HealthComponent::setPosition(const glm::vec3& position) {
    m_position = position;
}

const glm::vec3& HealthComponent::getPosition() const {
    return m_position;
}

void HealthComponent::enableDebugVisualization(bool enabled) {
    m_debugVisualizationEnabled = enabled;
    
    std::cout << "[HealthComponent] Debug visualization " 
              << (enabled ? "enabled" : "disabled") << std::endl;
    
    updateDebugInfo();
}

bool HealthComponent::isDebugVisualizationEnabled() const {
    return m_debugVisualizationEnabled;
}

std::string HealthComponent::getDebugInfo() const {
    return m_debugInfo;
}

void HealthComponent::createDamageParticles(const DamageInfo& damage, float actualDamage) {
    if (!m_particleSystem || !m_isVisible) return;

    glm::vec4 particleColor;
    float particleSize = 0.5f;
    float particleLife = 1.0f;
    glm::vec3 baseVelocity(0.0f, 1.0f, 0.0f); // Default velocity upwards

    // Determine properties based on damage type
    switch (damage.type) {
        case DamageType::PLASMA:
            particleColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
            particleSize = 0.5f;
            particleLife = 1.0f;
            break;
        case DamageType::KINETIC:
            particleColor = glm::vec4(0.5f, 0.5f, 0.5f, 1.0f); // Gray
            particleSize = 0.3f;
            particleLife = 0.5f;
            break;
        case DamageType::ENERGY:
            particleColor = glm::vec4(1.0f, 1.0f, 0.0f, 1.0f); // Yellow
            particleSize = 0.4f;
            particleLife = 0.7f;
            break;
        default:
            particleColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red for unknown/physical
            particleSize = 0.4f;
            particleLife = 0.6f;
            break;
    }

    // Emit a particle using the graphics system's method
#ifndef STANDALONE_TEST
    // Add some randomness if not in a basic test
    glm::vec3 randomVelocityOffset = glm::sphericalRand(0.5f);
    baseVelocity += randomVelocityOffset;
#endif

    m_particleSystem->emit(
        m_position,       // Position
        baseVelocity,     // Velocity (simple upwards for now)
        particleColor,    // Color (as vec4)
        particleSize,     // Size
        particleLife      // Life (using duration)
    );
}

void HealthComponent::createHealingParticles(float amount, const glm::vec3& source) {
    if (!m_particleSystem || !m_isVisible) return;

    // Create healing particles
    glm::vec4 particleColor = glm::vec4(0.0f, 1.0f, 1.0f, 1.0f); // Cyan
    float particleSize = 0.6f;
    float particleLife = 1.0f;
    glm::vec3 velocity = glm::vec3(0.0f, 0.5f, 0.0f); // Gentle upwards float

    m_particleSystem->emit(
        m_position,
        velocity,
        particleColor,
        particleSize,
        particleLife
    );
}

void HealthComponent::updateDebugInfo() {
    if (!m_debugVisualizationEnabled) {
        m_debugInfo = "";
        return;
    }
    
    std::stringstream ss;
    ss << "Health: " << static_cast<int>(m_currentHealth) 
       << "/" << static_cast<int>(m_maxHealth)
       << " (" << static_cast<int>(getHealthPercentage() * 100.0f) << "%)";
    
    float shield = m_resilience->getShield();
    if (shield > 0.0f) {
        ss << " | Shield: " << static_cast<int>(shield);
    }
    
    float physRes = m_resilience->getResistance(DamageType::PHYSICAL);
    float plasmaRes = m_resilience->getResistance(DamageType::PLASMA);
    if (physRes > 0.0f || plasmaRes > 0.0f) {
        ss << " | Res: ";
        if (physRes > 0.0f) {
            ss << "P:" << static_cast<int>(physRes * 100.0f) << "% ";
        }
        if (plasmaRes > 0.0f) {
            ss << "E:" << static_cast<int>(plasmaRes * 100.0f) << "%";
        }
    }
    
    m_debugInfo = ss.str();
}

} // namespace Combat
} // namespace TurtleEngine
