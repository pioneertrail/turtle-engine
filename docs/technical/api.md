# API Documentation

## Overview
This document provides a comprehensive overview of the TurtleEngine API, focusing on the public interfaces for major systems. It serves as a reference for developers working with the engine components.

## Core Engine API

### Engine Initialization
```cpp
// Initialize the engine with configuration
bool TurtleEngine::initialize(const EngineConfig& config);

// Shutdown the engine
void TurtleEngine::shutdown();

// Run the main game loop
int TurtleEngine::run();
```

### Engine Configuration
```cpp
struct EngineConfig {
    std::string windowTitle = "TurtleEngine";
    int windowWidth = 1280;
    int windowHeight = 720;
    bool fullscreen = false;
    bool vsync = true;
    bool enableDebug = false;
    RenderAPIType renderAPI = RenderAPIType::OPENGL;
    std::string assetPath = "assets/";
    std::string logPath = "logs/";
    float targetFPS = 60.0f;
};
```

## Combat System API

### PlasmaWeapon API
```cpp
namespace TurtleEngine {
namespace Combat {

class PlasmaWeapon {
public:
    // Firing modes enumeration
    enum class FiringMode {
        BURST,     // Quick burst of plasma particles
        BEAM,      // Continuous beam of plasma
        CHARGED,   // Charged shot (variable power based on charge time)
        SCATTER    // Wide-angle scatter shot
    };
    
    // Constructor with particle system
    PlasmaWeapon(std::shared_ptr<ParticleSystem> particleSystem, 
                 float maxCharge = DEFAULT_MAX_CHARGE);
    
    // Core gameplay methods
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);
    float beginCharging();
    bool fire(const glm::vec3& origin, const glm::vec3& direction);
    bool quickFire(const glm::vec3& origin, const glm::vec3& direction);
    
    // Configuration
    void setFiringMode(FiringMode mode);
    
    // Status queries
    float getChargePercentage() const;
    bool isCoolingDown() const;
    
    // Debug
    void enableDebugVisualization(bool enabled);
};

} // namespace Combat
} // namespace TurtleEngine
```

### AIConstruct API
```cpp
namespace TurtleEngine {
namespace Combat {

class AIConstruct {
public:
    // AI states
    enum class State {
        IDLE,       // Not moving or attacking
        PATROL,     // Moving along patrol points
        ATTACK,     // Actively attacking player
        RETREAT,    // Moving away from player
        DAMAGED     // Taking damage (temporary state)
    };

    // AI types
    enum class Type {
        SENTRY,     // Stationary, long-range attacks
        HUNTER,     // Mobile, aggressive
        GUARDIAN,   // Defensive, high health
        SWARM       // Low health, spawns in groups
    };
    
    // Constructors
    AIConstruct(std::shared_ptr<ParticleSystem> particleSystem,
                Type type = Type::SENTRY,
                const glm::vec3& position = glm::vec3(0.0f),
                float health = DEFAULT_HEALTH);
    
    // Core gameplay methods
    void update(float deltaTime, const glm::vec3& playerPosition);
    void render(const glm::mat4& view, const glm::mat4& projection);
    bool applyDamage(float amount, const glm::vec3& damageSource);
    bool applyDamage(const DamageInfo& damage);
    
    // Status queries
    glm::vec3 getPosition() const;
    State getState() const;
    float getHealth() const;
    float getMaxHealth() const;
    float getHealthPercentage() const;
    bool isAlive() const;
    HealthComponent& getHealthComponent();
    
    // Configuration
    void addPatrolPoint(const glm::vec3& point);
    void setAttackRange(float range);
    void setMovementSpeed(float speed);
    
    // Debug and testing
    void enableDebugVisualization(bool enabled);
    void forceState(State state);
    void setStateChangeCallback(std::function<void(State, State)> callback);
};

} // namespace Combat
} // namespace TurtleEngine
```

