#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "ParticleSystem.hpp"
#include "HealthSystem.hpp"

namespace TurtleEngine {
namespace Combat {

namespace AIConstructConstants {
    // Default values
    constexpr float DEFAULT_HEALTH = 100.0f;
    constexpr float DEFAULT_ATTACK_DAMAGE = 10.0f;
    constexpr float DEFAULT_ATTACK_COOLDOWN = 2.0f;
    constexpr float DEFAULT_ATTACK_RANGE = 5.0f;
    constexpr float DEFAULT_DETECTION_RANGE = 10.0f;
    constexpr float DEFAULT_RETREAT_THRESHOLD = 0.3f;
    constexpr float DEFAULT_MOVEMENT_SPEED = 3.0f;
    
    // Sentry specific
    constexpr float SENTRY_ATTACK_RANGE = 15.0f;
    constexpr float SENTRY_DETECTION_RANGE = 20.0f;
    constexpr float SENTRY_MOVEMENT_SPEED = 1.0f;
    constexpr float SENTRY_ATTACK_DAMAGE = 15.0f;
    constexpr float SENTRY_ATTACK_COOLDOWN = 3.0f;
    constexpr float SENTRY_PLASMA_RESISTANCE = 0.5f;
    
    // Hunter specific
    constexpr float HUNTER_ATTACK_RANGE = 5.0f;
    constexpr float HUNTER_DETECTION_RANGE = 25.0f;
    constexpr float HUNTER_MOVEMENT_SPEED = 6.0f;
    constexpr float HUNTER_ATTACK_DAMAGE = 20.0f;
    constexpr float HUNTER_ATTACK_COOLDOWN = 1.0f;
    constexpr float HUNTER_PHYSICAL_RESISTANCE = 0.2f;
    
    // Guardian specific
    constexpr float GUARDIAN_ATTACK_RANGE = 8.0f;
    constexpr float GUARDIAN_DETECTION_RANGE = 15.0f;
    constexpr float GUARDIAN_MOVEMENT_SPEED = 2.0f;
    constexpr float GUARDIAN_ATTACK_DAMAGE = 10.0f;
    constexpr float GUARDIAN_ATTACK_COOLDOWN = 2.0f;
    constexpr float GUARDIAN_HEALTH_MULTIPLIER = 2.0f;
    constexpr float GUARDIAN_PHYSICAL_RESISTANCE = 0.4f;
    constexpr float GUARDIAN_SHIELD = 50.0f;
    constexpr float GUARDIAN_FLAT_REDUCTION = 5.0f;
    
    // Swarm specific
    constexpr float SWARM_ATTACK_RANGE = 3.0f;
    constexpr float SWARM_DETECTION_RANGE = 12.0f;
    constexpr float SWARM_MOVEMENT_SPEED = 8.0f;
    constexpr float SWARM_ATTACK_DAMAGE = 5.0f;
    constexpr float SWARM_ATTACK_COOLDOWN = 0.5f;
    constexpr float SWARM_HEALTH_MULTIPLIER = 0.5f;
    
    // State Timers
    constexpr float IDLE_STATE_BASE_TIME = 2.0f;
    constexpr float IDLE_STATE_RANDOM_TIME = 1.0f;
    constexpr float RETREAT_STATE_TIME = 5.0f;
    constexpr float DAMAGED_STATE_TIME = 0.5f;
    
    // Attack calculation
    constexpr float ATTACK_DAMAGE_RANDOM_MIN = 0.9f;
    constexpr float ATTACK_DAMAGE_RANDOM_MAX = 1.1f;
    
    // Particle related
    constexpr float PARTICLE_SPREAD_RADIUS = 0.2f;
    constexpr float PARTICLE_SPEED_RANDOM_MIN = 0.9f;
    constexpr float PARTICLE_SPEED_RANDOM_MAX = 1.1f;
    constexpr float PARTICLE_LIFETIME_RANDOM_MIN = 0.9f;
    constexpr float PARTICLE_LIFETIME_RANDOM_MAX = 1.1f;
    constexpr float PARTICLE_OFFSET_MULTIPLIER = 0.5f;
}

/**
 * @class AIConstruct
 * @brief Represents an AI-controlled combat construct with state-based behavior
 * 
 * AIConstruct implements a simple state machine for enemy behavior, including
 * idle, patrol, attack, and retreat states. It integrates with the particle system
 * for visual feedback and debug overlays.
 */
class AIConstruct {
public:
    // AI Construct states
    enum class State {
        IDLE,       // Not moving or attacking
        PATROL,     // Moving along patrol points
        ATTACK,     // Actively attacking player
        RETREAT,    // Moving away from player
        DAMAGED     // Taking damage (temporary state)
    };

    // AI Construct types (affects behavior and appearance)
    enum class Type {
        SENTRY,     // Stationary, long-range attacks
        HUNTER,     // Mobile, aggressive
        GUARDIAN,   // Defensive, high health
        SWARM       // Low health, spawns in groups
    };

    /**
     * @brief Construct a new AI Construct with default parameters
     * @param particleSystem Shared pointer to the particle system to use
     * @param type Type of AI construct
     * @param position Initial position
     * @param health Initial health
     */
    AIConstruct(std::shared_ptr<ParticleSystem> particleSystem,
                Type type = Type::SENTRY,
                const glm::vec3& position = glm::vec3(0.0f),
                float health = AIConstructConstants::DEFAULT_HEALTH);
    
