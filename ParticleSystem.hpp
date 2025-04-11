// Mock implementation of ParticleSystem for testing
#pragma once
#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

namespace TurtleEngine {
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted particle at (" 
                     << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
        
        virtual void initialize() {}
        virtual void update(float deltaTime) {}
        virtual void render() {}
        
        virtual void spawnBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Emitted burst of " << count << " particles." << std::endl;
        }
        
        virtual void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                 const glm::vec4& color, float size, float life) {
            std::cout << "[MockParticleSystem] Spawned a particle." << std::endl;
        }
    };
}

#endif
