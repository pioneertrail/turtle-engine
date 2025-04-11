#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <string>

namespace TurtleEngine {
namespace Combat {

// Mock ParticleSystem
class ParticleSystem {
public:
    virtual ~ParticleSystem() = default;
    
    virtual void spawnBurst(const glm::vec3& position, size_t count,
                          float minVel, float maxVel,
                          const glm::vec4& color, float size, float life) {
        std::cout << "[ParticleSystem] Spawned burst of " << count << " particles" << std::endl;
    }
    
    virtual void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                             const glm::vec4& color, float size, float life) {
        std::cout << "[ParticleSystem] Spawned particle" << std::endl;
    }
};

namespace PlasmaWeaponConstants {
    const float MIN_FIRE_CHARGE = 25.0f;
    const float MAX_CHARGE = 100.0f;
    const float CHARGE_RATE = 50.0f;
    const float COOLDOWN_TIME = 0.5f;
}

// PlasmaWeapon implementation
enum class FiringMode {
    BURST,
    BEAM,
    SPREAD,
    PULSE
};

class PlasmaWeapon {
public:
    PlasmaWeapon(std::shared_ptr<ParticleSystem> particleSystem = nullptr) 
        : m_currentCharge(0.0f), m_isCharging(false), m_cooldown(0.0f), 
          m_firingMode(FiringMode::BURST), m_particleSystem(particleSystem) 
    {
        std::cout << "PlasmaWeapon created\n";
    }

    void startCharging() {
        m_isCharging = true;
        std::cout << "Charging started\n";
    }

    void update(float deltaTime) {
        if (m_isCharging) {
            m_currentCharge += PlasmaWeaponConstants::CHARGE_RATE * deltaTime;
            if (m_currentCharge > PlasmaWeaponConstants::MAX_CHARGE) {
                m_currentCharge = PlasmaWeaponConstants::MAX_CHARGE;
            }
            std::cout << "Current charge: " << m_currentCharge << "\n";
        }

        if (m_cooldown > 0.0f) {
            m_cooldown -= deltaTime;
            if (m_cooldown < 0.0f) {
                m_cooldown = 0.0f;
            }
        }
    }

    bool fire(const glm::vec3& origin, const glm::vec3& direction) {
        if (m_cooldown > 0.0f) {
            std::cout << "Cannot fire, weapon is cooling down\n";
            return false;
        }

        if (m_currentCharge < PlasmaWeaponConstants::MIN_FIRE_CHARGE) {
            std::cout << "Cannot fire, insufficient charge\n";
            return false;
        }

        std::cout << "Weapon fired with mode: " << static_cast<int>(m_firingMode) 
                  << " and charge: " << m_currentCharge << "\n";
        
        if (m_particleSystem) {
            // Spawn particles based on firing mode
            glm::vec4 color(0.2f, 0.6f, 1.0f, 1.0f); // Plasma blue color
            
            switch (m_firingMode) {
                case FiringMode::BURST:
                    m_particleSystem->spawnBurst(origin, 50, 2.0f, 10.0f, color, 0.1f, 0.5f);
                    break;
                case FiringMode::BEAM:
                    for (int i = 0; i < 10; i++) {
                        glm::vec3 pos = origin + direction * static_cast<float>(i) * 0.5f;
                        m_particleSystem->spawnParticle(pos, direction * 20.0f, color, 0.05f, 0.3f);
                    }
                    break;
                default:
                    m_particleSystem->spawnBurst(origin, 20, 1.0f, 5.0f, color, 0.1f, 0.5f);
                    break;
            }
        }
        
        m_currentCharge = 0.0f;
        m_isCharging = false;
        m_cooldown = PlasmaWeaponConstants::COOLDOWN_TIME;
        std::cout << "Charge reset to " << m_currentCharge << ", cooldown set to " << m_cooldown << "\n";
        return true;
    }

    bool quickFireWithFlammil(const glm::vec3& origin, const glm::vec3& direction) {
        if (m_cooldown > 0.0f) {
            std::cout << "Cannot quick-fire, weapon is cooling down\n";
            return false;
        }

        std::cout << "Quick-fired with Flammil gesture\n";
        
        if (m_particleSystem) {
            glm::vec4 color(1.0f, 0.3f, 0.1f, 1.0f); // Orange-red color for quick fire
            m_particleSystem->spawnBurst(origin, 15, 5.0f, 15.0f, color, 0.08f, 0.2f);
        }
        
        m_cooldown = PlasmaWeaponConstants::COOLDOWN_TIME * 0.5f;
        return true;
    }

    void setFiringMode(FiringMode mode) {
        m_firingMode = mode;
        std::cout << "Firing mode set to: " << static_cast<int>(mode) << "\n";
    }

    float getCurrentCharge() const { return m_currentCharge; }
    float getCurrentCooldown() const { return m_cooldown; }

private:
    float m_currentCharge;
    bool m_isCharging;
    float m_cooldown;
    FiringMode m_firingMode;
    std::shared_ptr<ParticleSystem> m_particleSystem;
};

} // namespace Combat
} // namespace TurtleEngine

int main() {
    std::cout << "=== PlasmaWeapon Test ===\n";

    // Create a particle system
    auto particleSystem = std::make_shared<TurtleEngine::Combat::ParticleSystem>();
    
    // Create a PlasmaWeapon instance
    TurtleEngine::Combat::PlasmaWeapon weapon(particleSystem);

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
    // Wait for cooldown to expire
    while (weapon.getCurrentCooldown() > 0.0f) {
        weapon.update(0.1f);
    }
    
    bool quickFireSuccess = weapon.quickFireWithFlammil(glm::vec3(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    bool quickFireResult = quickFireSuccess && (weapon.getCurrentCooldown() > 0.0f);
    std::cout << "Quick-Fire Test: " << (quickFireResult ? "PASSED" : "FAILED") 
              << " (Cooldown: " << weapon.getCurrentCooldown() << "s)\n";

    std::cout << "\n=== PlasmaWeapon Test Complete ===\n";
    return (fireResult && quickFireResult) ? 0 : 1;
}
