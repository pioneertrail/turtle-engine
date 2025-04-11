#include "../../engine/combat/include/AIConstruct.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <iostream>
#include <glm/glm.hpp>
#include <queue>

using namespace TurtleEngine;
using namespace TurtleEngine::Combat;

// Create a mock ParticleSystem for testing
class MockParticleSystem : public ParticleSystem {
public:
    MockParticleSystem() : ParticleSystem(1000), m_particleCount(0) {}
    
    void spawnParticle(const Particle& particle) override {
        m_lastParticle = particle;
        m_particleCount++;
    }
    
    void spawnBurst(int count, const glm::vec3& position, float initialSpeed, 
                   float lifetime, const glm::vec4& color) override {
        m_burstCount++;
        m_lastBurstPosition = position;
        m_lastBurstColor = color;
        m_particleCount += count;
    }
    
    int getParticleCount() const { return m_particleCount; }
    int getBurstCount() const { return m_burstCount; }
    Particle getLastParticle() const { return m_lastParticle; }
    glm::vec3 getLastBurstPosition() const { return m_lastBurstPosition; }
    glm::vec4 getLastBurstColor() const { return m_lastBurstColor; }
    
    void resetCounters() { 
        m_particleCount = 0; 
        m_burstCount = 0;
    }
    
private:
    int m_particleCount = 0;
    int m_burstCount = 0;
    Particle m_lastParticle;
    glm::vec3 m_lastBurstPosition;
    glm::vec4 m_lastBurstColor;
};

// Create a state change tracker for tests
class StateChangeTracker {
public:
    void recordStateChange(AIConstruct::State oldState, AIConstruct::State newState) {
        m_stateChanges.push({ oldState, newState });
    }
    
    bool hasStateChange() const {
        return !m_stateChanges.empty();
    }
    
    std::pair<AIConstruct::State, AIConstruct::State> getNextStateChange() {
        if (m_stateChanges.empty()) {
            return { AIConstruct::State::IDLE, AIConstruct::State::IDLE }; // Default if empty
        }
        
        auto change = m_stateChanges.front();
        m_stateChanges.pop();
        return change;
    }
    
    void clear() {
        while (!m_stateChanges.empty()) {
            m_stateChanges.pop();
        }
    }
    
private:
    std::queue<std::pair<AIConstruct::State, AIConstruct::State>> m_stateChanges;
};

class AIConstructTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock particle system
        particleSystem = std::make_shared<MockParticleSystem>();
        
        // Create AI construct
        construct = std::make_unique<AIConstruct>(AIConstruct::Type::HUNTER, glm::vec3(0.0f, 0.0f, 0.0f), 100.0f);
        construct->initialize(particleSystem);
        
        // Set state change callback
        construct->setStateChangeCallback([this](AIConstruct::State oldState, AIConstruct::State newState) {
            stateTracker.recordStateChange(oldState, newState);
        });
        
        // Enable debug visualization for testing
        construct->enableDebugVisualization(true);
        
        // Set up patrol points
        construct->addPatrolPoint(glm::vec3(5.0f, 0.0f, 0.0f));
        construct->addPatrolPoint(glm::vec3(0.0f, 0.0f, 5.0f));
        construct->addPatrolPoint(glm::vec3(-5.0f, 0.0f, 0.0f));
        
        // Reset particle counters
        particleSystem->resetCounters();
        
        // Clear state tracker
        stateTracker.clear();
        
        // Default player position (far away)
        playerPosition = glm::vec3(100.0f, 0.0f, 100.0f);
    }
    
    std::shared_ptr<MockParticleSystem> particleSystem;
    std::unique_ptr<AIConstruct> construct;
    StateChangeTracker stateTracker;
    glm::vec3 playerPosition;
};

TEST_F(AIConstructTest, Initialize) {
    AIConstruct testConstruct;
    EXPECT_TRUE(testConstruct.initialize(particleSystem));
    EXPECT_FALSE(testConstruct.initialize(nullptr)); // Should fail with null particle system
}

TEST_F(AIConstructTest, StateIdleToPatrol) {
    // Force idle state to start
    construct->forceState(AIConstruct::State::IDLE);
    stateTracker.clear();
    
    // Update for enough time for idle timer to expire
    for (int i = 0; i < 30; i++) {
        construct->update(0.1f, playerPosition); // 3 seconds total
    }
    
    // Should transition to patrol since player is far away and patrol points exist
    EXPECT_TRUE(stateTracker.hasStateChange());
    auto stateChange = stateTracker.getNextStateChange();
    EXPECT_EQ(AIConstruct::State::IDLE, stateChange.first);
    EXPECT_EQ(AIConstruct::State::PATROL, stateChange.second);
    
    // Check that particles were created for state change
    EXPECT_GT(particleSystem->getBurstCount(), 0);
}

