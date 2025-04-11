#include "../include/AIConstruct.hpp"
#include <iostream>
#include <glm/gtc/random.hpp>
#include <glm/gtc/constants.hpp>
#include <sstream>
#include <algorithm>
#include <cmath>

namespace TurtleEngine {
namespace Combat {

AIConstruct::AIConstruct(Type type, const glm::vec3& position, float health)
    : m_type(type),
      m_currentState(State::IDLE),
      m_position(position),
      m_velocity(0.0f),
      m_forward(0.0f, 0.0f, 1.0f),
      m_health(health),
      m_maxHealth(health),
      m_attackDamage(10.0f),
      m_attackCooldown(2.0f),
      m_currentAttackCooldown(0.0f),
      m_attackRange(5.0f),
      m_detectionRange(10.0f),
      m_retreatHealthThreshold(0.3f), // Retreat at 30% health
      m_movementSpeed(3.0f),
      m_stateTimer(0.0f),
      m_currentPatrolPoint(0),
      m_debugVisualizationEnabled(false),
      m_constructColor(1.0f, 0.0f, 0.0f, 1.0f) { // Red color for enemy constructs

    // Initialize type-specific properties
    initializeTypeProperties();
    
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
            m_attackRange = 15.0f;
            m_detectionRange = 20.0f;
            m_movementSpeed = 1.0f;
            m_attackDamage = 15.0f;
            m_attackCooldown = 3.0f;
            m_constructColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Red
            break;
            
        case Type::HUNTER:
            m_attackRange = 5.0f;
            m_detectionRange = 25.0f;
            m_movementSpeed = 6.0f;
            m_attackDamage = 20.0f;
            m_attackCooldown = 1.0f;
            m_constructColor = glm::vec4(1.0f, 0.5f, 0.0f, 1.0f); // Orange
            break;
            
        case Type::GUARDIAN:
            m_attackRange = 8.0f;
            m_detectionRange = 15.0f;
            m_movementSpeed = 2.0f;
            m_attackDamage = 10.0f;
            m_attackCooldown = 2.0f;
            m_health = m_maxHealth = 200.0f;
            m_constructColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f); // Blue
            break;
            
        case Type::SWARM:
            m_attackRange = 3.0f;
            m_detectionRange = 12.0f;
            m_movementSpeed = 8.0f;
            m_attackDamage = 5.0f;
            m_attackCooldown = 0.5f;
            m_health = m_maxHealth = 50.0f;
            m_constructColor = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f); // Green
            break;
    }
}

bool AIConstruct::initialize(std::shared_ptr<ParticleSystem> particleSystem) {
    if (!particleSystem) {
        std::cerr << "[AIConstruct] Error: Null particle system provided!" << std::endl;
        return false;
    }

    m_particleSystem = particleSystem;
    std::cout << "[AIConstruct] Successfully initialized with particle system" << std::endl;
    
    // Create initial state particles
    createStateParticles();
    
    return true;
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
    // Apply damage to health
    m_health = std::max(0.0f, m_health - amount);
    
    std::cout << "[AIConstruct] Took " << amount << " damage. Health: " 
              << m_health << "/" << m_maxHealth << std::endl;
    
    // Create damage particle effect
    createDamageParticles(damageSource);
    
    // Check if should enter damaged state
    if (isAlive() && m_currentState != State::DAMAGED) {
        enterState(State::DAMAGED);
    }
    
    return isAlive();
}

void AIConstruct::addPatrolPoint(const glm::vec3& point) {
    m_patrolPoints.push_back(point);
    std::cout << "[AIConstruct] Added patrol point at (" 
              << point.x << ", " << point.y << ", " << point.z << ")" << std::endl;
}

void AIConstruct::enableDebugVisualization(bool enabled) {
    m_debugVisualizationEnabled = enabled;
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
            m_stateTimer = 2.0f + glm::linearRand(0.0f, 1.0f); // Random idle time
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
            m_stateTimer = 5.0f; // Retreat for 5 seconds
            break;
            
        case State::DAMAGED:
            m_stateTimer = 0.5f; // Damaged state lasts 0.5 seconds
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
        if (m_health < m_maxHealth * m_retreatHealthThreshold) {
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
        if (m_health < m_maxHealth * m_retreatHealthThreshold) {
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
    if (m_health < m_maxHealth * m_retreatHealthThreshold) {
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
        
        if (m_health < m_maxHealth * m_retreatHealthThreshold) {
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
    return m_attackDamage * glm::linearRand(0.9f, 1.1f);
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

void AIConstruct::createDamageParticles(const glm::vec3& damageSource) {
    if (!m_particleSystem) {
        return;
    }
    
    // Calculate direction from damage source
    glm::vec3 direction = glm::normalize(m_position - damageSource);
    
    // Create particles for damage
    glm::vec4 particleColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f); // Bright red
    int particleCount = 20;
    float particleSpeed = 5.0f;
    float particleLife = 0.5f;
    
    // Spawn particles at construct position, moving away from damage source
    for (int i = 0; i < particleCount; ++i) {
        glm::vec3 offset = glm::sphericalRand(0.5f);
        glm::vec3 particlePos = m_position + offset;
        
        // Create velocity vector (mostly in direction away from damage, with some spread)
        glm::vec3 particleDir = direction + glm::sphericalRand(0.5f);
        particleDir = glm::normalize(particleDir);
        
        float speed = particleSpeed * glm::linearRand(0.8f, 1.2f);
        
        // Create and spawn particle
        Particle p(
            particlePos,
            particleDir * speed,
            particleColor,
            particleLife * glm::linearRand(0.8f, 1.2f)
        );
        
        m_particleSystem->spawnParticle(p);
    }
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
            particleDir += glm::sphericalRand(0.2f);
            particleDir = glm::normalize(particleDir);
        }
        
        // Vary speed and lifetime slightly
        float speed = particleSpeed * glm::linearRand(0.9f, 1.1f);
        float lifetime = particleLife * glm::linearRand(0.9f, 1.1f);
        
        // Create starting position slightly offset from construct
        glm::vec3 startPos = m_position + (direction * 0.5f);
        
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
       << " | State: " << static_cast<int>(m_currentState)
       << " | Health: " << static_cast<int>(m_health) << "/" << static_cast<int>(m_maxHealth)
       << " | Pos: (" << std::fixed << std::setprecision(1) << m_position.x
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
