# TurtleEngine

Engine for Silent Forge: Rebellion of Aethelgard

## Project Overview
This repository contains the source code for TurtleEngine, the custom game engine powering *Silent Forge: Rebellion of Aethelgard*. It includes core systems for rendering, physics, audio, and the unique Chronosign Language (CSL) gesture recognition system.

## Current Status (As of April 2025)
- **Phase 1: Core Systems** - In Progress
  - **Milestone 1.1: CSL Animation System** - In Progress
    - Basic gesture recognition for Khargail and Flammil implemented and validated.
    - Initial work on tribe-specific visual effects underway, focusing on Flammyx integration for the Flammil gesture (collaboration with Elena).
    - See `docs/project/roadmap.md` and `docs/project/tasks.md` for details.
  - **Milestone 1.2: Combat Mechanics** - In Progress (Marcus)
    - Hitbox detection and combo transition logging initiated.
  - **Milestone 1.3: Temporal Anomaly Framework** - Pending

## Hardware Requirements

### Minimum Requirements
- OpenGL 4.0 compatible graphics card
- 2GB RAM
- Windows 10/11 or Linux
- CMake 3.10 or higher
- C++17 compatible compiler

### Recommended Requirements
- OpenGL 4.0+ compatible graphics card
- 4GB RAM
- Modern CPU with good single-thread performance
- SSD storage

## Dependencies

- OpenGL 4.0
- GLEW
- GLFW3
- GLM

## Building the Project

1. Clone the repository:
```bash
git clone https://github.com/yourusername/turtle-engine.git
cd turtle-engine
```

2. Initialize and update submodules:
```bash
git submodule update --init --recursive
```

3. Create build directory:
```bash
mkdir build && cd build
```

4. Configure with CMake:
```bash
cmake ..
```

5. Build the project:
```bash
cmake --build .
```

## Project Structure

```
turtle-engine/
├── src/
│   ├── engine/         # Core engine components
│   ├── main.cpp        # Entry point
│   └── tests/          # Test suite
├── shaders/            # GLSL shaders
└── deps/              # External dependencies
```

## Solo Developer Git Workflow

This project uses a simplified Git workflow designed for solo development.

### Main Branch
- **new-main**: Primary development branch, should remain stable enough for builds

### Optional Working Branches
- **feature/XXX**: For experimental features or major changes
- **bugfix/XXX**: For complex bug fixes that require isolation
- **refactor/XXX**: For significant code reorganization

### Commit Message Format
```
<type>(<scope>): <subject>
```

#### Types
- **feat**: New feature
- **fix**: Bug fix
- **docs**: Documentation changes
- **style**: Formatting, missing semicolons, etc; no code change
- **refactor**: Code refactoring
- **test**: Adding tests, refactoring tests
- **perf**: Performance improvements
- **build**: Build system changes
- **chore**: Routine tasks, maintenance, etc.

#### Examples
```
feat(combat): add scatter firing mode to plasma weapons
fix(health): correct damage calculation for shield penetration
```

### Daily Workflow
For most changes, work directly on the `new-main` branch. For experimental features, create a feature branch and merge back when ready:

```bash
# Creating a feature branch
git checkout -b feature/new-feature

# When ready to merge back
git checkout new-main
git merge feature/new-feature
```

### Releasing
1. Update version numbers in relevant files
2. Commit with message `chore(release): vX.Y.Z`
3. Create tag: `git tag -a vX.Y.Z -m "Release vX.Y.Z: Brief description"`
4. Push changes and tag: `git push && git push --tags`

## Development Best Practices & Growth Plan

For detailed guidance on improving development practices as the project grows, see [Development Next Steps](docs/development-next-steps.md), which covers:

- Advanced Git workflow techniques
- Test coverage expansion strategies 
- Edge case documentation for complex systems
- Continuous improvement processes

## Features

- Hardware-aware OpenGL context configuration
- Performance monitoring
- Basic rendering pipeline
- Input handling
- Scene management

## Controls

- ESC: Exit application
- F1: Toggle performance metrics
- WASD: Camera movement (when implemented)
- Mouse: Camera rotation (when implemented)

## Performance Optimization

The engine automatically detects hardware capabilities and adjusts its behavior accordingly:
- Texture size limits
- Uniform buffer limits
- Vertex attribute limits
- Shader complexity

## License

This project is licensed under the MIT License - see the LICENSE file for details.

#pragma once

#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <functional>
#include <string>
#include <unordered_map>

