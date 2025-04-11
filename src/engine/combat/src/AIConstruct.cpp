#include "../include/AIConstruct.hpp"
#include <iostream>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include "../../graphics/include/ParticleSystem.hpp"
#include "../../include/ParticleSystem.hpp"

namespace TurtleEngine {
namespace Combat {

AIConstruct::AIConstruct(std::shared_ptr<ParticleSystem> particleSystem,
                       Type type, 
                       const glm::vec3& position, 
                       float health)
    : m_type(type),
      m_currentState(State::IDLE),
      m_position(position),
      m_velocity(glm::vec3(0.0f)),
      m_forward(0.0f, 0.0f, 1.0f),
      m_attackDamage(AIConstructConstants::DEFAULT_ATTACK_DAMAGE),
      m_attackCooldown(AIConstructConstants::DEFAULT_ATTACK_COOLDOWN),
      m_currentAttackCooldown(0.0f),
      m_attackRange(AIConstructConstants::DEFAULT_ATTACK_RANGE),
      m_detectionRange(AIConstructConstants::DEFAULT_DETECTION_RANGE),
      m_retreatHealthThreshold(AIConstructConstants::DEFAULT_RETREAT_THRESHOLD),
      m_movementSpeed(AIConstructConstants::DEFAULT_MOVEMENT_SPEED),
      m_stateTimer(0.0f),
      m_currentPatrolPoint(0),
      m_debugVisualizationEnabled(false),
      m_constructColor(1.0f, 0.0f, 0.0f, 1.0f), // Red color for enemy constructs
      m_particleSystem(particleSystem) {

    // Initialize type-specific properties
    initializeTypeProperties();
    
    // Create health component (now receives particle system)
    m_healthComponent = std::make_unique<HealthComponent>(health, m_particleSystem);
    
    // Set health component position
    m_healthComponent->setPosition(m_position);
    
    // Setup damage and death callbacks
    m_healthComponent->setDamageCallback([this](const DamageInfo& damage, float actualDamage) {
        this->onDamageTaken(damage, actualDamage);
    });
    
    m_healthComponent->setDeathCallback([this]() {
        this->onDeath();
    });
    
    // Setup resistances based on type
    setupResistances();
    
    std::cout << "[AIConstruct] Initialized " << static_cast<int>(type) 
              << " type at position (" << position.x << ", " 
              << position.y << ", " << position.z << ")" << std::endl;
}

// Constructor overload for Graphics::ParticleSystem
AIConstruct::AIConstruct(std::shared_ptr<Graphics::ParticleSystem> particleSystem,
                        Type type, 
                        const glm::vec3& position, 
                        float health)
    : m_type(type),
      m_currentState(State::IDLE),
      m_position(position),
      m_velocity(glm::vec3(0.0f)),
      m_forward(0.0f, 0.0f, 1.0f),
      m_attackDamage(AIConstructConstants::DEFAULT_ATTACK_DAMAGE),
      m_attackCooldown(AIConstructConstants::DEFAULT_ATTACK_COOLDOWN),
      m_currentAttackCooldown(0.0f),
      m_attackRange(AIConstructConstants::DEFAULT_ATTACK_RANGE),
      m_detectionRange(AIConstructConstants::DEFAULT_DETECTION_RANGE),
      m_retreatHealthThreshold(AIConstructConstants::DEFAULT_RETREAT_THRESHOLD),
      m_movementSpeed(AIConstructConstants::DEFAULT_MOVEMENT_SPEED),
      m_stateTimer(0.0f),
      m_currentPatrolPoint(0),
      m_debugVisualizationEnabled(false),
      m_constructColor(1.0f, 0.0f, 0.0f, 1.0f), // Red color for enemy constructs
      m_particleSystem(std::shared_ptr<ParticleSystem>()) { // Initialize with empty pointer for now

    // Initialize type-specific properties
    initializeTypeProperties();
    
    // Create health component (now receives particle system)
    m_healthComponent = std::make_unique<HealthComponent>(health, m_particleSystem);
    
    // Set health component position
    m_healthComponent->setPosition(m_position);
    
    // Setup damage and death callbacks
    m_healthComponent->setDamageCallback([this](const DamageInfo& damage, float actualDamage) {
        this->onDamageTaken(damage, actualDamage);
    });
    
    m_healthComponent->setDeathCallback([this]() {
        this->onDeath();
    });
    
    // Setup resistances based on type
    setupResistances();
    
    std::cout << "[AIConstruct] Initialized " << static_cast<int>(type) 
              << " type at position (" << position.x << ", " 
              << position.y << ", " << position.z << ")" << std::endl;
}

AIConstruct::~AIConstruct() {
    std::cout << "[AIConstruct] Destroyed" << std::endl;
}

void AIConstruct::initializeTypeProperties() {
    switch (m_type) {
        case Type::SENTRY:
            m_attackRange = AIConstructConstants::SENTRY_ATTACK_RANGE;
            m_detectionRange = AIConstructConstants::SENTRY_DETECTION_RANGE;
            m_movementSpeed = AIConstructConstants::SENTRY_MOVEMENT_SPEED;
            m_attackDamage = AIConstructConstants::SENTRY_ATTACK_DAMAGE;
            m_attackCooldown = AIConstructConstants::SENTRY_ATTACK_COOLDOWN;
            m_constructColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
            break;
            
        case Type::HUNTER:
            m_attackRange = AIConstructConstants::HUNTER_ATTACK_RANGE;
            m_detectionRange = AIConstructConstants::HUNTER_DETECTION_RANGE;
            m_movementSpeed = AIConstructConstants::HUNTER_MOVEMENT_SPEED;
            m_attackDamage = AIConstructConstants::HUNTER_ATTACK_DAMAGE;
            m_attackCooldown = AIConstructConstants::HUNTER_ATTACK_COOLDOWN;
            m_constructColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
            break;
            
        case Type::GUARDIAN:
            m_attackRange = AIConstructConstants::GUARDIAN_ATTACK_RANGE;
            m_detectionRange = AIConstructConstants::GUARDIAN_DETECTION_RANGE;
            m_movementSpeed = AIConstructConstants::GUARDIAN_MOVEMENT_SPEED;
            m_attackDamage = AIConstructConstants::GUARDIAN_ATTACK_DAMAGE;
            m_attackCooldown = AIConstructConstants::GUARDIAN_ATTACK_COOLDOWN;
            // Guardian has higher health, but it's now set in the setupResistances method
            m_constructColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
            break;
            
        case Type::SWARM:
            m_attackRange = AIConstructConstants::SWARM_ATTACK_RANGE;
            m_detectionRange = AIConstructConstants::SWARM_DETECTION_RANGE;
            m_movementSpeed = AIConstructConstants::SWARM_MOVEMENT_SPEED;
            m_attackDamage = AIConstructConstants::SWARM_ATTACK_DAMAGE;
            m_attackCooldown = AIConstructConstants::SWARM_ATTACK_COOLDOWN;
            // Swarm has lower health, but it's now set in the setupResistances method
            m_constructColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
            break;
    }
}

void AIConstruct::setupResistances() {
    if (!m_healthComponent) {
        return;
    }
    
    Resilience& resilience = m_healthComponent->getResilience();
    
    // Set type-specific resistances and health
    switch (m_type) {
        case Type::SENTRY:
            // Standard health, strong against plasma (energy turret)
            resilience.setResistance(DamageType::PLASMA, AIConstructConstants::SENTRY_PLASMA_RESISTANCE);
            break;
            
        case Type::HUNTER:
            // Standard health, fast and agile, but no special resistances
            resilience.setResistance(DamageType::PHYSICAL, AIConstructConstants::HUNTER_PHYSICAL_RESISTANCE);
            break;
            
        case Type::GUARDIAN:
            // High health, strong armor, shield
            if (m_healthComponent) {
                // Double the health for Guardians
                m_healthComponent->setMaxHealth(m_healthComponent->getMaxHealth() * AIConstructConstants::GUARDIAN_HEALTH_MULTIPLIER);
            }
            
            resilience.setResistance(DamageType::PHYSICAL, AIConstructConstants::GUARDIAN_PHYSICAL_RESISTANCE);
            resilience.setShield(AIConstructConstants::GUARDIAN_SHIELD);
            resilience.setFlatReduction(AIConstructConstants::GUARDIAN_FLAT_REDUCTION);
            break;
            
        case Type::SWARM:
            // Low health, fast but fragile
            if (m_healthComponent) {
                // Halve the health for Swarm units
                m_healthComponent->setMaxHealth(m_healthComponent->getMaxHealth() * AIConstructConstants::SWARM_HEALTH_MULTIPLIER);
            }
            
            // No resistances for swarm units - they're meant to be fragile
            break;
    }
    
    // Callbacks are already set in the constructor
    // Position is already set in the constructor
}

void AIConstruct::update(float deltaTime, const glm::vec3& playerPosition) {
    // Skip update if not alive
    if (!isAlive()) {
        return;
    }
    
    // Update attack cooldown
    if (m_currentAttackCooldown > 0.0f) {
        m_currentAttackCooldown = std::max(0.0f, m_currentAttackCooldown - deltaTime);
    }
    
    // Update state timer
    m_stateTimer -= deltaTime;
    
    // Update based on current state
    switch (m_currentState) {
        case State::IDLE:
            updateIdleState(deltaTime, playerPosition);
            break;
            
        case State::PATROL:
            updatePatrolState(deltaTime, playerPosition);
            break;
            
        case State::ATTACK:
            updateAttackState(deltaTime, playerPosition);
            break;
            
        case State::RETREAT:
            updateRetreatState(deltaTime, playerPosition);
            break;
            
        case State::DAMAGED:
            updateDamagedState(deltaTime, playerPosition);
            break;
    }
    
    // Update health component position
    if (m_healthComponent) {
        m_healthComponent->setPosition(m_position);
    }
    
    // Update debug information
    if (m_debugVisualizationEnabled) {
        updateDebugInfo();
    }
}

void AIConstruct::render(const glm::mat4& view, const glm::mat4& projection) {
    // In a real implementation, we would render the AI construct model here
    // For this prototype, we'll just use the particle system for visual representation
    
    if (m_debugVisualizationEnabled) {
        // In a real implementation, we would render debug text or indicators here
        std::cout << "[AIConstruct] Debug: " << m_debugStateMessage << std::endl;
    }
}

bool AIConstruct::applyDamage(float amount, const glm::vec3& damageSource) {
    if (!isAlive() || amount <= 0.0f) {
        return isAlive();
    }
    
    // Create a simple damage info with physical damage
    DamageInfo damage(
        amount,
        DamageType::PHYSICAL,
        damageSource,
        glm::normalize(m_position - damageSource), // Direction from source to construct
        amount * 0.1f // Some impact force based on damage
    );
    
    return applyDamage(damage);
}

bool AIConstruct::applyDamage(const DamageInfo& damage) {
    if (!isAlive() || !m_healthComponent) {
        return isAlive();
    }
    
    // Apply damage through health component
    m_healthComponent->applyDamage(damage);
    
    return isAlive();
}

float AIConstruct::getHealth() const {
    return m_healthComponent ? m_healthComponent->getCurrentHealth() : 0.0f;
}

float AIConstruct::getMaxHealth() const {
    return m_healthComponent ? m_healthComponent->getMaxHealth() : 0.0f;
}

float AIConstruct::getHealthPercentage() const {
    return m_healthComponent ? m_healthComponent->getHealthPercentage() : 0.0f;
}

bool AIConstruct::isAlive() const {
    return m_healthComponent && m_healthComponent->isAlive();
}

void AIConstruct::onDamageTaken(const DamageInfo& damage, float actualDamage) {
    // Enter damaged state when hit (if not already in it)
    if (m_currentState != State::DAMAGED && isAlive()) {
        enterState(State::DAMAGED);
    }
    
    std::cout << "[AIConstruct] Took " << actualDamage 
              << " damage (" << damage.amount << " raw). Health: " 
              << getHealth() << "/" << getMaxHealth() << std::endl;
}

void AIConstruct::onDeath() {
    std::cout << "[AIConstruct] Died" << std::endl;
    
    // Create death particles
    if (m_particleSystem) {
        // Death particles are more dramatic
        glm::vec4 deathColor = glm::vec4(0.8f, 0.2f, 0.2f, 1.0f); // Red
        int particleCount = 50;
        float particleSpeed = 8.0f;
        float particleLife = 2.0f;
        
        m_particleSystem->spawnBurst(
            particleCount,
            m_position,
            particleSpeed,
            particleLife,
            deathColor
        );
    }
    
    // No need to change state since we're dead
}

void AIConstruct::addPatrolPoint(const glm::vec3& point) {
    m_patrolPoints.push_back(point);
    std::cout << "[AIConstruct] Added patrol point at (" 
              << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
}

void AIConstruct::enableDebugVisualization(bool enabled) {
    m_debugVisualizationEnabled = enabled;
    
    // Also enable debug visualization for health component
    if (m_healthComponent) {
        m_healthComponent->enableDebugVisualization(enabled);
    }
    
    std::cout << "[AIConstruct] Debug visualization " 
              << (enabled ? "enabled" : "disabled") << std::endl;
}

void AIConstruct::forceState(State state) {
    std::cout << "[AIConstruct] Forcing state to " << static_cast<int>(state) << std::endl;
    enterState(state);
}

// Private methods

void AIConstruct::enterState(State newState) {
    State oldState = m_currentState;
    m_currentState = newState;
    
    std::cout << "[AIConstruct] State changed from " 
              << static_cast<int>(oldState) << " to " 
              << static_cast<int>(newState) << std::endl;
    
    // Reset state timer based on new state
    switch (newState) {
        case State::IDLE:
            m_stateTimer = AIConstructConstants::IDLE_STATE_BASE_TIME + 
                          glm::linearRand(0.0f, AIConstructConstants::IDLE_STATE_RANDOM_TIME);
            break;
            
        case State::PATROL:
            // No timer for patrol, it continues until interrupted
            m_stateTimer = 0.0f;
            break;
            
        case State::ATTACK:
            // No timer for attack, it continues until interrupted
            m_stateTimer = 0.0f;
            break;
            
        case State::RETREAT:
            m_stateTimer = AIConstructConstants::RETREAT_STATE_TIME;
            break;
            
        case State::DAMAGED:
            m_stateTimer = AIConstructConstants::DAMAGED_STATE_TIME;
            break;
    }
    
    // Create particles for state transition
    createStateParticles();
    
    // Call state change callback if registered
    if (m_stateChangeCallback) {
        m_stateChangeCallback(oldState, newState);
    }
}

void AIConstruct::updateIdleState(float deltaTime, const glm::vec3& playerPosition) {
    // Check if player is within detection range
    float distanceToPlayer = glm::distance(m_position, playerPosition);
    
    if (distanceToPlayer <= m_detectionRange) {
        // Player detected, attack if health is good, retreat if low
        if (getHealthPercentage() < m_retreatHealthThreshold) {
            enterState(State::RETREAT);
        } else {
            enterState(State::ATTACK);
        }
        return;
    }
    
    // If we have patrol points and idle timer expired, start patrolling
    if (!m_patrolPoints.empty() && m_stateTimer <= 0.0f) {
        enterState(State::PATROL);
        return;
    }
    
    // No movement in idle
    m_velocity = glm::vec3(0.0f);
}

void AIConstruct::updatePatrolState(float deltaTime, const glm::vec3& playerPosition) {
    // Check if player is within detection range
    float distanceToPlayer = glm::distance(m_position, playerPosition);
    
    if (distanceToPlayer <= m_detectionRange) {
        // Player detected, attack if health is good, retreat if low
        if (getHealthPercentage() < m_retreatHealthThreshold) {
            enterState(State::RETREAT);
        } else {
            enterState(State::ATTACK);
        }
        return;
    }
    
    // If no patrol points, go back to idle
    if (m_patrolPoints.empty()) {
        enterState(State::IDLE);
        return;
    }
    
    // Move toward current patrol point
    glm::vec3 targetPoint = m_patrolPoints[m_currentPatrolPoint];
    float distanceToTarget = glm::distance(m_position, targetPoint);
    
    if (distanceToTarget < 0.5f) {
        // Reached patrol point, move to next one
        m_currentPatrolPoint = (m_currentPatrolPoint + 1) % m_patrolPoints.size();
        std::cout << "[AIConstruct] Reached patrol point, moving to next one: " 
                  << m_currentPatrolPoint << std::endl;
    } else {
        // Move toward patrol point
        moveTowards(targetPoint, deltaTime);
    }
}

void AIConstruct::updateAttackState(float deltaTime, const glm::vec3& playerPosition) {
    float distanceToPlayer = glm::distance(m_position, playerPosition);
    
    // Check if player is out of detection range
    if (distanceToPlayer > m_detectionRange) {
        // Lost track of player, return to patrol or idle
        if (!m_patrolPoints.empty()) {
            enterState(State::PATROL);
        } else {
            enterState(State::IDLE);
        }
        return;
    }
    
    // Check if health is low
    if (getHealthPercentage() < m_retreatHealthThreshold) {
        enterState(State::RETREAT);
        return;
    }
    
    // If in attack range, attack; otherwise move closer
    if (distanceToPlayer <= m_attackRange) {
        // In attack range, attack if cooldown is ready
        if (m_currentAttackCooldown <= 0.0f) {
            performAttack(playerPosition);
        }
        
        // Face the player
        glm::vec3 dirToPlayer = glm::normalize(playerPosition - m_position);
        m_forward = dirToPlayer;
        
        // Stop moving when attacking
        m_velocity = glm::vec3(0.0f);
    } else {
        // Outside attack range, move closer
        moveTowards(playerPosition, deltaTime);
    }
}

void AIConstruct::updateRetreatState(float deltaTime, const glm::vec3& playerPosition) {
    // Check if retreat timer expired
    if (m_stateTimer <= 0.0f) {
        // End retreat, go back to idle or patrol
        if (!m_patrolPoints.empty()) {
            enterState(State::PATROL);
        } else {
            enterState(State::IDLE);
        }
        return;
    }
    
    // Move away from player
    moveAwayFrom(playerPosition, deltaTime);
}

void AIConstruct::updateDamagedState(float deltaTime, const glm::vec3& playerPosition) {
    // Check if damaged timer expired
    if (m_stateTimer <= 0.0f) {
        // End damaged state
        
        // Determine next state based on health and player distance
        float distanceToPlayer = glm::distance(m_position, playerPosition);
        
        if (getHealthPercentage() < m_retreatHealthThreshold) {
            enterState(State::RETREAT);
        } else if (distanceToPlayer <= m_detectionRange) {
            enterState(State::ATTACK);
        } else if (!m_patrolPoints.empty()) {
            enterState(State::PATROL);
        } else {
            enterState(State::IDLE);
        }
        return;
    }
    
    // No movement during damaged state
    m_velocity = glm::vec3(0.0f);
}

void AIConstruct::performAttack(const glm::vec3& targetPosition) {
    std::cout << "[AIConstruct] Attacking target at (" 
              << targetPosition.x << ", " << targetPosition.y << ", " 
              << targetPosition.z << ")" << std::endl;
    
    // Calculate damage
    float damage = calculateAttackDamage();
    
    // Reset attack cooldown
    m_currentAttackCooldown = m_attackCooldown;
    
    // Create attack particles
    createAttackParticles(targetPosition);
    
    // Note: In a real implementation, we would apply damage to the player
    // For this prototype, we'll just create visual feedback
}

float AIConstruct::calculateAttackDamage() const {
    // Base damage with small random variation
    return m_attackDamage * glm::linearRand(AIConstructConstants::ATTACK_DAMAGE_RANDOM_MIN, 
                                            AIConstructConstants::ATTACK_DAMAGE_RANDOM_MAX);
}

void AIConstruct::moveTowards(const glm::vec3& target, float deltaTime) {
    glm::vec3 direction = target - m_position;
    float distance = glm::length(direction);
    
    if (distance > 0.001f) {
        direction = glm::normalize(direction);
        m_forward = direction; // Update forward direction
        
        // Calculate movement this frame
        glm::vec3 movement = direction * m_movementSpeed * deltaTime;
        
        // Cap movement to not overshoot target
        if (glm::length(movement) > distance) {
            movement = direction * distance;
        }
        
        // Update position and velocity
        m_position += movement;
        m_velocity = direction * m_movementSpeed;
    } else {
        m_velocity = glm::vec3(0.0f);
    }
}

void AIConstruct::moveAwayFrom(const glm::vec3& target, float deltaTime) {
    glm::vec3 direction = m_position - target;
    float distance = glm::length(direction);
    
    if (distance > 0.001f) {
        direction = glm::normalize(direction);
        m_forward = -direction; // Face away from danger
        
        // Calculate movement this frame
        glm::vec3 movement = direction * m_movementSpeed * deltaTime;
        
        // Update position and velocity
        m_position += movement;
        m_velocity = direction * m_movementSpeed;
    } else {
        // If at same position as target (shouldn't happen), move in random direction
        glm::vec3 randomDir = glm::sphericalRand(1.0f);
        randomDir.y = 0.0f; // Keep on ground plane
        randomDir = glm::normalize(randomDir);
        
        m_position += randomDir * m_movementSpeed * deltaTime;
        m_velocity = randomDir * m_movementSpeed;
        m_forward = -randomDir;
    }
}

void AIConstruct::createStateParticles() {
    if (!m_particleSystem) {
        return;
    }
    
    // Create particles based on state
    glm::vec4 particleColor;
    int particleCount = 10;
    float particleSpeed = 2.0f;
    float particleLife = 1.0f;
    
    switch (m_currentState) {
        case State::IDLE:
            particleColor = glm::vec4(0.7f, 0.7f, 0.7f, 0.5f); // Gray
            particleCount = 5;
            particleSpeed = 1.0f;
            break;
            
        case State::PATROL:
            particleColor = glm::vec4(0.0f, 0.7f, 0.7f, 0.5f); // Cyan
            particleCount = 8;
            particleSpeed = 1.5f;
            break;
            
        case State::ATTACK:
            particleColor = glm::vec4(1.0f, 0.1f, 0.1f, 0.8f); // Bright red
            particleCount = 15;
            particleSpeed = 3.0f;
            break;
            
        case State::RETREAT:
            particleColor = glm::vec4(1.0f, 1.0f, 0.0f, 0.5f); // Yellow
            particleCount = 20;
            particleSpeed = 4.0f;
            break;
            
        case State::DAMAGED:
            particleColor = glm::vec4(1.0f, 0.5f, 0.0f, 0.8f); // Orange
            particleCount = 25;
            particleSpeed = 5.0f;
            break;
    }
    
    // Spawn particles around construct
    m_particleSystem->spawnBurst(
        particleCount,
        m_position,
        particleSpeed,
        particleLife,
        particleColor
    );
}

void AIConstruct::createAttackParticles(const glm::vec3& targetPosition) {
    if (!m_particleSystem) {
        return;
    }
    
    // Create particles based on construct type
    glm::vec4 particleColor;
    int particleCount;
    float particleSpeed;
    float particleLife;
    
    switch (m_type) {
        case Type::SENTRY:
            // Laser-like beam attack
            particleColor = glm::vec4(1.0f, 0.0f, 0.0f, 0.8f); // Red
            particleCount = 50;
            particleSpeed = 20.0f;
            particleLife = 0.5f;
            break;
            
        case Type::HUNTER:
            // Fast projectile attack
            particleColor = glm::vec4(1.0f, 0.5f, 0.0f, 0.8f); // Orange
            particleCount = 30;
            particleSpeed = 15.0f;
            particleLife = 1.0f;
            break;
            
        case Type::GUARDIAN:
            // Area effect attack
            particleColor = glm::vec4(0.0f, 0.0f, 1.0f, 0.8f); // Blue
            particleCount = 100;
            particleSpeed = 10.0f;
            particleLife = 1.5f;
            break;
            
        case Type::SWARM:
            // Small, quick projectile
            particleColor = glm::vec4(0.0f, 1.0f, 0.0f, 0.8f); // Green
            particleCount = 15;
            particleSpeed = 12.0f;
            particleLife = 0.7f;
            break;
    }
    
    // Calculate direction to target
    glm::vec3 direction = glm::normalize(targetPosition - m_position);
    
    // Create attack particles
    for (int i = 0; i < particleCount; ++i) {
        // Create slight spread in direction
        glm::vec3 particleDir = direction;
        if (m_type != Type::SENTRY) { // Sentries have straight beams
            particleDir += glm::sphericalRand(AIConstructConstants::PARTICLE_SPREAD_RADIUS);
            particleDir = glm::normalize(particleDir);
        }
        
        // Vary speed and lifetime slightly
        float speed = particleSpeed * glm::linearRand(AIConstructConstants::PARTICLE_SPEED_RANDOM_MIN, 
                                                    AIConstructConstants::PARTICLE_SPEED_RANDOM_MAX);
        float lifetime = particleLife * glm::linearRand(AIConstructConstants::PARTICLE_LIFETIME_RANDOM_MIN, 
                                                    AIConstructConstants::PARTICLE_LIFETIME_RANDOM_MAX);
        
        // Create starting position slightly offset from construct
        glm::vec3 startPos = m_position + (direction * AIConstructConstants::PARTICLE_OFFSET_MULTIPLIER);
        
        // Create and spawn particle
        Particle p(
            startPos,
            particleDir * speed,
            particleColor,
            lifetime
        );
        
        m_particleSystem->spawnParticle(p);
    }
}

void AIConstruct::updateDebugInfo() {
    std::stringstream ss;
    ss << "Type: " << static_cast<int>(m_type)
       << " | State: " << static_cast<int>(m_currentState);
       
    // Add health info if available
    if (m_healthComponent) {
        std::string healthInfo = m_healthComponent->getDebugInfo();
        if (!healthInfo.empty()) {
            ss << " | " << healthInfo;
        }
    }
    
    ss << " | Pos: (" << std::fixed << std::setprecision(1) << m_position.x
       << ", " << m_position.y << ", " << m_position.z << ")";
    
    if (m_currentAttackCooldown > 0.0f) {
        ss << " | Cooldown: " << std::fixed << std::setprecision(1) << m_currentAttackCooldown;
    } else {
        ss << " | Ready";
    }
    
    m_debugStateMessage = ss.str();
}

} // namespace Combat
} // namespace TurtleEngine
