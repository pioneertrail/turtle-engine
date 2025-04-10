#include "Engine.hpp"
#include <iostream>

int main() {
    TurtleEngine::Engine engine;
    // Note: For a logic-only test, we might avoid the full GL init
    // but for now, let's use the standard init to ensure systems are ready.
    if (!engine.initialize("Test Window", 100, 100)) { // Basic init
        std::cerr << "TEST_FAILED: EngineInit" << std::endl;
        return 1;
    }
    
    // Give CSL a moment if needed (might not be necessary without real input)
    engine.update(0.01f); 

    engine.simulateKeyPress('F'); // Trigger FLAMMIL
    
    // Update engine logic (particles, combos etc.)
    engine.update(0.016f); // Simulate one frame update
    
    // Check particle count *after* update
    if (engine.getParticleSystem().getActiveParticleCount() > 0) {
        std::cout << "TEST_PASSED: ParticleSpawn" << std::endl;
    } else {
        std::cerr << "TEST_FAILED: ParticleSpawn - Count: " << engine.getParticleSystem().getActiveParticleCount() << std::endl;
        return 1;
    }
    
    engine.shutdown(); // Clean shutdown
    return 0;
} 