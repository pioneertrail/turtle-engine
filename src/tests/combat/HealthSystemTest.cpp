#include "../../engine/combat/include/HealthSystem.hpp"
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <string>
#include <functional>

using namespace TurtleEngine;
using namespace TurtleEngine::Combat;

// Mock ParticleSystem for testing
class MockParticleSystem : public ParticleSystem {
public:
    MockParticleSystem() : ParticleSystem(1000) {}
    
    void spawnParticle(const Particle& particle) override {
        m_spawnedParticles.push_back(particle);
    }
    
    size_t getParticleCount() const {
        return m_spawnedParticles.size();
    }
    
    void reset() {
        m_spawnedParticles.clear();
    }
    
private:
    std::vector<Particle> m_spawnedParticles;
};

// Test fixture
class HealthSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        particleSystem = std::make_shared<MockParticleSystem>();
        health = std::make_unique<HealthComponent>(100.0f, particleSystem);
        
        // Set up callbacks for testing
        damageReceived = 0.0f;
        healingReceived = 0.0f;
        deathCalled = false;
        
        health->setDamageCallback([this](const DamageInfo& info, float amount) {
            damageReceived = amount;
            lastDamageInfo = info;
        });
        
        health->setHealingCallback([this](float amount, const glm::vec3& source) {
            healingReceived = amount;
            healingSource = source;
        });
        
        health->setDeathCallback([this]() {
            deathCalled = true;
        });
        
        // Set position for visual effects
        health->setPosition(glm::vec3(0.0f, 1.0f, 0.0f));
        
        // Enable debug visualization
        health->enableDebugVisualization(true);
    }
    
    void TearDown() override {
        health.reset();
        particleSystem.reset();
    }
    
    std::shared_ptr<MockParticleSystem> particleSystem;
    std::unique_ptr<HealthComponent> health;
    
    // Callback tracking
    float damageReceived;
    float healingReceived;
    bool deathCalled;
    DamageInfo lastDamageInfo;
    glm::vec3 healingSource;
};

TEST_F(HealthSystemTest, Initialization) {
    // Test initial state
    EXPECT_EQ(100.0f, health->getMaxHealth());
    EXPECT_EQ(100.0f, health->getCurrentHealth());
    EXPECT_EQ(1.0f, health->getHealthPercentage());
    EXPECT_TRUE(health->isAlive());
    EXPECT_TRUE(health->isDebugVisualizationEnabled());
}

