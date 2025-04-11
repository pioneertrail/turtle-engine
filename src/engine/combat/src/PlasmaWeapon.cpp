#include "../include/PlasmaWeapon.hpp"
#include <iostream>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <sstream>

namespace TurtleEngine {
namespace Combat {

PlasmaWeapon::PlasmaWeapon(std::shared_ptr<ParticleSystem> particleSystem, float maxCharge)
    : m_particleSystem(particleSystem),
      m_firingMode(FiringMode::BURST),
      m_currentCharge(0.0f),
      m_maxCharge(maxCharge),
      m_chargeRate(PlasmaWeaponConstants::DEFAULT_CHARGE_RATE),
      m_cooldownTime(PlasmaWeaponConstants::DEFAULT_COOLDOWN_TIME),
      m_currentCooldown(0.0f),
      m_isCharging(false),
      m_plasmaColor(1.0f, 0.7f, 0.1f, 1.0f), // Orange-yellow plasma
      m_particleSize(0.5f),
      m_particleLifetime(2.0f),
      m_particleVelocity(15.0f),
      m_debugVisualizationEnabled(false) {

    if (!m_particleSystem) {
        std::cerr << "[PlasmaWeapon] Error: Null particle system provided during construction!" << std::endl;
        // Consider throwing an exception or setting an error state
    }
    std::cout << "[PlasmaWeapon] Constructed with max charge: " << m_maxCharge << std::endl;
}

PlasmaWeapon::~PlasmaWeapon() {
    std::cout << "[PlasmaWeapon] Destroyed" << std::endl;
}

void PlasmaWeapon::update(float deltaTime) {
    // Update cooldown timer
    if (m_currentCooldown > 0.0f) {
        m_currentCooldown = std::max(0.0f, m_currentCooldown - deltaTime);
    }

    // Update charging if active
    if (m_isCharging) {
        m_currentCharge = std::min(m_maxCharge, m_currentCharge + (m_chargeRate * deltaTime));
    }

    // Update debug information
    if (m_debugVisualizationEnabled) {
        updateDebugInfo();
    }
}

void PlasmaWeapon::render(const glm::mat4& view, const glm::mat4& projection) {
    // Rendering is primarily handled by the particle system
    // Here we can add additional effects like weapon glow or charge indicators

    if (m_debugVisualizationEnabled) {
        // In a real implementation, we would render debug text or indicators here
        std::cout << "[PlasmaWeapon] Debug: " << m_debugStateMessage << std::endl;
    }
}

float PlasmaWeapon::beginCharging() {
    if (m_currentCooldown > 0.0f) {
        // Cannot charge during cooldown
        return m_currentCharge;
    }

    m_isCharging = true;
    std::cout << "[PlasmaWeapon] Charging initiated" << std::endl;
    return m_currentCharge;
}

bool PlasmaWeapon::fire(const glm::vec3& origin, const glm::vec3& direction) {
    if (m_currentCooldown > 0.0f) {
        // Cannot fire during cooldown
        std::cout << "[PlasmaWeapon] Cannot fire - weapon cooling down: " 
                  << m_currentCooldown << "s remaining" << std::endl;
        return false;
    }

    if (m_currentCharge < PlasmaWeaponConstants::MIN_FIRE_CHARGE) {
        // Minimum charge required for firing
        std::cout << "[PlasmaWeapon] Cannot fire - insufficient charge: " 
                  << m_currentCharge << "/" << m_maxCharge << std::endl;
        return false;
    }

    // Determine power based on charge level (normalized to 0.2-1.0 range)
    float normalizedCharge = (m_maxCharge > 0.0f) ? (m_currentCharge / m_maxCharge) : 1.0f;
    float power = PlasmaWeaponConstants::POWER_MIN_NORMALIZATION + 
                  (PlasmaWeaponConstants::POWER_RANGE_NORMALIZATION * normalizedCharge);
    
    std::cout << "[PlasmaWeapon] Firing with power: " << power 
              << " (" << m_currentCharge << "/" << m_maxCharge << ")" << std::endl;
    
    // Create visual effect based on firing mode
    createPlasmaParticles(origin, direction, power);
    
    // Reset charge and set cooldown
    m_currentCharge = 0.0f;
    m_currentCooldown = m_cooldownTime;
    m_isCharging = false;
    
    return true;
}

bool PlasmaWeapon::quickFire(const glm::vec3& origin, const glm::vec3& direction) {
    if (m_currentCooldown > 0.0f) {
        // Cannot quick-fire during cooldown
        std::cout << "[PlasmaWeapon] Cannot quick-fire - weapon cooling down: " 
                  << m_currentCooldown << "s remaining" << std::endl;
        return false;
    }
    
    std::cout << "[PlasmaWeapon] Quick-firing with Flammil gesture" << std::endl;
    
    // Quick-fire always uses a minimum power level
    float power = PlasmaWeaponConstants::QUICK_FIRE_POWER;
    
    // Create visual effect with quick-fire parameters
    createPlasmaParticles(origin, direction, power);
    
    // Set cooldown (reduced for quick-fire)
    m_currentCooldown = m_cooldownTime * PlasmaWeaponConstants::QUICK_FIRE_COOLDOWN_MULTIPLIER;
    
    return true;
}

void PlasmaWeapon::setFiringMode(FiringMode mode) {
    m_firingMode = mode;
    
    // Update visual properties based on firing mode
    switch (m_firingMode) {
        case FiringMode::BURST:
            m_particleLifetime = 2.0f;
            m_particleVelocity = 15.0f;
            m_plasmaColor = glm::vec4(1.0f, 0.7f, 0.1f, 1.0f); // Orange-yellow
            break;
            
        case FiringMode::BEAM:
            m_particleLifetime = 0.5f;
            m_particleVelocity = 25.0f;
            m_plasmaColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
            break;
            
        case FiringMode::CHARGED:
            m_particleLifetime = 3.0f;
            m_particleVelocity = 10.0f;
            m_plasmaColor = glm::vec4(1.0f, 0.3f, 0.0f, 1.0f); // Deep orange
            break;
            
        case FiringMode::SCATTER:
            m_particleLifetime = 1.5f;
            m_particleVelocity = 18.0f;
            m_plasmaColor = glm::vec4(1.0f, 0.6f, 0.1f, 1.0f); // Light orange
            break;
    }
    
    std::cout << "[PlasmaWeapon] Firing mode changed to: " << static_cast<int>(m_firingMode) << std::endl;
}

float PlasmaWeapon::getChargePercentage() const {
    return m_currentCharge / m_maxCharge;
}

bool PlasmaWeapon::isCoolingDown() const {
    return m_currentCooldown > 0.0f;
}

void PlasmaWeapon::enableDebugVisualization(bool enabled) {
    m_debugVisualizationEnabled = enabled;
    std::cout << "[PlasmaWeapon] Debug visualization " << (enabled ? "enabled" : "disabled") << std::endl;
}

// Private methods

void PlasmaWeapon::createPlasmaParticles(const glm::vec3& origin, const glm::vec3& direction, float power) {
    if (!m_particleSystem) {
        std::cerr << "[PlasmaWeapon] Error: Cannot create particles - particle system not initialized!" << std::endl;
        return;
    }

    // Calculate parameters based on power level and firing mode
    int particleCount = static_cast<int>(PlasmaWeaponConstants::BASE_PARTICLE_COUNT + 
                                         (power * PlasmaWeaponConstants::POWER_PARTICLE_MULTIPLIER));
    float spreadAngle = 0.0f;
    
    switch (m_firingMode) {
        case FiringMode::BURST:
            spreadAngle = glm::radians(PlasmaWeaponConstants::BURST_SPREAD_DEGREES);
            break;
            
        case FiringMode::BEAM:
            spreadAngle = glm::radians(PlasmaWeaponConstants::BEAM_SPREAD_DEGREES);
            particleCount = static_cast<int>(particleCount * PlasmaWeaponConstants::BEAM_PARTICLE_MULTIPLIER);
            break;
            
        case FiringMode::CHARGED:
            spreadAngle = glm::radians(PlasmaWeaponConstants::CHARGED_SPREAD_DEGREES);
            particleCount = static_cast<int>(particleCount * PlasmaWeaponConstants::CHARGED_PARTICLE_MULTIPLIER);
            break;
            
        case FiringMode::SCATTER:
            spreadAngle = glm::radians(PlasmaWeaponConstants::SCATTER_SPREAD_DEGREES);
            particleCount = static_cast<int>(particleCount * PlasmaWeaponConstants::SCATTER_PARTICLE_MULTIPLIER);
            break;
    }
    
    // Apply power adjustments
    float velocity = m_particleVelocity * power;
    float size = m_particleSize * (PlasmaWeaponConstants::PARTICLE_LIFETIME_RANDOM_MIN + 
                                  (power * (PlasmaWeaponConstants::POWER_RANGE_NORMALIZATION / 2.0f))); // Size increases with power
    
    // Create a normalized direction vector
    glm::vec3 normDirection = glm::normalize(direction);
    
    // Create orthogonal vectors for spreading particles
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    if (glm::abs(glm::dot(normDirection, up)) > 0.99f) {
        up = glm::vec3(1.0f, 0.0f, 0.0f); // Use x-axis if direction is aligned with y-axis
    }
    glm::vec3 right = glm::normalize(glm::cross(normDirection, up));
    up = glm::normalize(glm::cross(right, normDirection));
    
    // Spawn particles in a cone pattern
    for (int i = 0; i < particleCount; ++i) {
        // Create random spread within cone
        float angle1 = glm::linearRand(-spreadAngle, spreadAngle);
        float angle2 = glm::linearRand(-spreadAngle, spreadAngle);
        
        // Rotate direction vector based on random angles
        glm::vec3 particleDir = normDirection;
        particleDir = glm::normalize(particleDir + (right * glm::tan(angle1)));
        particleDir = glm::normalize(particleDir + (up * glm::tan(angle2)));
        
        // Random velocity variation (80%-120% of base velocity)
        float particleVelocity = velocity * glm::linearRand(PlasmaWeaponConstants::PARTICLE_VELOCITY_RANDOM_MIN, 
                                                         PlasmaWeaponConstants::PARTICLE_VELOCITY_RANDOM_MAX);
        
        // Create particle
        Particle plasma(
            origin,                                // Position
            particleDir * particleVelocity,       // Velocity
            m_plasmaColor,                        // Color
            m_particleLifetime * glm::linearRand(PlasmaWeaponConstants::PARTICLE_LIFETIME_RANDOM_MIN, 
                                                 PlasmaWeaponConstants::PARTICLE_LIFETIME_RANDOM_MAX) // Lifetime with variation
        );
        
        m_particleSystem->spawnParticle(plasma);
    }
    
    std::cout << "[PlasmaWeapon] Created " << particleCount << " plasma particles" << std::endl;
}

void PlasmaWeapon::updateDebugInfo() {
    std::stringstream ss;
    ss << "Mode: " << static_cast<int>(m_firingMode) 
       << " | Charge: " << static_cast<int>(m_currentCharge) << "/" << static_cast<int>(m_maxCharge) 
       << " (" << static_cast<int>(getChargePercentage() * 100.0f) << "%)"
       << " | Cooldown: " << (m_currentCooldown > 0.0f ? std::to_string(m_currentCooldown) + "s" : "Ready");
    
    m_debugStateMessage = ss.str();
}

} // namespace Combat
} // namespace TurtleEngine 