### HealthSystem API
```cpp
namespace TurtleEngine {
namespace Combat {

// Damage types
enum class DamageType {
    PHYSICAL,   // Physical damage (kinetic, impact)
    PLASMA,     // Plasma-based energy damage
    ENERGY,     // General energy damage
    KINETIC,    // Specific kinetic damage (subtype of PHYSICAL)
    SONIC,      // Sonic wave damage
    TEMPORAL,   // Time-based damage from anomalies
    PSYCHIC,    // Mental/consciousness damage
    COUNT       // Meta-entry for iterating over types
};

// Damage information structure
struct DamageInfo {
    float amount;                // Base damage amount
    DamageType type;             // Type of damage
    glm::vec3 source;            // Source position of damage
    glm::vec3 direction;         // Direction of damage application
    float impactForce;           // Force of impact (for knockback)
    bool isCritical;             // Whether this is a critical hit
    std::string sourceIdentifier; // Identifier of damage source
    
    // Constructor with default values
    DamageInfo(float damageAmount = 0.0f,
               DamageType damageType = DamageType::PHYSICAL,
               const glm::vec3& sourcePos = glm::vec3(0.0f),
               const glm::vec3& dir = glm::vec3(0.0f, 0.0f, 1.0f),
               float force = 0.0f,
               bool critical = false,
               const std::string& identifier = "");
};

// Resilience class for damage mitigation
class Resilience {
public:
    Resilience();
    
    // Core functionality
    float processDamage(const DamageInfo& damage);
    
    // Configuration
    void setResistance(DamageType type, float value);
    float getResistance(DamageType type) const;
    void setFlatReduction(float value);
    float getFlatReduction() const;
    void setShield(float value);
    float getShield() const;
    
    // Shield handling
    float processShieldDamage(float amount);
};

// Health component for entities
class HealthComponent {
public:
    HealthComponent(float maxHealth = 100.0f,
                   std::shared_ptr<Graphics::ParticleSystem> particleSystem = nullptr);
    
    // Core functionality
    float applyDamage(const DamageInfo& damage);
    float applyHealing(float amount, const glm::vec3& source = glm::vec3(0.0f));
    
    // Status queries
    float getCurrentHealth() const;
    float getMaxHealth() const;
    float getHealthPercentage() const;
    bool isAlive() const;
    const glm::vec3& getPosition() const;
    
    // Configuration
    void setMaxHealth(float newMaxHealth);
    Resilience& getResilience();
    void setPosition(const glm::vec3& position);
    
    // Callbacks
    void setDamageCallback(std::function<void(const DamageInfo&, float)> callback);
    void setHealingCallback(std::function<void(float, const glm::vec3&)> callback);
    void setDeathCallback(std::function<void()> callback);
    
    // Debug
    void enableDebugVisualization(bool enabled);
    bool isDebugVisualizationEnabled() const;
    std::string getDebugInfo() const;
};

} // namespace Combat
} // namespace TurtleEngine
```

## Temporal Anomaly API

```cpp
namespace TurtleEngine {
namespace Temporal {

// Anomaly types
enum class AnomalyType {
    RIFT,       // Spatial discontinuity in the timestream
    STASIS,     // Time freeze effect
    DILATION,   // Time slowdown effect
    ACCELERATION,// Time speedup effect
    REVERSION   // Time reversal effect
};

// Anomaly effect structure
struct AnomalyEffect {
    AnomalyType type;            // Type of anomaly
    float timeDistortion;        // Time distortion factor
    glm::vec3 position;          // Center position
    float radius;                // Radius of effect
    float duration;              // Duration in seconds
    std::string sourceIdentifier; // Source identifier
    
    // Constructor with default values
    AnomalyEffect(AnomalyType anomalyType = AnomalyType::RIFT,
                  float distortionFactor = 0.5f,
                  const glm::vec3& pos = glm::vec3(0.0f),
                  float rad = 5.0f,
                  float dur = 10.0f,
                  const std::string& identifier = "");
};

// Interface for entities affected by anomalies
class AffectedEntity {
public:
    virtual ~AffectedEntity() = default;
    
    // Pure virtual methods to be implemented
    virtual void applyTemporalEffect(const AnomalyEffect& effect, float deltaTime) = 0;
    virtual glm::vec3 getPosition() const = 0;
    virtual std::string getIdentifier() const = 0;
};

// Temporal anomaly class
class TemporalAnomaly {
public:
    TemporalAnomaly(const AnomalyEffect& effect);
    
    // Core functionality
    void update(float deltaTime);
    bool affectsEntity(const AffectedEntity& entity) const;
    void applyToEntity(AffectedEntity& entity, float deltaTime);
    
    // Status queries
    bool isActive() const;
    glm::vec3 getPosition() const;
    float getRadius() const;
    AnomalyType getType() const;
    const AnomalyEffect& getEffect() const;
    float getRemainingDuration() const;
    float getIntensityFactor() const;
};

// System managing multiple anomalies
class TemporalAnomalySystem {
public:
    TemporalAnomalySystem();
    
    // Core functionality
    void createAnomaly(const AnomalyEffect& effect);
    void update(float deltaTime);
    void registerEntity(std::shared_ptr<AffectedEntity> entity);
    void unregisterEntity(const std::string& entityId);
    
    // Status queries
    const std::vector<TemporalAnomaly>& getAnomalies() const;
    size_t getAnomalyCount() const;
    
    // Debug
    using DebugDrawCallback = std::function<void(const TemporalAnomaly&)>;
    void setDebugDrawCallback(DebugDrawCallback callback);
};

} // namespace Temporal
} // namespace TurtleEngine
```

