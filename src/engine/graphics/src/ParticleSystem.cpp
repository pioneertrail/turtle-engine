#include "../include/ParticleSystem.hpp"
#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <algorithm>
#include <random>

namespace TurtleEngine {
namespace Graphics {

ParticleSystem::ParticleSystem(size_t maxParticles)
    : maxParticles_(maxParticles)
    , activeParticles_(0)
    , vao_(0)
    , vbo_(0)
    , instanceVBO_(0)
    , texture_(0)
    , useInstancing_(true)
    , batchSize_(1000)
    , highContrastMode_(false)
    , depthTestEnabled_(true)
    , additiveBlending_(true)
    , minParticleSize_(1.0f)
    , maxParticleSize_(10.0f)
    , debugViewEnabled_(false)
    , debugColor_(1.0f, 0.0f, 0.0f, 1.0f)
{
    particles_.resize(maxParticles_);
    std::fill(particles_.begin(), particles_.end(), Particle());
}

ParticleSystem::~ParticleSystem() {
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (instanceVBO_) glDeleteBuffers(1, &instanceVBO_);
    if (texture_) glDeleteTextures(1, &texture_);
}

void ParticleSystem::initialize() {
    initializeQuadBuffers();
}

void ParticleSystem::initializeQuadBuffers() {
    // Quad vertices (centered at origin)
    float quadVertices[] = {
        // positions        // texture coords
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
         0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
         0.5f,  0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f,  0.5f, 0.0f, 0.0f, 1.0f
    };

    // Quad indices
    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create and bind VAO
    glGenVertexArrays(1, &vao_);
    glBindVertexArray(vao_);

    // Create and bind VBO for quad vertices
    glGenBuffers(1, &vbo_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    // Position attribute
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

    // Texture coordinate attribute
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

    // Create and bind instance VBO
    glGenBuffers(1, &instanceVBO_);
    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferData(GL_ARRAY_BUFFER, maxParticles_ * sizeof(float) * 16, nullptr, GL_DYNAMIC_DRAW);

    // Instance model matrix (4x4)
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(2 + i);
        glVertexAttribPointer(2 + i, 4, GL_FLOAT, GL_FALSE, sizeof(float) * 16, (void*)(sizeof(float) * 4 * i));
        glVertexAttribDivisor(2 + i, 1);
    }

    // Create and bind EBO
    unsigned int ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Unbind VAO
    glBindVertexArray(0);
}

void ParticleSystem::update(float deltaTime) {
    activeParticles_ = 0;
    for (auto& particle : particles_) {
        if (!particle.active) continue;

        particle.life -= deltaTime;
        if (particle.life <= 0) {
            particle.active = false;
            continue;
        }

        // Update position
        particle.position += particle.velocity * deltaTime;

        // Update color alpha based on life
        particle.color.a = (particle.life / particle.maxLife) * particle.color.a;

        // Count active particles
        ++activeParticles_;
    }

    // Update instance data if using instancing
    if (useInstancing_) {
        updateInstanceData();
    }
}

void ParticleSystem::updateInstanceData() {
    std::vector<glm::mat4> instanceData;
    instanceData.reserve(activeParticles_);

    for (const auto& particle : particles_) {
        if (!particle.active) continue;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, particle.position);
        model = glm::scale(model, glm::vec3(particle.size));

        instanceData.push_back(model);
    }

    glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
    glBufferSubData(GL_ARRAY_BUFFER, 0, instanceData.size() * sizeof(glm::mat4), instanceData.data());
}

void ParticleSystem::render() {
    // Set up OpenGL state
    glEnable(GL_BLEND);
    if (additiveBlending_) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    } else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    if (depthTestEnabled_) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }

    // Bind texture if available
    if (texture_) {
        glBindTexture(GL_TEXTURE_2D, texture_);
    }

    // Render particles
    if (useInstancing_) {
        renderInstanced();
    } else {
        renderBatched();
    }

    // Reset OpenGL state
    glEnable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

void ParticleSystem::renderInstanced() {
    glBindVertexArray(vao_);
    glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, activeParticles_);
    glBindVertexArray(0);
}

void ParticleSystem::renderBatched() {
    glBindVertexArray(vao_);
    
    size_t particlesRendered = 0;
    for (const auto& particle : particles_) {
        if (!particle.active) continue;

        if (particlesRendered >= batchSize_) {
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            particlesRendered = 0;
        }

        // Update model matrix for this particle
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, particle.position);
        model = glm::scale(model, glm::vec3(particle.size));

        // Update uniforms for this particle
        // Note: You'll need to set these in your shader:
        // - model matrix
        // - color
        // - any other particle-specific attributes

        ++particlesRendered;
    }

    // Render any remaining particles
    if (particlesRendered > 0) {
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    }

    glBindVertexArray(0);
}

void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life) {
    size_t index = findInactiveParticle();
    if (index >= particles_.size()) return;

    auto& particle = particles_[index];
    particle.position = position;
    particle.velocity = velocity;
    particle.color = color;
    particle.size = std::clamp(size, minParticleSize_, maxParticleSize_);
    particle.life = life;
    particle.maxLife = life;
    particle.active = true;
}

void ParticleSystem::emitBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> velDist(minVel, maxVel);
    std::uniform_real_distribution<float> angleDist(0.0f, 2.0f * 3.14159265358979323846f);

    for (size_t i = 0; i < count; ++i) {
        float angle = angleDist(gen);
        float velocity = velDist(gen);
        glm::vec3 direction(std::cos(angle), std::sin(angle), 0.0f);
        emit(position, direction * velocity, color, size, life);
    }
}

size_t ParticleSystem::findInactiveParticle() {
    for (size_t i = 0; i < particles_.size(); ++i) {
        if (!particles_[i].active) return i;
    }
    return particles_.size();
}

void ParticleSystem::setTexture(const std::string& texturePath) {
    // Load texture using your preferred texture loading method
    // For now, we'll just create a simple white texture
    if (texture_) glDeleteTextures(1, &texture_);

    glGenTextures(1, &texture_);
    glBindTexture(GL_TEXTURE_2D, texture_);

    unsigned char white[] = { 255, 255, 255, 255 };
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, white);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void ParticleSystem::setHighContrastMode(bool enabled) {
    highContrastMode_ = enabled;
}

void ParticleSystem::setDepthTesting(bool enabled) {
    depthTestEnabled_ = enabled;
}

void ParticleSystem::setBlendMode(bool additive) {
    additiveBlending_ = additive;
}

void ParticleSystem::setParticleSize(float minSize, float maxSize) {
    minParticleSize_ = minSize;
    maxParticleSize_ = maxSize;
}

void ParticleSystem::setMaxParticles(size_t count) {
    maxParticles_ = count;
    particles_.resize(maxParticles_);
    
    // Reallocate instance buffer
    if (useInstancing_) {
        glBindBuffer(GL_ARRAY_BUFFER, instanceVBO_);
        glBufferData(GL_ARRAY_BUFFER, maxParticles_ * sizeof(float) * 16, nullptr, GL_DYNAMIC_DRAW);
    }
}

void ParticleSystem::setInstancingEnabled(bool enabled) {
    useInstancing_ = enabled;
}

void ParticleSystem::setBatchSize(size_t size) {
    batchSize_ = size;
}

void ParticleSystem::enableDebugView(bool enabled) {
    debugViewEnabled_ = enabled;
}

void ParticleSystem::setDebugColor(const glm::vec4& color) {
    debugColor_ = color;
}

} // namespace Graphics
} // namespace TurtleEngine 