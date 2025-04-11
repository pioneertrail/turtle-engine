#include "engine/include/Engine.hpp"
#include "engine/combat/include/PlasmaWeapon.hpp"
#include "engine/combat/include/HealthSystem.hpp"
#include "engine/combat/include/AIConstruct.hpp"
#include "engine/temporal/include/TemporalAnomalySystem.hpp"
#include "engine/input/include/GestureRecognizer.hpp"
#include <iostream>
#include <memory>
#include <thread>
#include <chrono>

using namespace TurtleEngine;

class SilentForgeDemo {
private:
    std::shared_ptr<Combat::PlasmaWeapon> m_plasmaWeapon;
    std::shared_ptr<Combat::HealthSystem> m_healthSystem;
    std::shared_ptr<Combat::AIConstruct> m_aiConstruct;
    std::shared_ptr<Temporal::TemporalAnomalySystem> m_temporalSystem;
    std::shared_ptr<Input::GestureRecognizer> m_gestureRecognizer;
    std::shared_ptr<Graphics::ParticleSystem> m_particleSystem;
    
    bool m_running;
    
    // Mock player position
    glm::vec3 m_playerPosition;
    
    // Test entities
    Combat::Entity m_playerEntity;
    Combat::Entity m_enemyEntity;
    
public:
    SilentForgeDemo() : m_running(true), m_playerPosition(0.0f, 0.0f, 0.0f) {
        // Initialize systems
        m_particleSystem = std::make_shared<Graphics::ParticleSystem>();
        m_plasmaWeapon = std::make_shared<Combat::PlasmaWeapon>(m_particleSystem);
        m_healthSystem = std::make_shared<Combat::HealthSystem>(m_particleSystem);
        m_aiConstruct = std::make_shared<Combat::AIConstruct>(Combat::AIConstructType::GUARDIAN);
        m_temporalSystem = std::make_shared<Temporal::TemporalAnomalySystem>();
        m_gestureRecognizer = std::make_shared<Input::GestureRecognizer>();
        
        // Setup player and enemy entities
        m_playerEntity = m_healthSystem->createEntity(100.0f);
        m_enemyEntity = m_healthSystem->createEntity(150.0f);
        
        // Set up gesture callbacks
        m_gestureRecognizer->registerCallback(Input::GestureType::FLAMMIL, 
            [this]() { this->handleFlammilGesture(); });
        m_gestureRecognizer->registerCallback(Input::GestureType::AERETH, 
            [this]() { this->handleAerethGesture(); });
        m_gestureRecognizer->registerCallback(Input::GestureType::TURANIS, 
            [this]() { this->handleTuranisGesture(); });
    }
    
    void run() {
        std::cout << "=== Silent Forge: Phase 1 Demo ===" << std::endl;
        std::cout << "Demonstrating all five Phase 1 components working together" << std::endl;
        
        float deltaTime = 0.1f;
        int step = 1;
        
        while (m_running && step <= 12) {
            std::cout << "\n--- DEMO STEP " << step << " ---" << std::endl;
            
            switch (step) {
                case 1:
                    demoPlasmaWeapon();
                    break;
                case 2:
                    demoHealthSystem();
                    break;
                case 3:
                    demoAIConstruct();
                    break;
                case 4:
                    demoTemporalAnomaly();
                    break;
                case 5:
                    demoGestureRecognition();
                    break;
                case 6:
                    demoCombatIntegration();
                    break;
                case 7:
                    demoAIAndTemporalInteraction();
                    break;
                case 8:
                    demoAdvancedGestureCombat();
                    break;
                case 9:
                    demoTemporalHealthInteraction();
                    break;
                case 10:
                    demoAIDamagedState();
                    break;
                case 11:
                    demoFullSystemIntegration();
                    break;
                case 12:
                    std::cout << "\n=== Demo Complete ===" << std::endl;
                    m_running = false;
                    break;
            }
            
            // Update all systems
            m_plasmaWeapon->update(deltaTime);
            m_aiConstruct->update(deltaTime);
            m_temporalSystem->update(deltaTime);
            m_healthSystem->update(deltaTime);
            
            step++;
            
            // Pause between steps
            std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        }
    }
    
private:
    void demoPlasmaWeapon() {
        std::cout << "Demonstrating PlasmaWeapon system" << std::endl;
        
        // Test different firing modes
        m_plasmaWeapon->setFiringMode(Combat::FiringMode::BURST);
        m_plasmaWeapon->startCharging();
        
        // Simulate charging for 1 second
        for (int i = 0; i < 10; i++) {
            m_plasmaWeapon->update(0.1f);
        }
        
        // Fire the weapon
        m_plasmaWeapon->fire(m_playerPosition, glm::vec3(1.0f, 0.0f, 0.0f));
        
        // Switch to BEAM mode
        m_plasmaWeapon->setFiringMode(Combat::FiringMode::BEAM);
        m_plasmaWeapon->startCharging();
        
        // Simulate charging for 1.5 seconds
        for (int i = 0; i < 15; i++) {
            m_plasmaWeapon->update(0.1f);
        }
        
        // Fire the beam
        m_plasmaWeapon->fire(m_playerPosition, glm::vec3(1.0f, 0.0f, 0.0f));
    }
    