## Input System API

```cpp
namespace TurtleEngine {
namespace Input {

// Input event types
enum class InputEventType {
    KEY_PRESSED,
    KEY_RELEASED,
    MOUSE_PRESSED,
    MOUSE_RELEASED,
    MOUSE_MOVED,
    GESTURE_RECOGNIZED,
    GESTURE_FAILED
};

// Input event structure
struct InputEvent {
    InputEventType type;
    int key;            // For keyboard events
    int button;         // For mouse button events
    glm::vec2 position; // For mouse position events
    std::string gesture; // For gesture events
    float confidence;   // For gesture events
};

// Input manager
class InputManager {
public:
    static InputManager& getInstance();
    
    // Event handling
    void processEvent(const InputEvent& event);
    void update(float deltaTime);
    
    // Input state queries
    bool isKeyPressed(int key) const;
    bool isMouseButtonPressed(int button) const;
    glm::vec2 getMousePosition() const;
    
    // Gesture system
    bool isGestureActive(const std::string& gestureName) const;
    float getGestureConfidence(const std::string& gestureName) const;
    
    // Event callbacks
    void setKeyCallback(std::function<void(int, bool)> callback);
    void setMouseButtonCallback(std::function<void(int, bool, const glm::vec2&)> callback);
    void setMouseMoveCallback(std::function<void(const glm::vec2&)> callback);
    void setGestureCallback(std::function<void(const std::string&, float)> callback);
};

} // namespace Input
} // namespace TurtleEngine
```

## Particle System API

```cpp
namespace TurtleEngine {
namespace Graphics {

// Particle system settings
struct ParticleSystemSettings {
    size_t maxParticles = 1000;
    bool useGPUInstancing = true;
    bool useCulling = true;
    float cullingDistance = 100.0f;
    bool usePooling = true;
};

// Particle emitter settings
struct EmitterSettings {
    glm::vec3 position = glm::vec3(0.0f);
    glm::vec3 direction = glm::vec3(0.0f, 1.0f, 0.0f);
    float spreadAngle = 15.0f;
    float particleLifetime = 1.0f;
    float particleSize = 0.1f;
    float particleSpeed = 1.0f;
    glm::vec4 startColor = glm::vec4(1.0f);
    glm::vec4 endColor = glm::vec4(1.0f, 1.0f, 1.0f, 0.0f);
    float emissionRate = 10.0f;  // Particles per second
    bool loop = true;
    float duration = 0.0f;       // 0 = infinite
};

// Particle system class
class ParticleSystem {
public:
    ParticleSystem(const ParticleSystemSettings& settings = ParticleSystemSettings());
    
    // Core functionality
    void update(float deltaTime);
    void render(const glm::mat4& view, const glm::mat4& projection);
    
    // Emitter management
    int createEmitter(const EmitterSettings& settings);
    void updateEmitter(int emitterId, const EmitterSettings& settings);
    void startEmitter(int emitterId);
    void stopEmitter(int emitterId);
    void removeEmitter(int emitterId);
    
    // One-shot effects
    void emitParticles(const glm::vec3& position, 
                      const glm::vec3& direction, 
                      int count, 
                      const EmitterSettings& settings);
    
    // Status queries
    size_t getActiveParticleCount() const;
    
    // Configuration
    void configure(const ParticleSystemSettings& settings);
    void setShader(std::shared_ptr<Shader> shader);
    
    // Debug
    void enableDebugVisualization(bool enabled);
};

} // namespace Graphics
} // namespace TurtleEngine
```

