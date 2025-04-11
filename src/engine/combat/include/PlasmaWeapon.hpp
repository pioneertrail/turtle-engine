#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <string>
#include "ParticleSystem.hpp"

namespace TurtleEngine {

// Forward declarations
class Shader;

namespace Combat {

namespace PlasmaWeaponConstants {
    constexpr float DEFAULT_MAX_CHARGE = 100.0f;
    constexpr float DEFAULT_CHARGE_RATE = 50.0f;  // units/sec
    constexpr float DEFAULT_COOLDOWN_TIME = 0.5f; // seconds
    constexpr float MIN_FIRE_CHARGE = 10.0f;
    constexpr float QUICK_FIRE_POWER = 0.4f;
    constexpr float QUICK_FIRE_COOLDOWN_MULTIPLIER = 0.7f;
    constexpr float POWER_MIN_NORMALIZATION = 0.2f;
    constexpr float POWER_MAX_NORMALIZATION = 1.0f;
    constexpr float POWER_RANGE_NORMALIZATION = POWER_MAX_NORMALIZATION - POWER_MIN_NORMALIZATION;
    constexpr int   BASE_PARTICLE_COUNT = 30;
    constexpr int   POWER_PARTICLE_MULTIPLIER = 70;
    constexpr float BURST_SPREAD_DEGREES = 15.0f;
    constexpr float BEAM_SPREAD_DEGREES = 5.0f;
    constexpr float CHARGED_SPREAD_DEGREES = 10.0f;
    constexpr float SCATTER_SPREAD_DEGREES = 30.0f;
    constexpr float BEAM_PARTICLE_MULTIPLIER = 1.5f;
    constexpr float CHARGED_PARTICLE_MULTIPLIER = 2.0f;
    constexpr float SCATTER_PARTICLE_MULTIPLIER = 0.8f;
    constexpr float PARTICLE_VELOCITY_RANDOM_MIN = 0.8f;
    constexpr float PARTICLE_VELOCITY_RANDOM_MAX = 1.2f;
    constexpr float PARTICLE_LIFETIME_RANDOM_MIN = 0.8f;
    constexpr float PARTICLE_LIFETIME_RANDOM_MAX = 1.2f;
}

/**
 * @class PlasmaWeapon
 * @brief Represents a plasma-based weapon used primarily by the Flammyx tribe
 * 
 * PlasmaWeapon integrates with the particle system to create visually rich
 * plasma effects when fired. It supports different firing modes, charge levels,
 * and can be triggered by the Flammil gesture recognition system.
 */
class PlasmaWeapon {
public:
    // Firing modes for the plasma weapon
    enum class FiringMode {
        BURST,      // Quick burst of plasma particles
        BEAM,       // Continuous beam of plasma
        CHARGED,    // Charged shot (variable power based on charge time)
        SCATTER     // Wide-angle scatter shot
    };

    /**
     * @brief Construct a new Plasma Weapon with default parameters
     * @param particleSystem Shared pointer to the particle system to use
     * @param maxCharge Maximum charge level this weapon can achieve
     */
    PlasmaWeapon(std::shared_ptr<ParticleSystem> particleSystem, 
                 float maxCharge = PlasmaWeaponConstants::DEFAULT_MAX_CHARGE);
    
    /**
     * @brief Destructor
     */
    ~PlasmaWeapon();

    /**
     * @brief Update the weapon state (charging, cooling down, etc.)
     * @param deltaTime Time since last update in seconds
     */
    void update(float deltaTime);

    /**
     * @brief Render weapon effects (particles, glow, etc.)
     * @param view View matrix for rendering
     * @param projection Projection matrix for rendering
     */
    void render(const glm::mat4& view, const glm::mat4& projection);

    /**
     * @brief Begin charging the weapon
     * @return Current charge level (0 to maxCharge)
     */
    float beginCharging();

    /**
     * @brief Stop charging and fire the weapon
     * @param origin Origin point of the firing
     * @param direction Direction vector for the firing
     * @return True if firing was successful, false if cooling down or not charged
     */
    bool fire(const glm::vec3& origin, const glm::vec3& direction);

    /**
     * @brief Quick-fire the weapon without charging (for Flammil gesture)
     * @param origin Origin point of the firing
     * @param direction Direction vector for the firing
     * @return True if quick-fire was successful, false if cooling down
     */
    bool quickFire(const glm::vec3& origin, const glm::vec3& direction);

    /**
     * @brief Set the firing mode of the weapon
     * @param mode New firing mode to set
     */
    void setFiringMode(FiringMode mode);

    /**
     * @brief Get the current charge level of the weapon
     * @return Current charge as a percentage (0.0f to 1.0f)
     */
    float getChargePercentage() const;

    /**
     * @brief Get the current cooldown state of the weapon
     * @return True if the weapon is in cooldown and cannot fire
     */
    bool isCoolingDown() const;

    /**
     * @brief Enable or disable debug visualization
     * @param enabled Whether debug visualization should be enabled
     */
    void enableDebugVisualization(bool enabled);

private:
    // Internal implementation methods
    void createPlasmaParticles(const glm::vec3& origin, const glm::vec3& direction, float power);
    void updateDebugInfo();

    // Weapon properties
    FiringMode m_firingMode;
    float m_currentCharge;
    float m_maxCharge;
    float m_chargeRate;
    float m_cooldownTime;
    float m_currentCooldown;
    bool m_isCharging;

    // Visual properties
    glm::vec4 m_plasmaColor;
    float m_particleSize;
    float m_particleLifetime;
    float m_particleVelocity;

    // Particle system for visual effects
    std::shared_ptr<ParticleSystem> m_particleSystem;

    // Debug properties
    bool m_debugVisualizationEnabled;
    std::string m_debugStateMessage;
};

} // namespace Combat
} // namespace TurtleEngine 