TEST_F(AIConstructTest, DetectAndAttackPlayer) {
    // Start in idle state
    construct->forceState(AIConstruct::State::IDLE);
    stateTracker.clear();
    particleSystem->resetCounters();
    
    // Move player within detection range
    playerPosition = glm::vec3(8.0f, 0.0f, 0.0f); // Within detection range for HUNTER
    construct->update(0.1f, playerPosition);
    
    // Should transition to attack
    EXPECT_TRUE(stateTracker.hasStateChange());
    auto stateChange = stateTracker.getNextStateChange();
    EXPECT_EQ(AIConstruct::State::IDLE, stateChange.first);
    EXPECT_EQ(AIConstruct::State::ATTACK, stateChange.second);
    
    // Check that particles were created for state change
    EXPECT_GT(particleSystem->getBurstCount(), 0);
    
    // Update multiple times to move closer to player
    particleSystem->resetCounters();
    for (int i = 0; i < 10; i++) {
        construct->update(0.1f, playerPosition);
    }
    
    // Construct should be moving toward player
    EXPECT_LT(glm::distance(construct->getPosition(), playerPosition), 8.0f);
    
    // Move player within attack range
    playerPosition = construct->getPosition() + glm::vec3(1.0f, 0.0f, 0.0f); // Very close
    
    // Update to allow attack
    particleSystem->resetCounters();
    construct->update(0.1f, playerPosition);
    
    // Should attack (create attack particles)
    EXPECT_GT(particleSystem->getParticleCount(), 0);
}

TEST_F(AIConstructTest, RetreatWhenDamaged) {
    // Start in idle state with full health
    construct->forceState(AIConstruct::State::IDLE);
    stateTracker.clear();
    particleSystem->resetCounters();
    
    // Apply significant damage
    construct->applyDamage(80.0f, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Should transition to damaged state
    EXPECT_TRUE(stateTracker.hasStateChange());
    auto stateChange = stateTracker.getNextStateChange();
    EXPECT_EQ(AIConstruct::State::IDLE, stateChange.first);
    EXPECT_EQ(AIConstruct::State::DAMAGED, stateChange.second);
    
    // Update to exit damaged state
    construct->update(1.0f, playerPosition);
    
    // Should transition to retreat due to low health
    EXPECT_TRUE(stateTracker.hasStateChange());
    stateChange = stateTracker.getNextStateChange();
    EXPECT_EQ(AIConstruct::State::DAMAGED, stateChange.first);
    EXPECT_EQ(AIConstruct::State::RETREAT, stateChange.second);
    
    // Initial position before retreat
    glm::vec3 initialPosition = construct->getPosition();
    
    // Move player closer
    playerPosition = initialPosition + glm::vec3(3.0f, 0.0f, 0.0f);
    
    // Update to retreat
    for (int i = 0; i < 10; i++) {
        construct->update(0.1f, playerPosition);
    }
    
    // Should be moving away from player
    glm::vec3 retreatDirection = construct->getPosition() - initialPosition;
    glm::vec3 playerDirection = playerPosition - initialPosition;
    float dotProduct = glm::dot(glm::normalize(retreatDirection), glm::normalize(playerDirection));
    
    // Dot product should be negative if moving away from player
    EXPECT_LT(dotProduct, 0.0f);
}

TEST_F(AIConstructTest, PatrolMovement) {
    // Start in patrol state
    construct->forceState(AIConstruct::State::PATROL);
    stateTracker.clear();
    
    // Record initial position
    glm::vec3 initialPosition = construct->getPosition();
    
    // Update to move along patrol route
    for (int i = 0; i < 50; i++) {
        construct->update(0.1f, playerPosition);
    }
    
    // Should have moved from initial position
    EXPECT_NE(initialPosition, construct->getPosition());
    
    // Should be moving toward one of the patrol points
    bool isOnPatrolRoute = false;
    glm::vec3 currentPosition = construct->getPosition();
    
    std::vector<glm::vec3> patrolPoints = {
        glm::vec3(5.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(-5.0f, 0.0f, 0.0f)
    };
    
    for (const auto& point : patrolPoints) {
        if (glm::distance(currentPosition, point) < glm::distance(initialPosition, point)) {
            isOnPatrolRoute = true;
            break;
        }
    }
    
    EXPECT_TRUE(isOnPatrolRoute);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 