#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <unordered_map>

namespace TurtleEngine {
namespace Graphics {
    // Mock ParticleSystem implementation
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                        const glm::vec4& color, float size, float life) {
            std::cout << "[ParticleSystem] Emitted particle at (" 
                    << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
        
        virtual void emitBurst(const glm::vec3& position, size_t count,
                            float minVel, float maxVel,
                            const glm::vec4& color, float size, float life) {
            std::cout << "[ParticleSystem] Emitted burst of " << count << " particles" << std::endl;
        }
    };
}

namespace Combat {
    enum class DamageType {
        PHYSICAL,
        PLASMA,
        SONIC,
        TEMPORAL,
        PSYCHIC
    };

    struct DamageInfo {
        float amount;
        DamageType type;
        glm::vec3 source;
        bool isCritical;

        DamageInfo(float amount_ = 0.0f, 
                DamageType type_ = DamageType::PHYSICAL,
                const glm::vec3& source_ = glm::vec3(0.0f),
                bool isCritical_ = false)
            : amount(amount_), type(type_), source(source_), isCritical(isCritical_) {}
    };

    // Resilience class for damage resistance
    class Resilience {
    public:
        Resilience() : m_flatReduction(0.0f), m_shield(0.0f) {
            for (int i = 0; i < 5; ++i) {
                m_resistances[i] = 0.0f;
            }
        }
        
        void setResistance(DamageType type, float value) {
            int index = static_cast<int>(type);
            m_resistances[index] = std::max(0.0f, std::min(1.0f, value));
        }
        
        float getResistance(DamageType type) const {
            return m_resistances[static_cast<int>(type)];
        }
        
        void setFlatReduction(float value) {
            m_flatReduction = std::max(0.0f, value);
        }
        
        float getFlatReduction() const {
            return m_flatReduction;
        }
        
        void setShield(float value) {
            m_shield = std::max(0.0f, value);
        }
        
        float getShield() const {
            return m_shield;
        }
        
    private:
        float m_resistances[5];
        float m_flatReduction;
        float m_shield;
    };

    // HealthComponent implementation
    class HealthComponent {
    public:
        HealthComponent(float maxHealth = 100.0f, 
                    std::shared_ptr<Graphics::ParticleSystem> particleSystem = nullptr)
            : m_currentHealth(maxHealth), 
            m_maxHealth(maxHealth),
            m_position(0.0f),
            m_isAlive(true),
            m_regenerationRate(0.0f),
            m_particleSystem(particleSystem) {
            std::cout << "[HealthComponent] Created with max health: " << maxHealth << std::endl;
        }

        float applyDamage(const DamageInfo& damage) {
            if (!m_isAlive) return 0.0f;
            
            float actualDamage = damage.amount;
            
            // Apply shield first if available
            float remainingShield = m_resilience.getShield();
            if (remainingShield > 0.0f) {
                if (actualDamage <= remainingShield) {
                    m_resilience.setShield(remainingShield - actualDamage);
                    std::cout << "[HealthComponent] Shield absorbed " << actualDamage 
                            << " damage, shield remaining: " << m_resilience.getShield() << std::endl;
                    
                    // Create shield hit particles if particle system exists
                    if (m_particleSystem) {
                        createShieldHitParticles(damage, actualDamage);
                    }
                    
                    if (m_damageCallback) m_damageCallback(damage, 0.0f);
                    return 0.0f;
                } else {
                    actualDamage -= remainingShield;
                    m_resilience.setShield(0.0f);
                    std::cout << "[HealthComponent] Shield depleted, " << actualDamage 
                            << " damage passes through" << std::endl;
                    
                    // Create shield break particles if particle system exists
                    if (m_particleSystem) {
                        createShieldBreakParticles();
                    }
                }
            }
            
            // Apply damage reduction from resilience
            if (damage.type != DamageType::PSYCHIC) { // Psychic damage ignores armor
                float resistance = m_resilience.getResistance(damage.type);
                float reducedDamage = actualDamage * (1.0f - resistance);
                
                std::cout << "[HealthComponent] Resistance reduced damage from " 
                        << actualDamage << " to " << reducedDamage 
                        << " (" << (resistance * 100.0f) << "% reduction)" << std::endl;
                
                actualDamage = reducedDamage;
                
                // Apply flat reduction
                float flatReduction = m_resilience.getFlatReduction();
                if (flatReduction > 0.0f) {
                    float finalDamage = std::max(0.0f, actualDamage - flatReduction);
                    
                    std::cout << "[HealthComponent] Flat reduction reduced damage from " 
                            << actualDamage << " to " << finalDamage 
                            << " (" << flatReduction << " flat reduction)" << std::endl;
                    
                    actualDamage = finalDamage;
                }
            }
            
            // Apply critical damage
            if (damage.isCritical) {
                float critDamage = actualDamage * 2.0f;
                std::cout << "[HealthComponent] Critical hit! Damage increased from " 
                        << actualDamage << " to " << critDamage << std::endl;
                
                actualDamage = critDamage;
            }
            
            m_currentHealth -= actualDamage;
            
            std::cout << "[HealthComponent] Applied " << actualDamage 
                    << " damage, health now: " << m_currentHealth << "/" << m_maxHealth << std::endl;
            
            if (m_currentHealth <= 0.0f) {
                m_currentHealth = 0.0f;
                m_isAlive = false;
                std::cout << "[HealthComponent] Entity died" << std::endl;
                if (m_deathCallback) m_deathCallback();
            }
            
            // Create damage particles if particle system exists
            if (m_particleSystem) {
                createDamageParticles(damage, actualDamage);
            }
            
            if (m_damageCallback) m_damageCallback(damage, actualDamage);
            
            return actualDamage;
        }

