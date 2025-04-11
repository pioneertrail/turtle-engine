#pragma once

#include "TemporalAnomalySystem.hpp"
#include <memory>
#include <vector>
#include <glm/glm.hpp>

namespace TurtleEngine {

// Forward declarations
namespace Graphics {
class ParticleSystem;
class ShaderProgram;
}

namespace Temporal {

/**
 * @class TemporalVisualizer
 * @brief Provides visual feedback for temporal anomalies
 */
class TemporalVisualizer {
public:
    TemporalVisualizer(std::shared_ptr<Graphics::ParticleSystem> particleSystem);
    ~TemporalVisualizer() = default;
    
    // Initialize the visualizer
    void initialize();
    
    // Set the temporal anomaly system to visualize
    void setTemporalSystem(std::shared_ptr<TemporalAnomalySystem> system);
    
    // Update visual effects
    void update(float deltaTime);
    
    // Render anomaly visualizations
    void render(const glm::mat4& view, const glm::mat4& projection);
    
    // Debug visualization
    void drawDebugInfo(bool enabled);
    
    // Toggle high contrast mode
    void setHighContrastMode(bool enabled);
    
    // Toggle different visualization modes
    enum class VisualizationMode {
        PARTICLES,    // Particle-based visualization
        RIPPLES,      // Ripple effect visualization
        WIREFRAME,    // Wireframe visualization
        COMBINED      // Combined visualization
    };
    void setVisualizationMode(VisualizationMode mode);
    
private:
    // Internal visualization functions
    void visualizeAnomaly(const TemporalAnomaly& anomaly, float deltaTime);
    void visualizeRift(const TemporalAnomaly& anomaly, float deltaTime);
    void visualizeStasis(const TemporalAnomaly& anomaly, float deltaTime);
    void visualizeDilation(const TemporalAnomaly& anomaly, float deltaTime);
    void visualizeAcceleration(const TemporalAnomaly& anomaly, float deltaTime);
    void visualizeReversion(const TemporalAnomaly& anomaly, float deltaTime);
    
    // Create particles for an anomaly
    void createAnomalyParticles(const TemporalAnomaly& anomaly, 
                               const glm::vec4& color,
                               float particleSize,
                               float particleLifetime,
                               int particleCount);
    
    // Create ripple effect at anomaly location
    void createRippleEffect(const TemporalAnomaly& anomaly, 
                           float intensity,
                           const glm::vec4& color);
    
    // Draw debug wireframe for anomaly
    void drawAnomalyWireframe(const TemporalAnomaly& anomaly, 
                             const glm::vec4& color);
    
    // Member variables
    std::shared_ptr<Graphics::ParticleSystem> m_particleSystem;
    std::shared_ptr<Graphics::ShaderProgram> m_rippleShader;
    std::shared_ptr<Graphics::ShaderProgram> m_wireframeShader;
    std::shared_ptr<TemporalAnomalySystem> m_temporalSystem;
    VisualizationMode m_visualizationMode;
    bool m_highContrastMode;
    bool m_debugEnabled;
    
    // Visualization parameters
    struct RippleEffect {
        glm::vec3 position;
        float radius;
        float intensity;
        float lifetime;
        float age;
        glm::vec4 color;
    };
    std::vector<RippleEffect> m_rippleEffects;
};

} // namespace Temporal
} // namespace TurtleEngine 