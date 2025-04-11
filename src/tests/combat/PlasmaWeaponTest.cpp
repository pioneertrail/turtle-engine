#include "../../engine/combat/include/PlasmaWeapon.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <chrono>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

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
    
    int getParticleCount() const { return m_particleCount; }
    Particle getLastParticle() const { return m_lastParticle; }
    
    void resetCounters() { m_particleCount = 0; }
    
private:
    int m_particleCount;
    Particle m_lastParticle;
};

class PlasmaWeaponTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Create mock particle system
        particleSystem = std::make_shared<MockParticleSystem>();
        
        // Create plasma weapon
        weapon = std::make_unique<PlasmaWeapon>(100.0f);
        weapon->initialize(particleSystem);
        
        // Enable debug visualization for testing
        weapon->enableDebugVisualization(true);
    }
    
    std::shared_ptr<MockParticleSystem> particleSystem;
    std::unique_ptr<PlasmaWeapon> weapon;
};

TEST_F(PlasmaWeaponTest, Initialize) {
    PlasmaWeapon testWeapon;
    EXPECT_TRUE(testWeapon.initialize(particleSystem));
    EXPECT_FALSE(testWeapon.initialize(nullptr)); // Should fail with null particle system
}

TEST_F(PlasmaWeaponTest, Charging) {
    // Test charging
    EXPECT_FLOAT_EQ(0.0f, weapon->getChargePercentage());
    weapon->beginCharging();
    
    // Update with 0.5 second delta time (should charge by 25 units)
    weapon->update(0.5f);
    EXPECT_NEAR(0.25f, weapon->getChargePercentage(), 0.01f);
    
    // Update with another 1.0 second (should charge by another 50 units)
    weapon->update(1.0f);
    EXPECT_NEAR(0.75f, weapon->getChargePercentage(), 0.01f);
    
    // Update with another 1.0 second (should max out at 100%)
    weapon->update(1.0f);
    EXPECT_NEAR(1.0f, weapon->getChargePercentage(), 0.01f);
}

TEST_F(PlasmaWeaponTest, Firing) {
    // Reset particle counter
    particleSystem->resetCounters();
    
    // Set up test parameters
    glm::vec3 origin(0.0f, 0.0f, 0.0f);
    glm::vec3 direction(0.0f, 0.0f, 1.0f);
    
    // Try firing without charge (should fail)
    EXPECT_FALSE(weapon->fire(origin, direction));
    EXPECT_EQ(0, particleSystem->getParticleCount());
    
    // Charge weapon
    weapon->beginCharging();
    weapon->update(1.0f); // Charge for 1 second (50 units)
    
    // Fire weapon (should succeed)
    EXPECT_TRUE(weapon->fire(origin, direction));
    EXPECT_GT(particleSystem->getParticleCount(), 0);
    
    // Check that charge was reset
    EXPECT_FLOAT_EQ(0.0f, weapon->getChargePercentage());
    
    // Check that weapon is in cooldown
    EXPECT_TRUE(weapon->isCoolingDown());
    
    // Try firing during cooldown (should fail)
    particleSystem->resetCounters();
    EXPECT_FALSE(weapon->fire(origin, direction));
    EXPECT_EQ(0, particleSystem->getParticleCount());
    
    // Wait for cooldown to end
    weapon->update(1.0f); // Cooldown is 0.5 seconds, so this should clear it
    EXPECT_FALSE(weapon->isCoolingDown());
}

TEST_F(PlasmaWeaponTest, QuickFire) {
    // Reset particle counter
    particleSystem->resetCounters();
    
    // Set up test parameters
    glm::vec3 origin(0.0f, 0.0f, 0.0f);
    glm::vec3 direction(0.0f, 0.0f, 1.0f);
    
    // Quick fire (should succeed without charging)
    EXPECT_TRUE(weapon->quickFire(origin, direction));
    EXPECT_GT(particleSystem->getParticleCount(), 0);
    
    // Check that weapon is in cooldown
    EXPECT_TRUE(weapon->isCoolingDown());
    
    // Try quick firing during cooldown (should fail)
    particleSystem->resetCounters();
    EXPECT_FALSE(weapon->quickFire(origin, direction));
    EXPECT_EQ(0, particleSystem->getParticleCount());
}

TEST_F(PlasmaWeaponTest, FiringModes) {
    // Reset particle counter
    particleSystem->resetCounters();
    
    // Set up test parameters
    glm::vec3 origin(0.0f, 0.0f, 0.0f);
    glm::vec3 direction(0.0f, 0.0f, 1.0f);
    
    // Test each firing mode
    for (int mode = 0; mode < 4; ++mode) {
        // Set firing mode
        weapon->setFiringMode(static_cast<PlasmaWeapon::FiringMode>(mode));
        
        // Charge and fire
        weapon->beginCharging();
        weapon->update(0.5f); // Charge for 0.5 seconds
        
        // Reset counters for this test
        particleSystem->resetCounters();
        
        // Fire weapon
        weapon->fire(origin, direction);
        
        // Check that particles were created
        EXPECT_GT(particleSystem->getParticleCount(), 0);
        
        // Wait for cooldown to end before next test
        weapon->update(1.0f);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 