        float applyHealing(float amount, const glm::vec3& source = glm::vec3(0.0f)) {
            if (!m_isAlive || amount <= 0.0f) return 0.0f;
            
            float actualHealing = std::min(amount, m_maxHealth - m_currentHealth);
            m_currentHealth += actualHealing;
            
            std::cout << "[HealthComponent] Healed for " << actualHealing 
                    << ", health now: " << m_currentHealth << "/" << m_maxHealth << std::endl;
            
            // Create healing particles if particle system exists
            if (m_particleSystem && actualHealing > 0.0f) {
                createHealingParticles(source, actualHealing);
            }
            
            return actualHealing;
        }

        void update(float deltaTime) {
            if (!m_isAlive || m_regenerationRate <= 0.0f) return;
            
            float regenAmount = m_regenerationRate * deltaTime;
            if (regenAmount > 0.0f && m_currentHealth < m_maxHealth) {
                float actualRegen = std::min(regenAmount, m_maxHealth - m_currentHealth);
                m_currentHealth += actualRegen;
                
                std::cout << "[HealthComponent] Regenerated " << actualRegen 
                        << " health, now: " << m_currentHealth << "/" << m_maxHealth << std::endl;
                
                // Create subtle regen particles if particle system exists
                if (m_particleSystem && actualRegen > 0.0f) {
                    createRegenParticles(actualRegen);
                }
            }
        }

        void setPosition(const glm::vec3& position) { m_position = position; }
        glm::vec3 getPosition() const { return m_position; }
        float getCurrentHealth() const { return m_currentHealth; }
        float getMaxHealth() const { return m_maxHealth; }
        bool isAlive() const { return m_isAlive; }
        float getHealthPercentage() const { return m_maxHealth > 0.0f ? (m_currentHealth / m_maxHealth) : 0.0f; }
        
        void setRegenerationRate(float rate) { m_regenerationRate = std::max(0.0f, rate); }
        float getRegenerationRate() const { return m_regenerationRate; }
        
        void setDamageCallback(std::function<void(const DamageInfo&, float)> callback) {
            m_damageCallback = callback;
        }
        
        void setDeathCallback(std::function<void()> callback) {
            m_deathCallback = callback;
        }
        
        Resilience& getResilience() { return m_resilience; }

