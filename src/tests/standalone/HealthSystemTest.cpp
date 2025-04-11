#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include "engine/combat/include/HealthSystem.hpp"

// Mock ParticleSystem that implements the minimum functionality needed for the test
namespace TurtleEngine {
namespace Graphics {
    // Create mock implementations of the methods used by HealthComponent
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        // Simplified emit method for testing
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted particle at (" 
                     << position.x << ", " << position.y << ", " << position.z << ")"
                     << " with color " << color.r << "," << color.g << "," << color.b 
                     << " size " << size << " life " << life << std::endl;
        }
        
        // Mock methods that are not used by our test but might be called
        virtual void initialize() {}
        virtual void update(float deltaTime) {}
        virtual void render() {}
        
        virtual void emitBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted burst of " << count << " particles." << std::endl;
        }
    };
}
}

// Our simple test helper
class TestHelper {
public:
    // Test the HealthSystem functionality
    static bool RunTests() {
        std::cout << "=== HealthSystem Test ===\n";

        // Create a particle system
        auto particleSystem = std::make_shared<TurtleEngine::Graphics::ParticleSystem>();

        // Test 1: Basic HealthComponent Creation and Properties
        std::cout << "\nTest 1: Basic HealthComponent Creation\n";
        TurtleEngine::Combat::HealthComponent health(100.0f, particleSystem);
        bool creationSuccess = (health.getCurrentHealth() == 100.0f && 
                              health.getMaxHealth() == 100.0f &&
                              health.isAlive());
        std::cout << "Creation Test: " << (creationSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 2: Damage Application
        std::cout << "\nTest 2: Damage Application\n";
        TurtleEngine::Combat::DamageInfo damage(30.0f, 
                                              TurtleEngine::Combat::DamageType::PLASMA,
                                              glm::vec3(0.0f));
        float actualDamage = health.applyDamage(damage);
        bool damageSuccess = (health.getCurrentHealth() == 70.0f && 
                             actualDamage == 30.0f);
        std::cout << "Damage Test: " << (damageSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 3: Healing
        std::cout << "\nTest 3: Healing\n";
        float healingAmount = health.applyHealing(20.0f, glm::vec3(0.0f));
        bool healingSuccess = (health.getCurrentHealth() == 90.0f && 
                              healingAmount == 20.0f);
        std::cout << "Healing Test: " << (healingSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 4: Resilience System
        std::cout << "\nTest 4: Resilience System\n";
        auto& resilience = health.getResilience();
        resilience.setResistance(TurtleEngine::Combat::DamageType::PLASMA, 0.5f); // 50% resistance
        resilience.setFlatReduction(5.0f); // 5 points flat reduction
        
        TurtleEngine::Combat::DamageInfo plasmaDamage(40.0f, 
                                                     TurtleEngine::Combat::DamageType::PLASMA,
                                                     glm::vec3(0.0f));
        float reducedDamage = health.applyDamage(plasmaDamage);
        // Expected damage: (40 - 5) * (1 - 0.5) = 35 * 0.5 = 17.5
        // Expected health: 90 - 17.5 = 72.5
        bool resilienceSuccess = (std::abs(reducedDamage - 17.5f) < 0.01f && 
                                std::abs(health.getCurrentHealth() - 72.5f) < 0.01f);
        std::cout << "Resilience Test: " << (resilienceSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 5: Death and Callbacks
        std::cout << "\nTest 5: Death and Callbacks\n";
        bool deathCallbackCalled = false;
        health.setDeathCallback([&deathCallbackCalled]() {
            deathCallbackCalled = true;
            std::cout << "Death callback triggered!\n";
        });
        
        TurtleEngine::Combat::DamageInfo lethalDamage(200.0f, 
                                                     TurtleEngine::Combat::DamageType::PHYSICAL,
                                                     glm::vec3(0.0f));
        health.applyDamage(lethalDamage);
        bool deathSuccess = (!health.isAlive() && deathCallbackCalled);
        std::cout << "Death Test: " << (deathSuccess ? "PASSED" : "FAILED") 
                  << " (Health: " << health.getCurrentHealth() << "/" 
                  << health.getMaxHealth() << ")\n";

        // Test 6: Debug Visualization
        std::cout << "\nTest 6: Debug Visualization\n";
        health.enableDebugVisualization(true);
        std::string debugInfo = health.getDebugInfo();
        bool debugSuccess = (!debugInfo.empty() && 
                            debugInfo.find("Health:") != std::string::npos);
        std::cout << "Debug Test: " << (debugSuccess ? "PASSED" : "FAILED") 
                  << " (Debug Info: " << debugInfo << ")\n";

        std::cout << "\n=== HealthSystem Test Complete ===\n";
        return (creationSuccess && damageSuccess && healingSuccess && 
                resilienceSuccess && deathSuccess && debugSuccess);
    }
};

int main() {
    bool success = TestHelper::RunTests();
    return success ? 0 : 1;
} 