#include "../include/TemporalVisualizer.hpp"
#include <iostream>
#include <iomanip>
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

namespace TurtleEngine {
namespace Temporal {

// Define particle colors for each anomaly type
namespace {
    const glm::vec4 RIFT_COLOR(0.4f, 0.1f, 0.9f, 0.8f);        // Purple
    const glm::vec4 STASIS_COLOR(0.0f, 0.9f, 1.0f, 0.8f);      // Cyan
    const glm::vec4 DILATION_COLOR(0.2f, 0.6f, 0.9f, 0.8f);    // Blue
    const glm::vec4 ACCELERATION_COLOR(0.9f, 0.5f, 0.1f, 0.8f); // Orange
    const glm::vec4 REVERSION_COLOR(0.9f, 0.2f, 0.4f, 0.8f);   // Red
}

TemporalVisualizer::TemporalVisualizer(std::shared_ptr<Graphics::ParticleSystem> particleSystem)
    : m_particleSystem(particleSystem),
      m_rippleShader(nullptr),
      m_wireframeShader(nullptr),
      m_temporalSystem(nullptr),
      m_visualizationMode(VisualizationMode::COMBINED),
      m_highContrastMode(false),
      m_debugEnabled(false) {
    std::cout << "[TemporalVisualizer] Created" << std::endl;
}

void TemporalVisualizer::initialize() {
    // In a real implementation, we would load shaders here
    std::cout << "[TemporalVisualizer] Initialized" << std::endl;
    
    // Set up callback for temporal system
    if (m_temporalSystem) {
        m_temporalSystem->setDebugDrawCallback([this](const TemporalAnomaly& anomaly) {
            std::cout << "[TemporalVisualizer] Debug anomaly callback: " 
                      << static_cast<int>(anomaly.getType()) << " at " 
                      << glm::to_string(anomaly.getPosition()) << std::endl;
            
            // Create initial visualization on creation
            visualizeAnomaly(anomaly, 0.0f);
        });
    }
}

void TemporalVisualizer::setTemporalSystem(std::shared_ptr<TemporalAnomalySystem> system) {
    m_temporalSystem = system;
    
    // Re-initialize callback
    if (m_temporalSystem) {
        m_temporalSystem->setDebugDrawCallback([this](const TemporalAnomaly& anomaly) {
            std::cout << "[TemporalVisualizer] Debug anomaly callback: " 
                      << static_cast<int>(anomaly.getType()) << " at " 
                      << glm::to_string(anomaly.getPosition()) << std::endl;
            
            // Create initial visualization on creation
            visualizeAnomaly(anomaly, 0.0f);
        });
    }
}

void TemporalVisualizer::update(float deltaTime) {
    if (!m_temporalSystem) return;
    
    // Update all active anomaly visualizations
    for (const auto& anomaly : m_temporalSystem->getAnomalies()) {
        if (anomaly.isActive()) {
            visualizeAnomaly(anomaly, deltaTime);
        }
    }
    
    // Update ripple effects
    auto rippleIt = m_rippleEffects.begin();
    while (rippleIt != m_rippleEffects.end()) {
        rippleIt->age += deltaTime;
        
        // Remove expired ripples
        if (rippleIt->age >= rippleIt->lifetime) {
            rippleIt = m_rippleEffects.erase(rippleIt);
        } else {
            // Expand ripple
            float progress = rippleIt->age / rippleIt->lifetime;
            rippleIt->radius = rippleIt->radius * (1.0f + deltaTime * 2.0f);
            
            // Fade out
            rippleIt->color.a = 1.0f - progress;
            
            ++rippleIt;
        }
    }
}

void TemporalVisualizer::render(const glm::mat4& view, const glm::mat4& projection) {
    // In a real implementation, we would render ripples and wireframes here
    
    // Particle rendering is handled by the particle system
    // It's called elsewhere in the rendering loop
    
    // Debug output for active anomalies
    if (m_debugEnabled && m_temporalSystem) {
        std::cout << "[TemporalVisualizer] Debug Render: " 
                  << m_temporalSystem->getAnomalyCount() << " anomalies, "
                  << m_rippleEffects.size() << " ripples" << std::endl;
    }
}

void TemporalVisualizer::drawDebugInfo(bool enabled) {
    m_debugEnabled = enabled;
}

void TemporalVisualizer::setHighContrastMode(bool enabled) {
    m_highContrastMode = enabled;
}

void TemporalVisualizer::setVisualizationMode(VisualizationMode mode) {
    m_visualizationMode = mode;
    std::cout << "[TemporalVisualizer] Visualization mode set to: " 
              << static_cast<int>(mode) << std::endl;
}

void TemporalVisualizer::visualizeAnomaly(const TemporalAnomaly& anomaly, float deltaTime) {
    // Dispatch to specific visualization based on anomaly type
    switch (anomaly.getType()) {
        case AnomalyType::RIFT:
            visualizeRift(anomaly, deltaTime);
            break;
            
        case AnomalyType::STASIS:
            visualizeStasis(anomaly, deltaTime);
            break;
            
        case AnomalyType::DILATION:
            visualizeDilation(anomaly, deltaTime);
            break;
            
        case AnomalyType::ACCELERATION:
            visualizeAcceleration(anomaly, deltaTime);
            break;
            
        case AnomalyType::REVERSION:
            visualizeReversion(anomaly, deltaTime);
            break;
    }
}

void TemporalVisualizer::visualizeRift(const TemporalAnomaly& anomaly, float deltaTime) {
    if (!m_particleSystem) return;
    
    // Particle emission rate based on anomaly intensity
    float particlesToEmit = 150.0f * anomaly.getIntensityFactor() * deltaTime; // Base 150 particles/sec
    
    // Emit particles periodically
    int numParticles = static_cast<int>(particlesToEmit);
    if (numParticles > 0) {
        
        // Rift creates chaotic, unpredictable particles
        createAnomalyParticles(
            anomaly,
            RIFT_COLOR,
            0.3f + 0.2f * anomaly.getIntensityFactor(),  // Size
            0.8f + 0.5f * anomaly.getIntensityFactor(),  // Lifetime
            numParticles // Count
        );
    }
        
    // Create periodic ripple effects (timer can remain static for simplicity, 
    // as it controls frequency, not quantity per anomaly)
    if (m_visualizationMode == VisualizationMode::RIPPLES || 
        m_visualizationMode == VisualizationMode::COMBINED) {
        static float rippleTimer = 0.0f;
        rippleTimer += deltaTime;
            
        if (rippleTimer >= 0.5f) { // Emit ripple every 0.5 seconds
            rippleTimer = 0.0f;
            createRippleEffect(
                anomaly,
                anomaly.getIntensityFactor(),
                RIFT_COLOR
            );
        }
    }

    // Draw wireframe if in wireframe mode or debug mode
    if (m_visualizationMode == VisualizationMode::WIREFRAME || 
        m_visualizationMode == VisualizationMode::COMBINED || 
        m_debugEnabled) {
        drawAnomalyWireframe(anomaly, RIFT_COLOR);
    }
}

void TemporalVisualizer::visualizeStasis(const TemporalAnomaly& anomaly, float deltaTime) {
    if (!m_particleSystem) return;
    
    // Stasis creates slow-moving, crystalline particles
    float particlesToEmit = 25.0f * anomaly.getIntensityFactor() * deltaTime; // Base 25 particles/sec
    int numParticles = static_cast<int>(particlesToEmit);
    
    if (numParticles > 0) {
        createAnomalyParticles(
            anomaly,
            STASIS_COLOR,
            0.15f + 0.1f * anomaly.getIntensityFactor(),  // Size
            2.0f + 1.0f * anomaly.getIntensityFactor(),   // Lifetime
            numParticles // Count
        );
    }

    // Stasis has subtle ripple effects
    if (m_visualizationMode == VisualizationMode::RIPPLES || 
        m_visualizationMode == VisualizationMode::COMBINED) {
        static float rippleTimer = 0.0f;
        rippleTimer += deltaTime;
        
        if (rippleTimer >= 1.0f) { // Emit ripple every 1.0 second
            rippleTimer = 0.0f;
            createRippleEffect(
                anomaly,
                0.5f * anomaly.getIntensityFactor(),
                STASIS_COLOR
            );
        }
    }

    // Draw wireframe if in wireframe mode or debug mode
    if (m_visualizationMode == VisualizationMode::WIREFRAME || 
        m_visualizationMode == VisualizationMode::COMBINED || 
        m_debugEnabled) {
        drawAnomalyWireframe(anomaly, STASIS_COLOR);
    }
}

void TemporalVisualizer::visualizeDilation(const TemporalAnomaly& anomaly, float deltaTime) {
    if (!m_particleSystem) return;
    
    // Dilation creates slow, flowing particles
    float particlesToEmit = 50.0f * anomaly.getIntensityFactor() * deltaTime; // Base 50 particles/sec
    int numParticles = static_cast<int>(particlesToEmit);
    
    if (numParticles > 0) {
        createAnomalyParticles(
            anomaly,
            DILATION_COLOR,
            0.2f + 0.15f * anomaly.getIntensityFactor(),  // Size
            1.5f + 0.8f * anomaly.getIntensityFactor(),   // Lifetime
            numParticles // Count
        );
    }

    // Dilation has regular ripple effects
    if (m_visualizationMode == VisualizationMode::RIPPLES || 
        m_visualizationMode == VisualizationMode::COMBINED) {
        static float rippleTimer = 0.0f;
        rippleTimer += deltaTime;
        
        if (rippleTimer >= 0.8f) { // Emit ripple every 0.8 seconds
            rippleTimer = 0.0f;
            createRippleEffect(
                anomaly,
                0.7f * anomaly.getIntensityFactor(),
                DILATION_COLOR
            );
        }
    }

    // Draw wireframe if in wireframe mode or debug mode
    if (m_visualizationMode == VisualizationMode::WIREFRAME || 
        m_visualizationMode == VisualizationMode::COMBINED || 
        m_debugEnabled) {
        drawAnomalyWireframe(anomaly, DILATION_COLOR);
    }
}

void TemporalVisualizer::visualizeAcceleration(const TemporalAnomaly& anomaly, float deltaTime) {
    if (!m_particleSystem) return;
    
    // Acceleration creates fast, energetic particles
    float particlesToEmit = 240.0f * anomaly.getIntensityFactor() * deltaTime; // Base 240 particles/sec
    int numParticles = static_cast<int>(particlesToEmit);
    
    if (numParticles > 0) {
        createAnomalyParticles(
            anomaly,
            ACCELERATION_COLOR,
            0.1f + 0.1f * anomaly.getIntensityFactor(),  // Size
            0.5f + 0.3f * anomaly.getIntensityFactor(),  // Lifetime
            numParticles // Count
        );
    }

    // Acceleration has fast ripple effects
    if (m_visualizationMode == VisualizationMode::RIPPLES || 
        m_visualizationMode == VisualizationMode::COMBINED) {
        static float rippleTimer = 0.0f;
        rippleTimer += deltaTime;
        
        if (rippleTimer >= 0.3f) { // Emit ripple every 0.3 seconds
            rippleTimer = 0.0f;
            createRippleEffect(
                anomaly,
                0.9f * anomaly.getIntensityFactor(),
                ACCELERATION_COLOR
            );
        }
    }

    // Draw wireframe if in wireframe mode or debug mode
    if (m_visualizationMode == VisualizationMode::WIREFRAME || 
        m_visualizationMode == VisualizationMode::COMBINED || 
        m_debugEnabled) {
        drawAnomalyWireframe(anomaly, ACCELERATION_COLOR);
    }
}

void TemporalVisualizer::visualizeReversion(const TemporalAnomaly& anomaly, float deltaTime) {
    if (!m_particleSystem) return;
    
    // Reversion creates particles that appear to move backward
    float particlesToEmit = 80.0f * anomaly.getIntensityFactor() * deltaTime; // Base 80 particles/sec
    int numParticles = static_cast<int>(particlesToEmit);
    
    if (numParticles > 0) {
        createAnomalyParticles(
            anomaly,
            REVERSION_COLOR,
            0.25f + 0.15f * anomaly.getIntensityFactor(),  // Size
            1.0f + 0.5f * anomaly.getIntensityFactor(),    // Lifetime
            numParticles // Count
        );
    }

    // Reversion has inward-moving ripple effects
    if (m_visualizationMode == VisualizationMode::RIPPLES || 
        m_visualizationMode == VisualizationMode::COMBINED) {
        static float rippleTimer = 0.0f;
        rippleTimer += deltaTime;
        
        if (rippleTimer >= 0.6f) { // Emit ripple every 0.6 seconds
            rippleTimer = 0.0f;
            createRippleEffect(
                anomaly,
                0.8f * anomaly.getIntensityFactor(),
                REVERSION_COLOR
            );
        }
    }

    // Draw wireframe if in wireframe mode or debug mode
    if (m_visualizationMode == VisualizationMode::WIREFRAME || 
        m_visualizationMode == VisualizationMode::COMBINED || 
        m_debugEnabled) {
        drawAnomalyWireframe(anomaly, REVERSION_COLOR);
    }
}

void TemporalVisualizer::createAnomalyParticles(
    const TemporalAnomaly& anomaly,
    const glm::vec4& color,
    float particleSize,
    float particleLifetime,
    int particleCount
) {
    if (!m_particleSystem) return;
    
    // Skip if not in particles mode and not in combined mode
    if (m_visualizationMode != VisualizationMode::PARTICLES && 
        m_visualizationMode != VisualizationMode::COMBINED) {
        return;
    }
    
    // Get anomaly parameters
    glm::vec3 position = anomaly.getPosition();
    float radius = anomaly.getRadius();
    
    // Adjust for high contrast mode
    glm::vec4 adjustedColor = color;
    if (m_highContrastMode) {
        adjustedColor.a = 1.0f;
    }
    
    // In a real implementation, we would create particles here
    // by interfacing with the particle system
    
    if (m_debugEnabled) {
        std::cout << "[TemporalVisualizer] Created " << particleCount 
                  << " particles for anomaly type " << static_cast<int>(anomaly.getType()) 
                  << " at " << glm::to_string(position) << std::endl;
    }
}

void TemporalVisualizer::createRippleEffect(
    const TemporalAnomaly& anomaly,
    float intensity,
    const glm::vec4& color
) {
    // Create a new ripple effect
    RippleEffect ripple;
    ripple.position = anomaly.getPosition();
    ripple.radius = 0.1f;
    ripple.intensity = intensity;
    ripple.lifetime = 1.5f;
    ripple.age = 0.0f;
    ripple.color = color;
    
    m_rippleEffects.push_back(ripple);
    
    if (m_debugEnabled) {
        std::cout << "[TemporalVisualizer] Created ripple effect at " 
                  << glm::to_string(anomaly.getPosition()) << std::endl;
    }
}

void TemporalVisualizer::drawAnomalyWireframe(
    const TemporalAnomaly& anomaly,
    const glm::vec4& color
) {
    // In a real implementation, we would draw a wireframe sphere representing
    // the anomaly's area of effect
    
    if (m_debugEnabled) {
        std::cout << "[TemporalVisualizer] Drew wireframe for anomaly at " 
                  << glm::to_string(anomaly.getPosition()) << " with radius " 
                  << anomaly.getRadius() << std::endl;
    }
}

} // namespace Temporal
} // namespace TurtleEngine 