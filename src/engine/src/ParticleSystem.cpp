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
    // Allocate buffer memory (dynamic draw because updated frequently)
    glBufferData(GL_ARRAY_BUFFER, m_maxParticles * 8 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);

    // Define vertex attributes
    size_t stride = 8 * sizeof(float); // 3 pos + 4 color + 1 life
    
    // Position attribute (location = 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);

    // Color attribute (location = 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    
    // Life Ratio attribute (location = 2) - Assuming we use location 2
    glEnableVertexAttribArray(2); 
    glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, stride, (void*)(7 * sizeof(float))); // Offset = 3 pos + 4 color

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
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
        // Use the provided properties, ensuring initialLife is set
        m_particles[particleIndex] = particleProperties; 
        // Ensure life is also set if not already in properties
        m_particles[particleIndex].life = particleProperties.initialLife; 
    }
}

void ParticleSystem::spawnBurst(int count, glm::vec3 origin, float initialSpeed, float lifetime, glm::vec4 color) {
    std::cout << "  [ParticleSystem SpawnBurst] Count: " << count << ", Origin: (" 
              << origin.x << "," << origin.y << "," << origin.z << "), Speed: " 
              << initialSpeed << ", Lifetime: " << lifetime << std::endl;

    for (int i = 0; i < count; ++i) {
        int particleIndex = findUnusedParticle();
        if (particleIndex != -1) {
            m_particles[particleIndex].position = origin;
            // Add random velocity variation
            glm::vec3 randomDir = glm::sphericalRand(1.0f);
            m_particles[particleIndex].velocity = randomDir * initialSpeed * (0.5f + (static_cast<float>(rand()) / RAND_MAX) * 0.5f); // Random speed variation
            m_particles[particleIndex].color = color;
            // Add random lifetime variation
            m_particles[particleIndex].life = lifetime * (0.8f + (static_cast<float>(rand()) / RAND_MAX) * 0.4f); 
            m_particles[particleIndex].initialLife = m_particles[particleIndex].life; // Store the randomized life as initial
        }
    }
}

void ParticleSystem::update(float deltaTime) {
    // logToFile(std::string("[ParticleSystem Update] Received deltaTime: ") + std::to_string(deltaTime)); // Removed deltaTime log

    m_particleBufferData.clear();
    m_activeParticleCount = 0; 

    for (size_t i = 0; i < m_maxParticles; ++i) {
        Particle& p = m_particles[i];

        if (p.life > 0.0f) {
            p.life -= deltaTime;

            if (p.life > 0.0f) {
                // Update position (simple Euler integration)
                p.position += p.velocity * deltaTime;
                // Optional: Add gravity or other forces
                // p.velocity.y -= 9.81f * deltaTime;

                // Add particle data to buffer
                m_particleBufferData.push_back(p.position.x);
                m_particleBufferData.push_back(p.position.y);
                m_particleBufferData.push_back(p.position.z);
                m_particleBufferData.push_back(p.color.r);
                m_particleBufferData.push_back(p.color.g);
                m_particleBufferData.push_back(p.color.b);
                m_particleBufferData.push_back(p.color.a);

                // Calculate and add normalized life ratio
                float lifeRatio = 0.0f;
                if (p.initialLife > 0.0f) { // Avoid division by zero
                    lifeRatio = glm::clamp(p.life / p.initialLife, 0.0f, 1.0f);
                }
                m_particleBufferData.push_back(lifeRatio);

                m_activeParticleCount++; // Increment count for active/rendered particles
            } else {
                // Particle just died
                // Optionally trigger something here
            }
        }
    }

    // logToFile(std::string("[ParticleSystem Update] Active particles after loop: ") + std::to_string(m_activeParticleCount)); // Removed active count log

    // Update VBO if there are active particles
    if (m_activeParticleCount > 0) {
        updateBuffers();
    }
}

void ParticleSystem::updateBuffers() {
    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    // Upload only the data for active particles
    // Use glBufferSubData to avoid reallocating the entire buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, m_particleBufferData.size() * sizeof(float), m_particleBufferData.data());
    glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind
}

void ParticleSystem::render(const glm::mat4& projection, const glm::mat4& view) {
    // logToFile(std::string("[Particle Render] Entered. Initialized: ") + // Removed entry log
    //           std::to_string(m_isInitialized) + ", Active Particles: " + std::to_string(m_activeParticleCount));

    if (!m_initialized || m_particleBufferData.empty()) return;

    // Enable additive blending for sparks (optional)
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // Disable depth writing so particles dont obscure each other incorrectly
    // glDepthMask(GL_FALSE); 

    m_shader.use();
    m_shader.setMat4("view", view);
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("model", glm::mat4(1.0f)); // Use identity model matrix for world-space particles

    glBindVertexArray(m_VAO);
    // logToFile(std::string("[Particle Render] Drawing Arrays: mode=GL_POINTS, first=0, count=") + // Removed draw log
    //          std::to_string(m_activeParticleCount));
    glDrawArrays(GL_POINTS, 0, m_activeParticleCount); 
    glBindVertexArray(0);

    // Restore blend/depth state if changed
    // glDepthMask(GL_TRUE);
    // glDisable(GL_BLEND);

    // Check shader program validity after use()
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    // logToFile(std::string("[Particle Render] Shader ID: ") + std::to_string(currentProgram)); // Removed shader ID log

    if (currentProgram == 0) {
        std::cerr << "ERROR::ParticleSystem: Shader program is not valid" << std::endl;
    }
}

} // namespace TurtleEngine 