    private:
        void createDamageParticles(const DamageInfo& damage, float actualDamage) {
            // Set particle color based on damage type
            glm::vec4 color;
            switch (damage.type) {
                case DamageType::PHYSICAL:
                    color = glm::vec4(0.8f, 0.0f, 0.0f, 0.8f); // Red
                    break;
                case DamageType::PLASMA:
                    color = glm::vec4(0.0f, 0.8f, 1.0f, 0.8f); // Cyan
                    break;
                case DamageType::SONIC:
                    color = glm::vec4(0.0f, 0.8f, 0.0f, 0.8f); // Green
                    break;
                case DamageType::TEMPORAL:
                    color = glm::vec4(0.8f, 0.0f, 0.8f, 0.8f); // Purple
                    break;
                case DamageType::PSYCHIC:
                    color = glm::vec4(1.0f, 0.8f, 0.0f, 0.8f); // Yellow
                    break;
            }
            
            // Determine particle count based on damage
            size_t particleCount = static_cast<size_t>(std::min(100.0f, actualDamage));
            
            // Create directional particles in opposite direction of source
            glm::vec3 direction = m_position - damage.source;
            if (glm::length(direction) > 0.0001f) {
                direction = glm::normalize(direction);
            } else {
                direction = glm::vec3(0.0f, 1.0f, 0.0f); // Default up if no direction
            }
            
            m_particleSystem->emitBurst(
                m_position, 
                particleCount,
                1.0f, 3.0f,
                color,
                0.1f, 0.5f
            );
            
            // Extra particles for critical hits
            if (damage.isCritical) {
                m_particleSystem->emitBurst(
                    m_position, 
                    particleCount * 2,
                    2.0f, 5.0f,
                    glm::vec4(1.0f, 1.0f, 0.0f, 0.9f), // Bright yellow for crits
                    0.15f, 0.7f
                );
            }
        }
        
        void createHealingParticles(const glm::vec3& source, float healAmount) {
            // Green healing particles rising upward
            glm::vec4 healColor(0.0f, 0.9f, 0.3f, 0.8f);
            
            size_t particleCount = static_cast<size_t>(std::min(50.0f, healAmount));
            
            m_particleSystem->emitBurst(
                m_position, 
                particleCount,
                0.5f, 1.5f,
                healColor,
                0.1f, 1.0f
            );
        }
        
        void createRegenParticles(float regenAmount) {
            // Subtle green particles for regeneration
            glm::vec4 regenColor(0.0f, 0.7f, 0.2f, 0.5f);
            
            size_t particleCount = static_cast<size_t>(std::min(10.0f, regenAmount * 5.0f));
            
            if (particleCount > 0) {
                m_particleSystem->emitBurst(
                    m_position, 
                    particleCount,
                    0.2f, 0.5f,
                    regenColor,
                    0.05f, 0.8f
                );
            }
        }
        
        void createShieldHitParticles(const DamageInfo& damage, float absorbedDamage) {
            // Blue shield particles
            glm::vec4 shieldColor(0.4f, 0.4f, 1.0f, 0.7f);
            
            size_t particleCount = static_cast<size_t>(std::min(30.0f, absorbedDamage));
            
            m_particleSystem->emitBurst(
                m_position, 
                particleCount,
                1.0f, 3.0f,
                shieldColor,
                0.1f, 0.3f
            );
        }
        
        void createShieldBreakParticles() {
            // Bright blue shield break particles
            glm::vec4 breakColor(0.2f, 0.2f, 1.0f, 0.9f);
            
            m_particleSystem->emitBurst(
                m_position, 
                50,
                2.0f, 8.0f,
                breakColor,
                0.1f, 0.5f
            );
        }

        float m_currentHealth;
        float m_maxHealth;
        glm::vec3 m_position;
        bool m_isAlive;
        float m_regenerationRate;
        std::shared_ptr<Graphics::ParticleSystem> m_particleSystem;
        std::function<void(const DamageInfo&, float)> m_damageCallback;
        std::function<void()> m_deathCallback;
        Resilience m_resilience;
    };
}
} // namespace TurtleEngine