    void demoHealthSystem() {
        std::cout << "Demonstrating HealthSystem" << std::endl;
        
        // Show initial health
        std::cout << "Player health: " << m_healthSystem->getHealth(m_playerEntity) << std::endl;
        std::cout << "Enemy health: " << m_healthSystem->getHealth(m_enemyEntity) << std::endl;
        
        // Apply different damage types
        m_healthSystem->applyDamage(m_enemyEntity, 25.0f, Combat::DamageType::ENERGY);
        std::cout << "After energy damage, enemy health: " << m_healthSystem->getHealth(m_enemyEntity) << std::endl;
        
        // Apply critical hit
        bool criticalHit = m_healthSystem->applyDamage(m_enemyEntity, 30.0f, Combat::DamageType::PHYSICAL, true);
        std::cout << "Critical hit: " << (criticalHit ? "Yes" : "No") << std::endl;
        std::cout << "After critical physical damage, enemy health: " << m_healthSystem->getHealth(m_enemyEntity) << std::endl;
    }
    
    void demoAIConstruct() {
        std::cout << "Demonstrating AIConstruct behavior" << std::endl;
        
        // Show initial state
        std::cout << "AI initial state: " << m_aiConstruct->getStateString() << std::endl;
        
        // Simulate player entering detection range
        m_aiConstruct->detectPlayer(m_playerPosition);
        std::cout << "After player detected, AI state: " << m_aiConstruct->getStateString() << std::endl;
        
        // Simulate player attacking the AI
        m_aiConstruct->takeDamage(20.0f);
        std::cout << "After taking damage, AI state: " << m_aiConstruct->getStateString() << std::endl;
    }
    
    void demoTemporalAnomaly() {
        std::cout << "Demonstrating TemporalAnomalySystem" << std::endl;
        
        // Create a SLOWFIELD anomaly
        glm::vec3 anomalyPosition(5.0f, 0.0f, 0.0f);
        m_temporalSystem->createAnomaly(anomalyPosition, 3.0f, Temporal::AnomalyType::SLOWFIELD);
        std::cout << "Created SLOWFIELD anomaly at (5,0,0)" << std::endl;
        
        // Check if entity is affected
        glm::vec3 entityPos(4.0f, 0.0f, 0.0f);
        float timeScale = m_temporalSystem->getTimeScaleAtPosition(entityPos);
        std::cout << "Entity at (4,0,0) has time scale: " << timeScale << std::endl;
        
        // Move entity out of range
        entityPos = glm::vec3(10.0f, 0.0f, 0.0f);
        timeScale = m_temporalSystem->getTimeScaleAtPosition(entityPos);
        std::cout << "Entity at (10,0,0) has time scale: " << timeScale << std::endl;
        
        // Create a RIFT anomaly
        m_temporalSystem->createAnomaly(glm::vec3(-5.0f, 0.0f, 0.0f), 2.0f, Temporal::AnomalyType::RIFT);
        std::cout << "Created RIFT anomaly at (-5,0,0)" << std::endl;
    }
    
