#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <string>

namespace TurtleEngine {
namespace Graphics {

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    glm::vec4 color;
    float size;
    float life;
    float maxLife;
    bool active;
};

class ParticleSystem {
public:
    ParticleSystem(size_t maxParticles = 10000);
    ~ParticleSystem();

    void initialize();
    void update(float deltaTime);
    void render();

    // Particle emission methods
    void emit(const glm::vec3& position, const glm::vec3& velocity,
              const glm::vec4& color, float size, float life);
    void emitBurst(const glm::vec3& position, size_t count,
                   float minVel, float maxVel,
                   const glm::vec4& color, float size, float life);

    // Visual settings
    void setTexture(const std::string& texturePath);
    void setHighContrastMode(bool enabled);
    void setDepthTesting(bool enabled);
    void setBlendMode(bool additive);
    void setParticleSize(float minSize, float maxSize);

    // Performance settings
    void setMaxParticles(size_t count);
    void setInstancingEnabled(bool enabled);
    void setBatchSize(size_t size);

    // Debug visualization
    void enableDebugView(bool enabled);
    void setDebugColor(const glm::vec4& color);

private:
    // Rendering methods
    void initializeQuadBuffers();
    void updateInstanceData();
    void renderInstanced();
    void renderBatched();

    // Helper methods
    void recycleParticles();
    size_t findInactiveParticle();

    // Particle data
    std::vector<Particle> particles_;
    size_t maxParticles_;
    size_t activeParticles_;

    // Rendering data
    unsigned int vao_;
    unsigned int vbo_;
    unsigned int instanceVBO_;
    unsigned int texture_;
    bool useInstancing_;
    size_t batchSize_;

    // Visual settings
    bool highContrastMode_;
    bool depthTestEnabled_;
    bool additiveBlending_;
    float minParticleSize_;
    float maxParticleSize_;

    // Debug settings
    bool debugViewEnabled_;
    glm::vec4 debugColor_;
};

} // namespace Graphics
} // namespace TurtleEngine 