## Error Handling and Logging API

```cpp
namespace TurtleEngine {
namespace Core {

// Log levels
enum class LogLevel {
    TRACE,
    DEBUG,
    INFO,
    WARNING,
    ERROR,
    FATAL
};

// Logger class
class Logger {
public:
    static Logger& getInstance();
    
    // Core logging methods
    void log(LogLevel level, const std::string& message);
    void trace(const std::string& message);
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    void fatal(const std::string& message);
    
    // Configuration
    void setLogLevel(LogLevel level);
    void setLogToConsole(bool enable);
    void setLogToFile(bool enable, const std::string& filename = "logs/engine.log");
    
    // Special logging
    void logPerformance(const std::string& system, float time);
    void logMemoryUsage(const std::string& system, size_t bytes);
};

// Error handling
class ErrorHandler {
public:
    static ErrorHandler& getInstance();
    
    // Error handling methods
    void handleError(const std::string& message, bool isFatal = false);
    void registerErrorCallback(std::function<void(const std::string&, bool)> callback);
};

} // namespace Core
} // namespace TurtleEngine
```

## Best Practices

### API Usage Guidelines
1. **Error Handling**: Always check return values from API calls that can fail
2. **Resource Management**: Use smart pointers for resource ownership
3. **Thread Safety**: Be aware of which APIs are thread-safe vs. single-threaded
4. **Performance**: Batch similar operations when possible

### Code Examples

#### Combat System Example
```cpp
// Setting up a weapon and AI construct
auto particleSystem = std::make_shared<Graphics::ParticleSystem>();
auto weapon = std::make_unique<Combat::PlasmaWeapon>(particleSystem);
auto enemy = std::make_unique<Combat::AIConstruct>(particleSystem, Combat::AIConstruct::Type::HUNTER);

// Adding patrol points
enemy->addPatrolPoint(glm::vec3(10.0f, 0.0f, 0.0f));
enemy->addPatrolPoint(glm::vec3(0.0f, 0.0f, 10.0f));
enemy->addPatrolPoint(glm::vec3(-10.0f, 0.0f, 0.0f));

// Setting up health resistances
auto& resilience = enemy->getHealthComponent().getResilience();
resilience.setResistance(Combat::DamageType::PLASMA, 0.3f);  // 30% plasma resistance

// Game loop
while (gameRunning) {
    float deltaTime = calculateDeltaTime();
    
    // Update enemy
    enemy->update(deltaTime, playerPosition);
    
    // Handle weapon charging/firing
    if (inputManager.isKeyPressed(KEY_CHARGE)) {
        weapon->beginCharging();
    }
    
    if (inputManager.isKeyReleased(KEY_FIRE)) {
        weapon->fire(playerPosition, aimDirection);
    }
    
    // Render
    weapon->render(viewMatrix, projectionMatrix);
    enemy->render(viewMatrix, projectionMatrix);
}
```

#### Temporal Anomaly Example
```cpp
// Create temporal anomaly system
Temporal::TemporalAnomalySystem anomalySystem;

// Register player as affected entity
class Player : public Temporal::AffectedEntity {
public:
    void applyTemporalEffect(const Temporal::AnomalyEffect& effect, float deltaTime) override {
        // Modify player speed based on time distortion
        m_speed *= effect.timeDistortion;
    }
    
    glm::vec3 getPosition() const override {
        return m_position;
    }
    
    std::string getIdentifier() const override {
        return "Player1";
    }
    
    // Player-specific methods...
    
private:
    glm::vec3 m_position;
    float m_speed;
};

auto player = std::make_shared<Player>();
anomalySystem.registerEntity(player);

// Create a temporal rift
Temporal::AnomalyEffect riftEffect(
    Temporal::AnomalyType::RIFT,
    0.5f,                 // 50% time dilation
    glm::vec3(0, 0, 0),   // Center position
    10.0f,                // 10 unit radius
    5.0f,                 // 5 second duration
    "PlayerGesture"       // Source identifier
);

anomalySystem.createAnomaly(riftEffect);

// Game loop
while (gameRunning) {
    float deltaTime = calculateDeltaTime();
    
    // Update anomalies
    anomalySystem.update(deltaTime);
}
``` 