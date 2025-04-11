#include <iostream>
#include <glm/glm.hpp>
#include "../../engine/combat/include/PlasmaWeapon.hpp"

int main() {
    std::cout << "=== PlasmaWeapon Standalone Test ===\n";

    // Create a PlasmaWeapon instance (without ParticleSystem dependency for simplicity)
    TurtleEngine::Combat::PlasmaWeapon weapon(nullptr);

    // Test 1: Charge and fire in BURST mode
    std::cout << "\nTest 1: Charge and fire in BURST mode\n";
    weapon.setFiringMode(TurtleEngine::Combat::FiringMode::BURST);
    weapon.startCharging();
    for (int i = 0; i < 10; ++i) {
        weapon.update(0.2f); // Simulate 2 seconds of charging (0.2s * 10)
    }
    bool fireSuccess = weapon.fire(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bool fireResult = fireSuccess && (weapon.getCurrentCharge() < TurtleEngine::Combat::PlasmaWeaponConstants::MIN_FIRE_CHARGE);
    std::cout << "Fire Test (BURST): " << (fireResult ? "PASSED" : "FAILED") 
              << " (Charge after firing: " << weapon.getCurrentCharge() << ")\n";

    // Test 2: Quick-fire with Flammil gesture
    std::cout << "\nTest 2: Quick-fire with Flammil gesture\n";
    weapon.startCharging();
    weapon.update(0.5f); // Charge for 0.5 seconds
    bool quickFireSuccess = weapon.quickFireWithFlammil(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bool quickFireResult = quickFireSuccess && (weapon.getCurrentCooldown() > 0.0f);
    std::cout << "Quick-Fire Test: " << (quickFireResult ? "PASSED" : "FAILED") 
              << " (Cooldown: " << weapon.getCurrentCooldown() << "s)\n";

    std::cout << "\n=== PlasmaWeapon Test Complete ===\n";
    return (fireResult && quickFireResult) ? 0 : 1;
} 