TEST_F(HealthSystemTest, BasicDamage) {
    // Create basic damage info
    DamageInfo damage(20.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Apply damage
    float actualDamage = health->applyDamage(damage);
    
    // Check health state
    EXPECT_EQ(20.0f, actualDamage);
    EXPECT_EQ(80.0f, health->getCurrentHealth());
    EXPECT_EQ(0.8f, health->getHealthPercentage());
    EXPECT_TRUE(health->isAlive());
    
    // Check callback
    EXPECT_EQ(20.0f, damageReceived);
    EXPECT_EQ(DamageType::PHYSICAL, lastDamageInfo.type);
    
    // Check particles
    EXPECT_GT(particleSystem->getParticleCount(), 0);
}

TEST_F(HealthSystemTest, DamageResistance) {
    // Set up resistances
    Resilience& resilience = health->getResilience();
    resilience.setResistance(DamageType::PLASMA, 0.5f); // 50% resistance
    
    // Create damage of the resistant type
    DamageInfo plasmaAttack(20.0f, DamageType::PLASMA, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Apply damage
    float actualDamage = health->applyDamage(plasmaAttack);
    
    // Check that damage was reduced by resistance
    EXPECT_EQ(10.0f, actualDamage); // 20 * (1 - 0.5) = 10
    EXPECT_EQ(90.0f, health->getCurrentHealth());
}

TEST_F(HealthSystemTest, CriticalHits) {
    // Set up resistances
    Resilience& resilience = health->getResilience();
    resilience.setResistance(DamageType::PHYSICAL, 0.5f); // 50% resistance
    
    // Create critical hit
    DamageInfo criticalHit(20.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    criticalHit.isCritical = true;
    
    // Apply damage
    float actualDamage = health->applyDamage(criticalHit);
    
    // Check that critical hit partially ignored resistance
    // Critical hits ignore 50% of resistance, so effective resistance is 25%
    // 20 * (1 - 0.25) = 15
    EXPECT_EQ(15.0f, actualDamage);
    EXPECT_EQ(85.0f, health->getCurrentHealth());
}

TEST_F(HealthSystemTest, FlatDamageReduction) {
    // Set up flat reduction
    Resilience& resilience = health->getResilience();
    resilience.setFlatReduction(5.0f);
    
    // Create damage
    DamageInfo damage(20.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Apply damage
    float actualDamage = health->applyDamage(damage);
    
    // Check that flat reduction was applied
    EXPECT_EQ(15.0f, actualDamage); // 20 - 5 = 15
    EXPECT_EQ(85.0f, health->getCurrentHealth());
}

TEST_F(HealthSystemTest, ShieldAbsorption) {
    // Set up shield
    Resilience& resilience = health->getResilience();
    resilience.setShield(10.0f);
    
    // Create damage
    DamageInfo damage(20.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    
    // Apply damage
    float actualDamage = health->applyDamage(damage);
    
    // Check that shield absorbed part of the damage
    EXPECT_EQ(10.0f, actualDamage); // 20 - 10 = 10
    EXPECT_EQ(90.0f, health->getCurrentHealth());
    EXPECT_EQ(0.0f, resilience.getShield()); // Shield should be depleted
}

TEST_F(HealthSystemTest, Healing) {
    // First apply some damage
    DamageInfo damage(40.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    health->applyDamage(damage);
    
    // Reset particle count
    particleSystem->reset();
    
    // Apply healing
    glm::vec3 healSource(0.0f, 5.0f, 0.0f);
    float actualHealing = health->applyHealing(20.0f, healSource);
    
    // Check healing applied
    EXPECT_EQ(20.0f, actualHealing);
    EXPECT_EQ(80.0f, health->getCurrentHealth());
    
    // Check callback
    EXPECT_EQ(20.0f, healingReceived);
    EXPECT_EQ(healSource, healingSource);
    
    // Check healing particles
    EXPECT_GT(particleSystem->getParticleCount(), 0);
}

TEST_F(HealthSystemTest, OverHealing) {
    // Apply damage
    DamageInfo damage(20.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    health->applyDamage(damage);
    
    // Apply excessive healing
    float actualHealing = health->applyHealing(30.0f);
    
    // Check healing was capped at missing health
    EXPECT_EQ(20.0f, actualHealing);
    EXPECT_EQ(100.0f, health->getCurrentHealth());
}

TEST_F(HealthSystemTest, Death) {
    // Apply lethal damage
    DamageInfo damage(110.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    float actualDamage = health->applyDamage(damage);
    
    // Check death occurred
    EXPECT_EQ(100.0f, actualDamage); // Limited by available health
    EXPECT_EQ(0.0f, health->getCurrentHealth());
    EXPECT_FALSE(health->isAlive());
    EXPECT_TRUE(deathCalled);
    
    // Verify no more damage can be applied
    particleSystem->reset();
    float damageAfterDeath = health->applyDamage(damage);
    EXPECT_EQ(0.0f, damageAfterDeath);
    EXPECT_EQ(0, particleSystem->getParticleCount());
    
    // Verify healing doesn't work after death
    float healingAfterDeath = health->applyHealing(50.0f);
    EXPECT_EQ(0.0f, healingAfterDeath);
    EXPECT_EQ(0.0f, health->getCurrentHealth());
}

TEST_F(HealthSystemTest, DebugInfo) {
    // Check initial debug info
    std::string initialInfo = health->getDebugInfo();
    EXPECT_FALSE(initialInfo.empty());
    EXPECT_TRUE(initialInfo.find("Health: 100/100") != std::string::npos);
    
    // Apply damage and check updated info
    DamageInfo damage(30.0f, DamageType::PHYSICAL, glm::vec3(1.0f, 0.0f, 0.0f));
    health->applyDamage(damage);
    
    std::string updatedInfo = health->getDebugInfo();
    EXPECT_TRUE(updatedInfo.find("Health: 70/100") != std::string::npos);
    
    // Add shield and resistance and check info again
    Resilience& resilience = health->getResilience();
    resilience.setShield(20.0f);
    resilience.setResistance(DamageType::PLASMA, 0.75f);
    
    std::string fullInfo = health->getDebugInfo();
    EXPECT_TRUE(fullInfo.find("Shield: 20") != std::string::npos);
    EXPECT_TRUE(fullInfo.find("Res:") != std::string::npos);
    
    // Disable debug visualization and check empty info
    health->enableDebugVisualization(false);
    EXPECT_TRUE(health->getDebugInfo().empty());
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 