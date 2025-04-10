#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/random.hpp>
#include <vector>
#include <memory>
#include <chrono>
#include "Shader.hpp" // Include shader class header

namespace TurtleEngine {

    // Represents a single particle
    struct Particle {
        glm::vec3 position{0.0f};
        glm::vec3 velocity{0.0f};
        glm::vec4 color{1.0f, 1.0f, 0.0f, 1.0f}; // Default yellow spark color
        float life = 0.0f; // Remaining life time in seconds

        Particle() = default; // Needed for vector resizing
        Particle(glm::vec3 pos, glm::vec3 vel, glm::vec4 col, float l) 
            : position(pos), velocity(vel), color(col), life(l) {}
    };

    // Manages simulation, buffering, and rendering of particles
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 1000);
        ~ParticleSystem();

        // Load shaders and initialize buffers
        bool initialize(const std::string& vertexShaderPath = "shaders/particle.vert", 
                        const std::string& fragmentShaderPath = "shaders/particle.frag");

        // Spawn a single particle
        void spawnParticle(const Particle& particleProperties);
        
        // Spawn multiple particles (e.g., for an explosion)
        void spawnBurst(int count, glm::vec3 origin, float initialSpeed, float lifetime, glm::vec4 color);

        // Update particle positions, life, etc.
        void update(float deltaTime);

        // Render active particles
        bool render(const glm::mat4& view, const glm::mat4& projection);

    private:
        void createBuffers();
        void updateBuffers(); // Update VBO with active particle data
        int findUnusedParticle(); // Find an inactive particle index

        size_t m_maxParticles;
        std::vector<Particle> m_particles; // Pool of all particles
        std::vector<float> m_particleBufferData; // Data to upload to VBO (pos + color)
        size_t m_lastUsedParticle = 0; // Optimization for finding unused particles
        size_t m_activeParticleCount = 0; // Count of particles active after last update

        // Rendering resources
        Shader m_shader;
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        bool m_initialized = false;
    };

} // namespace TurtleEngine 