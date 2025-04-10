#include "ParticleSystem.hpp"
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp> // For value_ptr
#include <iostream> // For errors
#include <algorithm> // For std::min

namespace TurtleEngine {

ParticleSystem::ParticleSystem(size_t maxParticles)
    : m_maxParticles(maxParticles) {
    m_particles.resize(m_maxParticles);
    // Pre-allocate buffer data storage to avoid reallocations
    // Size = maxParticles * (vec3 position + vec4 color) = maxParticles * 7 floats
    m_particleBufferData.reserve(m_maxParticles * 7);
}

ParticleSystem::~ParticleSystem() {
    if (m_initialized) {
        glDeleteVertexArrays(1, &m_VAO);
        glDeleteBuffers(1, &m_VBO);
    }
}

bool ParticleSystem::initialize(const std::string& vertexShaderPath, const std::string& fragmentShaderPath) {
    if (m_initialized) return true;

    if (!m_shader.loadFromFiles(vertexShaderPath, fragmentShaderPath)) {
        std::cerr << "ERROR::ParticleSystem: Failed to load shaders (" 
                  << vertexShaderPath << ", " << fragmentShaderPath << ")" << std::endl;
        return false;
    }

    createBuffers();
    m_initialized = true;
    return true;
}

void ParticleSystem::createBuffers() {
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);

    glBindVertexArray(m_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    // Allocate buffer space for maximum particles
    // Data will be uploaded dynamically via glBufferSubData
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 7 * sizeof(float), NULL, GL_DYNAMIC_DRAW);

    // Vertex Attributes
    // Position (vec3)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)0);

    // Color (vec4)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));

    glBindVertexArray(0); // Unbind VAO
}

int ParticleSystem::findUnusedParticle() {
    // Search from last used position
    for (size_t i = m_lastUsedParticle; i < m_maxParticles; ++i) {
        if (m_particles[i].life <= 0.0f) {
            m_lastUsedParticle = i;
            return i;
        }
    }
    // Search from the beginning if not found
    for (size_t i = 0; i < m_lastUsedParticle; ++i) {
        if (m_particles[i].life <= 0.0f) {
            m_lastUsedParticle = i;
            return i;
        }
    }
    return -1; // All particles are active
}

void ParticleSystem::spawnParticle(const Particle& particleProperties) {
    int particleIndex = findUnusedParticle();
    if (particleIndex != -1) {
        m_particles[particleIndex] = particleProperties; 
    }
}

void ParticleSystem::spawnBurst(int count, glm::vec3 origin, float initialSpeed, float lifetime, glm::vec4 color) {
     for(int i = 0; i < count; ++i) {
        int particleIndex = findUnusedParticle();
        if (particleIndex == -1) return; // No more free particles

        m_particles[particleIndex].position = origin;
        // Simple random direction (sphere)
        m_particles[particleIndex].velocity = glm::sphericalRand(initialSpeed); 
        m_particles[particleIndex].color = color;
        m_particles[particleIndex].life = lifetime; 
     }
}

void ParticleSystem::update(float deltaTime) {
    if (!m_initialized) return;

    // Log received deltaTime
    std::cout << "    [ParticleSystem Update] Received deltaTime: " << deltaTime << std::endl;

    m_particleBufferData.clear(); // Clear buffer for refill
    m_activeParticleCount = 0; // Reset member count for this frame

    for (size_t i = 0; i < m_maxParticles; ++i) {
        Particle& p = m_particles[i];

        if (p.life > 0.0f) {
            // Decrease life
            p.life -= deltaTime;

            if (p.life > 0.0f) {
                // Simulate physics (simple Euler integration)
                p.velocity += glm::vec3(0.0f, -9.81f, 0.0f) * deltaTime * 0.5f; // Simple gravity
                p.position += p.velocity * deltaTime;

                // Update color alpha based on life (simple fade)
                float lifeRatio = std::max(0.0f, p.life / 1.0f); // Assuming max life of 1 for fade base
                p.color.a = lifeRatio;

                // Add particle data to buffer
                m_particleBufferData.push_back(p.position.x);
                m_particleBufferData.push_back(p.position.y);
                m_particleBufferData.push_back(p.position.z);
                m_particleBufferData.push_back(p.color.r);
                m_particleBufferData.push_back(p.color.g);
                m_particleBufferData.push_back(p.color.b);
                m_particleBufferData.push_back(p.color.a);
                m_activeParticleCount++; // Increment member count
            }
        }
    }
    // Update the VBO if there are active particles
    if (m_activeParticleCount > 0) {
        // Log buffer state before updating VBO
        std::cout << "    [ParticleSystem Update] Active particles found: " << m_activeParticleCount << ". Buffer size (floats): " << m_particleBufferData.size() << std::endl;
        updateBuffers();
    } else {
        // Log if no active particles found
        std::cout << "    [ParticleSystem Update] No active particles found after update loop." << std::endl;
    }
}

void ParticleSystem::updateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // Upload only the data for active particles
    // Use glBufferSubData to avoid reallocating the entire buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particleBufferData.size() * sizeof(float), m_particleBufferData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind
}

void ParticleSystem::render(const glm::mat4& view, const glm::mat4& projection) {
    // Entry Log & State Check
    std::cout << "  [Particle Render] Entered. Initialized: " << m_initialized << ", Active Particles: " << m_activeParticleCount << std::endl;

    if (!m_initialized || m_activeParticleCount == 0) {
        std::cout << "  [Particle Render] Exiting early (Not initialized or no active particles)." << std::endl;
        return;
    }

    // Enable additive blending for sparks (optional)
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // Disable depth writing so particles dont obscure each other incorrectly
    // glDepthMask(GL_FALSE); 

    m_shader.use();
    // Debug Log
    GLint particleProgram = 0; glGetIntegerv(GL_CURRENT_PROGRAM, &particleProgram);
    std::cout << "  [Particle Render] Shader ID: " << particleProgram << ", Valid: " << m_shader.isValid() << std::endl;

    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("model", glm::mat4(1.0f)); // Use identity model matrix for world-space particles

    glBindVertexArray(m_VAO);
    // Draw only the number of active particles currently in the buffer
    glDrawArrays(GL_POINTS, 0, m_activeParticleCount); // Draw based on count from update
    glBindVertexArray(0);

    // Restore blend/depth state if changed
    // glDepthMask(GL_TRUE);
    // glDisable(GL_BLEND);
}

} // namespace TurtleEngine 