    void demoGestureRecognition() {
        std::cout << "Demonstrating GestureRecognizer" << std::endl;
        
        // Simulate Flammil gesture (forward thrust)
        std::vector<Input::GesturePoint> flammilPoints = {
            {0.5f, 0.5f, 0.0f},
            {0.6f, 0.5f, 0.1f},
            {0.7f, 0.5f, 0.2f},
            {0.8f, 0.5f, 0.3f},
            {0.9f, 0.5f, 0.4f}
        };
        
        bool recognized = m_gestureRecognizer->recognizeGesture(flammilPoints);
        std::cout << "Flammil gesture recognized: " << (recognized ? "Yes" : "No") << std::endl;
        
        // Simulate Aereth gesture (circular motion)
        std::vector<Input::GesturePoint> aerethPoints = {
            {0.5f, 0.5f, 0.0f},
            {0.6f, 0.4f, 0.1f},
            {0.7f, 0.5f, 0.2f},
            {0.6f, 0.6f, 0.3f},
            {0.5f, 0.5f, 0.4f}
        };
        
        recognized = m_gestureRecognizer->recognizeGesture(aerethPoints);
        std::cout << "Aereth gesture recognized: " << (recognized ? "Yes" : "No") << std::endl;
    }
    
    void demoCombatIntegration() {
        std::cout << "Demonstrating Combat System Integration" << std::endl;
        
        // Charge plasma weapon
        m_plasmaWeapon->startCharging();
        for (int i = 0; i < 10; i++) {
            m_plasmaWeapon->update(0.1f);
        }
        
        // Fire at enemy and apply damage
        if (m_plasmaWeapon->fire(m_playerPosition, glm::vec3(1.0f, 0.0f, 0.0f))) {
            m_healthSystem->applyDamage(m_enemyEntity, 40.0f, Combat::DamageType::ENERGY);
        }
        
        std::cout << "Enemy health after plasma attack: " << m_healthSystem->getHealth(m_enemyEntity) << std::endl;
    }
    
    void demoAIAndTemporalInteraction() {
        std::cout << "Demonstrating AI and Temporal System Interaction" << std::endl;
        
        // Create temporal anomaly near AI
        glm::vec3 aiPosition = m_aiConstruct->getPosition();
        m_temporalSystem->createAnomaly(aiPosition + glm::vec3(1.0f, 0.0f, 0.0f), 2.0f, Temporal::AnomalyType::SLOWFIELD);
        
        // Update AI under temporal effect
        float timeScale = m_temporalSystem->getTimeScaleAtPosition(aiPosition);
        std::cout << "AI is affected by time scale: " << timeScale << std::endl;
        
        // AI behavior will be slower in the anomaly
        m_aiConstruct->update(0.1f * timeScale);
        std::cout << "AI updated with modified time scale, state: " << m_aiConstruct->getStateString() << std::endl;
    }
    
    void demoAdvancedGestureCombat() {
        std::cout << "Demonstrating Advanced Gesture and Combat Integration" << std::endl;
        
        // Simulate Flammil gesture for quick-fire
        std::vector<Input::GesturePoint> flammilPoints = {
            {0.5f, 0.5f, 0.0f},
            {0.6f, 0.5f, 0.1f},
            {0.7f, 0.5f, 0.2f},
            {0.8f, 0.5f, 0.3f},
            {0.9f, 0.5f, 0.4f}
        };
        
        if (m_gestureRecognizer->recognizeGesture(flammilPoints)) {
            handleFlammilGesture();
        }
    }
    
    void demoTemporalHealthInteraction() {
        std::cout << "Demonstrating Temporal Effects on Health System" << std::endl;
        
        // Apply poison damage that should be affected by time
        m_healthSystem->applyDamageOverTime(m_enemyEntity, 5.0f, Combat::DamageType::TEMPORAL, 3.0f);
        
        // Get time scale at enemy position and modify damage effect
        float timeScale = m_temporalSystem->getTimeScaleAtPosition(glm::vec3(0.0f, 0.0f, 0.0f));
        m_healthSystem->update(0.1f * timeScale);
        
        std::cout << "Enemy health after temporal damage with time scale " << timeScale << ": " 
                  << m_healthSystem->getHealth(m_enemyEntity) << std::endl;
    }
    