    /**
     * @brief Construct a new AI Construct with Graphics::ParticleSystem
     * @param particleSystem Shared pointer to the particle system to use
     * @param type Type of AI construct
     * @param position Initial position
     * @param health Initial health
     */
    AIConstruct(std::shared_ptr<Graphics::ParticleSystem> particleSystem,
                Type type = Type::SENTRY,
                const glm::vec3& position = glm::vec3(0.0f),
                float health = AIConstructConstants::DEFAULT_HEALTH);
    
    /**
     * @brief Destructor
     */
    ~AIConstruct();

    /**
     * @brief Update the AI construct state and position
     * @param deltaTime Time since last update in seconds
     * @param playerPosition Current position of the player
     */
    void update(float deltaTime, const glm::vec3& playerPosition);

    /**
     * @brief Render the AI construct and its effects
     * @param view View matrix for rendering
     * @param projection Projection matrix for rendering
     */
    void render(const glm::mat4& view, const glm::mat4& projection);

    /**
     * @brief Apply damage to the construct
     * @param amount Amount of damage to apply
     * @param damageSource Source position of the damage (for directional feedback)
     * @return True if construct is still alive, false if destroyed
     */
    bool applyDamage(float amount, const glm::vec3& damageSource);
    
    /**
     * @brief Apply detailed damage to the construct
     * @param damage Detailed damage information
     * @return True if construct is still alive, false if destroyed
     */
    bool applyDamage(const DamageInfo& damage);

    /**
     * @brief Get the current position of the construct
     * @return Current position
     */
    glm::vec3 getPosition() const { return m_position; }

    /**
     * @brief Get the current state of the construct
     * @return Current state
     */
    State getState() const { return m_currentState; }

    /**
     * @brief Get the current health of the construct
     * @return Current health value
     */
    float getHealth() const;
    
    /**
     * @brief Get the maximum health of the construct
     * @return Maximum health value
     */
    float getMaxHealth() const;
    
    /**
     * @brief Get health as percentage
     * @return Health percentage from 0.0 to 1.0
     */
    float getHealthPercentage() const;

    /**
     * @brief Check if the construct is alive
     * @return True if health > 0
     */
    bool isAlive() const;
    
    /**
     * @brief Get access to the construct's health component
     * @return Reference to health component
     */
    HealthComponent& getHealthComponent() { return *m_healthComponent; }

    /**
     * @brief Add a patrol point for the construct to follow
     * @param point Position to add to patrol route
     */
    void addPatrolPoint(const glm::vec3& point);

    /**
     * @brief Set the attack range of the construct
     * @param range Distance at which construct will attack
     */
    void setAttackRange(float range) { m_attackRange = range; }

    /**
     * @brief Set the movement speed of the construct
     * @param speed Units per second movement speed
     */
    void setMovementSpeed(float speed) { m_movementSpeed = speed; }

    /**
     * @brief Enable or disable debug visualization
     * @param enabled Whether debug visualization should be enabled
     */
    void enableDebugVisualization(bool enabled);

    /**
     * @brief Force the construct to a specific state (for debugging/testing)
     * @param state State to force
     */
    void forceState(State state);

    /**
     * @brief Register a callback function for state changes
     * @param callback Function to call when state changes
     */
    void setStateChangeCallback(std::function<void(State, State)> callback) {
        m_stateChangeCallback = callback;
    }

private:
    // State machine methods
    void enterState(State newState);
    void updateIdleState(float deltaTime, const glm::vec3& playerPosition);
    void updatePatrolState(float deltaTime, const glm::vec3& playerPosition);
    void updateAttackState(float deltaTime, const glm::vec3& playerPosition);
    void updateRetreatState(float deltaTime, const glm::vec3& playerPosition);
    void updateDamagedState(float deltaTime, const glm::vec3& playerPosition);

    // Attack methods
    void performAttack(const glm::vec3& targetPosition);
    float calculateAttackDamage() const;

    // Movement methods
    void moveTowards(const glm::vec3& target, float deltaTime);
    void moveAwayFrom(const glm::vec3& target, float deltaTime);

    // Visual effect methods
    void createStateParticles();
    void createAttackParticles(const glm::vec3& targetPosition);
    void updateDebugInfo();
    
    // Health system callbacks
    void onDamageTaken(const DamageInfo& damage, float actualDamage);
    void onDeath();

    // Type-specific initialization
    void initializeTypeProperties();
    void setupResistances();

    // Basic properties
    Type m_type;
    State m_currentState;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_forward;
    float m_attackDamage;
    float m_attackCooldown;
    float m_currentAttackCooldown;
    float m_attackRange;
    float m_detectionRange;
    float m_retreatHealthThreshold;
    float m_movementSpeed;
    float m_stateTimer;

    // Patrol route
    std::vector<glm::vec3> m_patrolPoints;
    int m_currentPatrolPoint;

    // Debug properties
    bool m_debugVisualizationEnabled;
    std::string m_debugStateMessage;

    // Visuals
    std::shared_ptr<ParticleSystem> m_particleSystem;
    glm::vec4 m_constructColor;
    
    // Health system
    std::unique_ptr<HealthComponent> m_healthComponent;

    // Callback for state changes
    std::function<void(State, State)> m_stateChangeCallback;
};

} // namespace Combat
} // namespace TurtleEngine
