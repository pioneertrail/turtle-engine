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
        ParticleSystem(size_t maxParticles = 10000);
        virtual ~ParticleSystem();
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life);
        
        virtual void initialize();
        virtual void update(float deltaTime);
        virtual void render();
        
        virtual void spawnBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life);
        
        virtual void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                 const glm::vec4& color, float size, float life);
    };
    
    // Also provide a mock for the Graphics namespace version used in some components
    namespace Graphics {
        class ParticleSystem {
        public:
            ParticleSystem(size_t maxParticles = 10000);
            virtual ~ParticleSystem();
            
            virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                            const glm::vec4& color, float size, float life);
            
            virtual void initialize();
            virtual void update(float deltaTime);
            virtual void render();
            
            virtual void emitBurst(const glm::vec3& position, size_t count,
                                float minVel, float maxVel,
                                const glm::vec4& color, float size, float life);
        };
    }
}

#endif