namespace TurtleEngine {
namespace Temporal {

/**
 * @enum AnomalyType
 * @brief Types of temporal anomalies that can be created
 */
enum class AnomalyType {
    RIFT,       // Spatial discontinuity in the timestream
    STASIS,     // Time freeze effect
    DILATION,   // Time slowdown effect
    ACCELERATION,// Time speedup effect
    REVERSION   // Time reversal effect
};

/**
 * @struct AnomalyEffect
 * @brief Detailed information about a temporal anomaly
 */
struct AnomalyEffect {
    AnomalyType type;            // Type of anomaly
    float timeDistortion;        // Time distortion factor (0-1 for slowdown, >1 for speedup, negative for reversal)
    glm::vec3 position;          // Center position of the anomaly
    float radius;                // Radius of effect
    float duration;              // Duration in seconds
    std::string sourceIdentifier; // Identifier of anomaly source
    
    AnomalyEffect(
        AnomalyType anomalyType = AnomalyType::RIFT,
        float distortionFactor = 0.5f,
        const glm::vec3& pos = glm::vec3(0.0f),
        float rad = 5.0f,
        float dur = 10.0f,
        const std::string& identifier = ""
    ) : type(anomalyType), timeDistortion(distortionFactor), position(pos), 
        radius(rad), duration(dur), sourceIdentifier(identifier) {}
};

/**
 * @class AffectedEntity
 * @brief Interface for entities that can be affected by temporal anomalies
 */
class AffectedEntity {
public:
    virtual ~AffectedEntity() = default;
    
    // Apply temporal effect to this entity
    virtual void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) = 0;
    
    // Get entity position for spatial calculations
    virtual glm::vec3 getPosition() const = 0;
    
    // Get entity identifier
    virtual std::string getIdentifier() const = 0;
};

/**
 * @class TemporalAnomaly
 * @brief Manages temporal anomaly instances
 */
class TemporalAnomaly {
public:
    TemporalAnomaly(const AnomalyEffect& effect);
    ~TemporalAnomaly() = default;
    
    // Update the anomaly state
    void update(float deltaTime);
    
    // Check if entity is within effect radius
    bool affectsEntity(const AffectedEntity& entity) const;
    
    // Apply anomaly effect to an entity
    void applyToEntity(AffectedEntity& entity, float deltaTime);
    
    // Check if anomaly is still active
    bool isActive() const;
    
    // Get anomaly position
    glm::vec3 getPosition() const;
    
    // Get anomaly radius
    float getRadius() const;
    
    // Get anomaly type
    AnomalyType getType() const;
    
    // Get anomaly effect details
    const AnomalyEffect& getEffect() const;
    
    // Get remaining duration
    float getRemainingDuration() const;
    
    // Get intensity factor (based on remaining duration)
    float getIntensityFactor() const;
    
private:
    AnomalyEffect m_effect;
    float m_remainingDuration;
    float m_intensityFactor;
    bool m_isActive;
    float m_stabilityFactor; // How stable the anomaly is (affects visual effects)
};

/**
 * @class TemporalAnomalySystem
 * @brief Manages all temporal anomalies and their effects
 */
class TemporalAnomalySystem {
public:
    TemporalAnomalySystem();
    ~TemporalAnomalySystem() = default;
    
    // Create a new anomaly
    void createAnomaly(const AnomalyEffect& effect);
    
    // Update all anomalies
    void update(float deltaTime);
    
    // Register an entity to be affected by anomalies
    void registerEntity(std::shared_ptr<AffectedEntity> entity);
    
    // Unregister an entity
    void unregisterEntity(const std::string& entityId);
    
    // Get all active anomalies
    const std::vector<TemporalAnomaly>& getAnomalies() const;
    
    // Debug visualization callback
    using DebugDrawCallback = std::function<void(const TemporalAnomaly&)>;
    void setDebugDrawCallback(DebugDrawCallback callback);
    
    // Get count of active anomalies
    size_t getAnomalyCount() const;
    
    // Clear all anomalies
    void clearAnomalies();
    
    // Test if a point in space is affected by any anomaly
    bool isPointAffected(const glm::vec3& point) const;
    
    // Get combined time distortion factor at a point in space
    float getTimeDistortionAt(const glm::vec3& point) const;
    
private:
    std::vector<TemporalAnomaly> m_anomalies;
    std::unordered_map<std::string, std::shared_ptr<AffectedEntity>> m_entities;
    DebugDrawCallback m_debugDrawCallback;
    
    // Process anomaly-entity interactions
    void processInteractions(float deltaTime);
    
    // Remove expired anomalies
    void removeExpiredAnomalies();
};

} // namespace Temporal
} // namespace TurtleEngine 