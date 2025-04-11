#include <iostream>
#include <glm/glm.hpp>
#include "../../engine/combat/include/HealthSystem.hpp"

int main() {
    std::cout << "=== HealthComponent Standalone Test ===\n";

    // Create a HealthComponent instance (without ParticleSystem dependency for simplicity)
    TurtleEngine::Combat::HealthComponent health(100.0f, nullptr);

    // Test 1: Apply physical damage
    std::cout << "\nTest 1: Apply physical damage\n";
    float initialHealth = health.getCurrentHealth();
    TurtleEngine::Combat::DamageInfo damage(30.0f, TurtleEngine::Combat::DamageType::PHYSICAL, glm::vec3(0.0f));
    health.applyDamage(damage);
    bool damageSuccess = (health.getCurrentHealth() == (initialHealth - 30.0f));
    std::cout << "Damage Test: " << (damageSuccess ? "PASSED" : "FAILED") 
              << " (Health: " << health.getCurrentHealth() << ")\n";

    // Test 2: Apply healing
    std::cout << "\nTest 2: Apply healing\n";
    initialHealth = health.getCurrentHealth();
    health.applyHealing(20.0f, glm::vec3(0.0f));
    bool healingSuccess = (health.getCurrentHealth() == (initialHealth + 20.0f));
    std::cout << "Healing Test: " << (healingSuccess ? "PASSED" : "FAILED") 
              << " (Health: " << health.getCurrentHealth() << ")\n";

    std::cout << "\n=== HealthComponent Test Complete ===\n";
    return (damageSuccess && healingSuccess) ? 0 : 1;
} 