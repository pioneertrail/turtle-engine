// Mock implementation of ParticleSystem methods
#include "ParticleSystem.hpp"

namespace TurtleEngine {
    // Base ParticleSystem implementation
    ParticleSystem::ParticleSystem(size_t maxParticles) {
        std::cout << "[MockParticleSystem] Created with maxParticles: " << maxParticles << std::endl;
    }
    
    ParticleSystem::~ParticleSystem() {
        std::cout << "[MockParticleSystem] Destroyed" << std::endl;
    }
    
    void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity,
                             const glm::vec4& color, float size, float life) {
        std::cout << "[MockParticleSystem] Emitted particle at position: (" 
                 << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
    
    void ParticleSystem::initialize() {
        std::cout << "[MockParticleSystem] Initialized" << std::endl;
    }
    
    void ParticleSystem::update(float deltaTime) {
        // No output to avoid spam
    }
    
    void ParticleSystem::render() {
        // No output to avoid spam
    }
    
    void ParticleSystem::spawnBurst(const glm::vec3& position, size_t count,
                                  float minVel, float maxVel,
                                  const glm::vec4& color, float size, float life) {
        std::cout << "[MockParticleSystem] Spawned burst of " << count << " particles" << std::endl;
    }
    
    void ParticleSystem::spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                     const glm::vec4& color, float size, float life) {
        std::cout << "[MockParticleSystem] Spawned single particle" << std::endl;
    }
    
    // Graphics namespace implementation
    namespace Graphics {
        ParticleSystem::ParticleSystem(size_t maxParticles) {
            std::cout << "[MockGraphicsParticleSystem] Created with maxParticles: " << maxParticles << std::endl;
        }
        
        ParticleSystem::~ParticleSystem() {
            std::cout << "[MockGraphicsParticleSystem] Destroyed" << std::endl;
        }
        
        void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity,
                                const glm::vec4& color, float size, float life) {
            std::cout << "[MockGraphicsParticleSystem] Emitted particle at position: (" 
                     << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
        
        void ParticleSystem::initialize() {
            std::cout << "[MockGraphicsParticleSystem] Initialized" << std::endl;
        }
        
        void ParticleSystem::update(float deltaTime) {
            // No output to avoid spam
        }
        
        void ParticleSystem::render() {
            // No output to avoid spam
        }
        
        void ParticleSystem::emitBurst(const glm::vec3& position, size_t count,
                                    float minVel, float maxVel,
                                    const glm::vec4& color, float size, float life) {
            std::cout << "[MockGraphicsParticleSystem] Emitted burst of " << count << " particles" << std::endl;
        }
    }
}
