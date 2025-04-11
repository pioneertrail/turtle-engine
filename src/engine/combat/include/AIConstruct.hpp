#pragma once

#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include "ParticleSystem.hpp"

namespace TurtleEngine {
namespace Combat {

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
     * @param type Type of AI construct
     * @param position Initial position
     * @param health Initial health
     */
    AIConstruct(Type type = Type::SENTRY, 
                const glm::vec3& position = glm::vec3(0.0f),
                float health = 100.0f);
    
    /**
     * @brief Destructor
     */
    ~AIConstruct();

    /**
     * @brief Initialize the AI construct with a particle system for visual effects
     * @param particleSystem Shared pointer to the particle system to use
     * @return True if initialization was successful
     */
    bool initialize(std::shared_ptr<ParticleSystem> particleSystem);

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
    float getHealth() const { return m_health; }

    /**
     * @brief Check if the construct is alive
     * @return True if health > 0
     */
    bool isAlive() const { return m_health > 0.0f; }

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
    void createDamageParticles(const glm::vec3& damageSource);
    void createAttackParticles(const glm::vec3& targetPosition);
    void updateDebugInfo();

    // Type-specific initialization
    void initializeTypeProperties();

    // Basic properties
    Type m_type;
    State m_currentState;
    glm::vec3 m_position;
    glm::vec3 m_velocity;
    glm::vec3 m_forward;
    float m_health;
    float m_maxHealth;
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

    // Callback for state changes
    std::function<void(State, State)> m_stateChangeCallback;
};

} // namespace Combat
} // namespace TurtleEngine