int main() {
    std::cout << "=== HealthSystem Test ===\n";

    // Create a particle system for visual effects
    auto particleSystem = std::make_shared<TurtleEngine::Graphics::ParticleSystem>();
    
    // Test 1: Basic damage and damage types
    std::cout << "\nTest 1: Basic damage and damage types\n";
    auto health = std::make_shared<TurtleEngine::Combat::HealthComponent>(100.0f, particleSystem);
    
    // Apply physical damage
    TurtleEngine::Combat::DamageInfo physicalDamage(20.0f, TurtleEngine::Combat::DamageType::PHYSICAL);
    float damageApplied = health->applyDamage(physicalDamage);
    
    bool damageTest = (damageApplied == 20.0f && health->getCurrentHealth() == 80.0f);
    std::cout << "Basic Damage Test: " << (damageTest ? "PASSED" : "FAILED") << std::endl;
    
    // Test 2: Resilience and resistance
    std::cout << "\nTest 2: Resilience and resistance\n";
    auto& resilience = health->getResilience();
    resilience.setResistance(TurtleEngine::Combat::DamageType::PLASMA, 0.5f); // 50% plasma resistance
    
    TurtleEngine::Combat::DamageInfo plasmaDamage(20.0f, TurtleEngine::Combat::DamageType::PLASMA);
    damageApplied = health->applyDamage(plasmaDamage);
    
    bool resistanceTest = (damageApplied == 10.0f); // Should be reduced by 50%
    std::cout << "Resistance Test: " << (resistanceTest ? "PASSED" : "FAILED") 
              << " (Applied: " << damageApplied << ", Expected: 10.0)" << std::endl;
    
    // Test 3: Healing
    std::cout << "\nTest 3: Healing\n";
    float healingApplied = health->applyHealing(15.0f);
    
    bool healingTest = (healingApplied == 15.0f && health->getCurrentHealth() == 85.0f);
    std::cout << "Healing Test: " << (healingTest ? "PASSED" : "FAILED") 
              << " (Healed: " << healingApplied << ", Health: " << health->getCurrentHealth() << ")" << std::endl;
    
    // Test 4: Regeneration
    std::cout << "\nTest 4: Regeneration\n";
    health->setRegenerationRate(5.0f); // 5 HP per second
    health->update(1.0f); // Update for 1 second
    
    bool regenTest = (health->getCurrentHealth() == 90.0f);
    std::cout << "Regeneration Test: " << (regenTest ? "PASSED" : "FAILED") 
              << " (Health after regen: " << health->getCurrentHealth() << ")" << std::endl;
    
    // Test 5: Critical damage
    std::cout << "\nTest 5: Critical damage\n";
    TurtleEngine::Combat::DamageInfo criticalDamage(10.0f, TurtleEngine::Combat::DamageType::PHYSICAL, 
                                                  glm::vec3(0.0f), true);
    damageApplied = health->applyDamage(criticalDamage);
    
    bool critTest = (damageApplied == 20.0f); // Should be doubled
    std::cout << "Critical Damage Test: " << (critTest ? "PASSED" : "FAILED") 
              << " (Applied: " << damageApplied << ", Expected: 20.0)" << std::endl;
    
    // Test 6: Shield absorption
    std::cout << "\nTest 6: Shield absorption\n";
    auto shieldedHealth = std::make_shared<TurtleEngine::Combat::HealthComponent>(100.0f, particleSystem);
    shieldedHealth->getResilience().setShield(30.0f);
    
    TurtleEngine::Combat::DamageInfo shieldTestDamage(20.0f, TurtleEngine::Combat::DamageType::PHYSICAL);
    damageApplied = shieldedHealth->applyDamage(shieldTestDamage);
    
    bool shieldTest = (damageApplied == 0.0f && shieldedHealth->getCurrentHealth() == 100.0f &&
                     shieldedHealth->getResilience().getShield() == 10.0f);
    std::cout << "Shield Absorption Test: " << (shieldTest ? "PASSED" : "FAILED") 
              << " (Health: " << shieldedHealth->getCurrentHealth() 
              << ", Shield: " << shieldedHealth->getResilience().getShield() << ")" << std::endl;
    
    // Test 7: Death
    std::cout << "\nTest 7: Death\n";
    bool deathCallbackCalled = false;
    health->setDeathCallback([&deathCallbackCalled]() {
        deathCallbackCalled = true;
    });
    
    TurtleEngine::Combat::DamageInfo lethalDamage(100.0f, TurtleEngine::Combat::DamageType::PHYSICAL);
    health->applyDamage(lethalDamage);
    
    bool deathTest = (health->getCurrentHealth() == 0.0f && !health->isAlive() && deathCallbackCalled);
    std::cout << "Death Test: " << (deathTest ? "PASSED" : "FAILED") 
              << " (Health: " << health->getCurrentHealth() 
              << ", IsAlive: " << (health->isAlive() ? "true" : "false")
              << ", Callback: " << (deathCallbackCalled ? "called" : "not called") << ")" << std::endl;
    
    // Summary
    bool allTestsPassed = damageTest && resistanceTest && healingTest && 
                         regenTest && critTest && shieldTest && deathTest;
    
    std::cout << "\n=== HealthSystem Test Complete ===\n";
    std::cout << "Overall Result: " << (allTestsPassed ? "PASSED" : "FAILED") << std::endl;
    
    return allTestsPassed ? 0 : 1;
}