    void demoAIDamagedState() {
        std::cout << "Demonstrating AI Damaged State and Behavior Change" << std::endl;
        
        // Deal significant damage to AI
        m_aiConstruct->takeDamage(50.0f);
        std::cout << "AI heavily damaged, state: " << m_aiConstruct->getStateString() << std::endl;
        
        // Update AI behavior
        m_aiConstruct->update(0.1f);
        std::cout << "AI updated after taking damage, state: " << m_aiConstruct->getStateString() << std::endl;
        
        // Check if AI switches to FLEE state
        if (m_aiConstruct->getHealthPercentage() < 30.0f) {
            m_aiConstruct->setState(Combat::AIState::FLEE);
            std::cout << "AI health critical, switching to FLEE state" << std::endl;
        }
    }
    
    void demoFullSystemIntegration() {
        std::cout << "Demonstrating Full System Integration" << std::endl;
        
        // Create a complex combat scenario
        std::cout << "Scenario: Player encounters an enemy construct near a temporal anomaly" << std::endl;
        
        // Place AI and temporal anomaly
        glm::vec3 enemyPos(3.0f, 0.0f, 0.0f);
        m_aiConstruct->setPosition(enemyPos);
        m_temporalSystem->createAnomaly(glm::vec3(2.0f, 0.0f, 0.0f), 2.5f, Temporal::AnomalyType::SLOWFIELD);
        
        // AI detects player
        m_aiConstruct->detectPlayer(m_playerPosition);
        std::cout << "AI detected player, state: " << m_aiConstruct->getStateString() << std::endl;
        
        // Player performs Flammil gesture for quick attack
        handleFlammilGesture();
        
        // AI enters anomaly and is slowed
        m_aiConstruct->setPosition(glm::vec3(2.0f, 0.5f, 0.0f));
        float timeScale = m_temporalSystem->getTimeScaleAtPosition(m_aiConstruct->getPosition());
        std::cout << "AI entered anomaly, time scale: " << timeScale << std::endl;
        
        // Player performs Aereth gesture for area effect
        handleAerethGesture();
        
        // Update all systems
        m_aiConstruct->update(0.1f * timeScale);
        m_healthSystem->update(0.1f);
        
        std::cout << "After combat, enemy health: " << m_healthSystem->getHealth(m_enemyEntity) << std::endl;
        std::cout << "Final AI state: " << m_aiConstruct->getStateString() << std::endl;
    }
    
    // Gesture handlers
    void handleFlammilGesture() {
        std::cout << "Flammil gesture detected - Quick plasma burst!" << std::endl;
        m_plasmaWeapon->quickFireWithFlammil(m_playerPosition, glm::vec3(1.0f, 0.0f, 0.0f));
        m_healthSystem->applyDamage(m_enemyEntity, 20.0f, Combat::DamageType::ENERGY);
    }
    
    void handleAerethGesture() {
        std::cout << "Aereth gesture detected - Temporal distortion!" << std::endl;
        m_temporalSystem->createAnomaly(m_playerPosition + glm::vec3(2.0f, 0.0f, 0.0f), 
                                        1.5f, Temporal::AnomalyType::SLOWFIELD);
    }
    
    void handleTuranisGesture() {
        std::cout << "Turanis gesture detected - Energy surge!" << std::endl;
        m_plasmaWeapon->setFiringMode(Combat::FiringMode::BEAM);
        m_plasmaWeapon->startCharging();
        
        // Instant full charge due to Turanis gesture
        for (int i = 0; i < 20; i++) {
            m_plasmaWeapon->update(0.1f);
        }
        
        m_plasmaWeapon->fire(m_playerPosition, glm::vec3(1.0f, 0.0f, 0.0f));
        m_healthSystem->applyDamage(m_enemyEntity, 50.0f, Combat::DamageType::ENERGY, true);
    }
};

int main() {
    // Initialize the engine
    TurtleEngine::Engine engine;
    if (!engine.initialize("Silent Forge: Phase 1 Demo")) {
        std::cerr << "Failed to initialize engine" << std::endl;
        return -1;
    }
    
    // Create and run the demo
    SilentForgeDemo demo;
    demo.run();
    
    return 0;
} 