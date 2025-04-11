#pragma once

#include <functional>
#include <string>
#include <unordered_map>
#include <memory>
#include <vector>

#include <glm/glm.hpp>

namespace TurtleEngine {
// Bridge namespace for ParticleSystem
namespace Graphics { class ParticleSystem; }
using Graphics::ParticleSystem;
namespace Graphics {
    class ParticleSystem; // Forward declare the Graphics::ParticleSystem
}

namespace Combat {

/**
 * @enum DamageType
 * @brief Types of damage that can be applied to entities
 */
enum class DamageType {
    PHYSICAL,   // Physical damage (kinetic, impact)
    PLASMA,     // Plasma-based energy damage
    ENERGY,     // General energy damage
    KINETIC,    // Specific kinetic damage (subtype of PHYSICAL)
    SONIC,      // Sonic wave damage
    TEMPORAL,   // Time-based damage from anomalies
    PSYCHIC,    // Mental/consciousness damage
    COUNT       // Meta-entry for iterating over types
};

/**
 * @struct DamageInfo
 * @brief Detailed information about damage being applied
 */
struct DamageInfo {
    float amount;                // Base damage amount
    DamageType type;             // Type of damage
    glm::vec3 source;            // Source position of damage
    glm::vec3 direction;         // Direction of damage application
    float impactForce;           // Force of impact (for knockback)
    bool isCritical;             // Whether this is a critical hit
    std::string sourceIdentifier; // Identifier of damage source
    
    DamageInfo(
        float damageAmount = 0.0f,
        DamageType damageType = DamageType::PHYSICAL,
        const glm::vec3& sourcePos = glm::vec3(0.0f),
        const glm::vec3& dir = glm::vec3(0.0f, 0.0f, 1.0f),
        float force = 0.0f,
        bool critical = false,
        const std::string& identifier = ""
    ) : amount(damageAmount), type(damageType), source(sourcePos), 
        direction(dir), impactForce(force), isCritical(critical),
        sourceIdentifier(identifier) {}
};

/**
 * @class Resilience
 * @brief Manages damage reduction and resistance for entities
 */
class Resilience {
public:
    Resilience();
    
    /**
     * @brief Process incoming damage based on resistances and return actual damage to apply
     * @param damage The incoming damage info
     * @return Modified damage amount after applying resistances
     */
    float processDamage(const DamageInfo& damage);
    
    /**
     * @brief Set resistance to a specific damage type
     * @param type Type of damage
     * @param value Resistance value (0.0 to 1.0, where 1.0 is full immunity)
     */
    void setResistance(DamageType type, float value);
    
    /**
     * @brief Get current resistance to a damage type
     * @param type Type of damage
     * @return Resistance value (0.0 to 1.0)
     */
    float getResistance(DamageType type) const;
    
    /**
     * @brief Set flat damage reduction (applied before percentage resistance)
     * @param value Flat damage reduction amount
     */
    void setFlatReduction(float value);
    
    /**
     * @brief Get current flat damage reduction
     * @return Flat damage reduction amount
     */
    float getFlatReduction() const;
    
    /**
     * @brief Set shield value (temporary hit points)
     * @param value Shield amount
     */
    void setShield(float value);
    
    /**
     * @brief Get current shield value
     * @return Shield amount
     */
    float getShield() const;
    
    /**
     * @brief Process shield damage and return remaining damage
     * @param amount Incoming damage amount
     * @return Remaining damage after shield absorption
     */
    float processShieldDamage(float amount);
    
private:
    std::unordered_map<DamageType, float> m_resistances;
    float m_flatReduction;
    float m_shield;
};

/**
 * @class HealthComponent
 * @brief Manages health, resilience, and damage for an entity
 */
class HealthComponent {
public:
    /**
     * @brief Construct a new Health Component
     * @param maxHealth Maximum health value
     * @param particleSystem Particle system for visual effects (optional)
     */
    HealthComponent(float maxHealth = 100.0f,
                   std::shared_ptr<Graphics::ParticleSystem> particleSystem = nullptr);
                   
    /**
     * @brief Apply damage to the entity
     * @param damage Detailed damage info
     * @return Actual damage applied after resilience
     */
    float applyDamage(const DamageInfo& damage);
    
    /**
     * @brief Apply healing to the entity
     * @param amount Amount of healing to apply
     * @param source Source position for visual effects
     * @return Actual healing applied
     */
    float applyHealing(float amount, const glm::vec3& source = glm::vec3(0.0f));
    
    /**
     * @brief Get current health
     * @return Current health value
     */
    float getCurrentHealth() const;
    
    /**
     * @brief Get maximum health
     * @return Maximum health value
     */
    float getMaxHealth() const;
    
    /**
     * @brief Get health percentage
     * @return Health as percentage of maximum (0.0 to 1.0)
     */
    float getHealthPercentage() const;
    
    /**
     * @brief Check if entity is alive
     * @return True if health > 0
     */
    bool isAlive() const;
    
    /**
     * @brief Set the maximum health, adjusting current health proportionally
     * @param newMaxHealth New maximum health value
     */
    void setMaxHealth(float newMaxHealth);
    
    /**
     * @brief Get the resilience component
     * @return Reference to resilience component
     */
    Resilience& getResilience();
    
    /**
     * @brief Set callback for damage taken
     * @param callback Function to call when damage is taken
     */
    void setDamageCallback(std::function<void(const DamageInfo&, float)> callback);
    
    /**
     * @brief Set callback for healing received
     * @param callback Function to call when healing is received
     */
    void setHealingCallback(std::function<void(float, const glm::vec3&)> callback);
    
    /**
     * @brief Set callback for death
     * @param callback Function to call when health reaches 0
     */
    void setDeathCallback(std::function<void()> callback);
    
    /**
     * @brief Set the position of the entity (for visual effects)
     * @param position Current position
     */
    void setPosition(const glm::vec3& position);
    
    /**
     * @brief Get current position
     * @return Current position
     */
    const glm::vec3& getPosition() const;
    
    /**
     * @brief Enable or disable debug visualization
     * @param enabled Whether debug visualization should be enabled
     */
    void enableDebugVisualization(bool enabled);
    
    /**
     * @brief Check if debug visualization is enabled
     * @return True if debug visualization is enabled
     */
    bool isDebugVisualizationEnabled() const;
    
    /**
     * @brief Get the debug display message
     * @return Debug message with health info
     */
    std::string getDebugInfo() const;
    
private:
    void createDamageParticles(const DamageInfo& damage, float actualDamage);
    void createHealingParticles(float amount, const glm::vec3& source);
    void updateDebugInfo();
    
    float m_currentHealth;
    float m_maxHealth;
    std::unique_ptr<Resilience> m_resilience;
    glm::vec3 m_position;
    bool m_debugVisualizationEnabled;
    std::shared_ptr<Graphics::ParticleSystem> m_particleSystem;
    bool m_isVisible;
    std::string m_debugInfo;
    
    // Callbacks
    std::function<void(const DamageInfo&, float)> m_damageCallback;
    std::function<void(float, const glm::vec3&)> m_healingCallback;
    std::function<void()> m_deathCallback;
};

} // namespace Combat
} // namespace TurtleEngine

