#include <iostream>
#include <glm/glm.hpp>
#include "../../engine/combat/include/AIConstruct.hpp"

int main() {
    std::cout << "=== AIConstruct Standalone Test ===\n";

    // Create an AIConstruct instance (without ParticleSystem dependency for simplicity)
    TurtleEngine::Combat::AIConstruct construct(nullptr, 
                                               TurtleEngine::Combat::AIConstruct::Type::SENTRY, 
                                               glm::vec3(0.0f));

    // Test 1: State transition from IDLE to ATTACK
    std::cout << "\nTest 1: State transition from IDLE to ATTACK\n";
    glm::vec3 playerPos(5.0f, 0.0f, 0.0f); // Within detection range
    construct.update(0.1f, playerPos);
    bool stateTransitionSuccess = (construct.getCurrentState() == TurtleEngine::Combat::AIConstruct::State::ATTACK);
    std::cout << "State Transition Test: " << (stateTransitionSuccess ? "PASSED" : "FAILED") 
              << " (Current State: " << static_cast<int>(construct.getCurrentState()) << ")\n";

    // Test 2: Damage application and state change to DAMAGED
    std::cout << "\nTest 2: Damage application and state change to DAMAGED\n";
    float initialHealth = construct.getHealthComponent().getCurrentHealth();
    TurtleEngine::Combat::DamageInfo damage(20.0f, TurtleEngine::Combat::DamageType::PHYSICAL, playerPos);
    construct.getHealthComponent().applyDamage(damage);
    bool damageSuccess = (construct.getHealthComponent().getCurrentHealth() < initialHealth) &&
                        (construct.getCurrentState() == TurtleEngine::Combat::AIConstruct::State::DAMAGED);
    std::cout << "Damage Test: " << (damageSuccess ? "PASSED" : "FAILED") 
              << " (Health: " << construct.getHealthComponent().getCurrentHealth() << ")\n";

    std::cout << "\n=== AIConstruct Test Complete ===\n";
    return (stateTransitionSuccess && damageSuccess) ? 0 : 1;
} 