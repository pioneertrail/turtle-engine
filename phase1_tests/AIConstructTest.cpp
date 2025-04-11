#include <iostream>
#include <memory>
#include <glm/glm.hpp>
#include <string>
#include <functional>
#include <vector>
#include <unordered_map>
#include <random>
#include <algorithm>

// Mock ParticleSystem implementation
namespace TurtleEngine {
namespace Graphics {
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000) {}
        virtual ~ParticleSystem() = default;
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                        const glm::vec4& color, float size, float life) {
            std::cout << "[ParticleSystem] Emitted particle at (" 
                    << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
        
        virtual void emitBurst(const glm::vec3& position, size_t count,
                            float minVel, float maxVel,
                            const glm::vec4& color, float size, float life) {
            std::cout << "[ParticleSystem] Emitted burst of " << count << " particles" << std::endl;
        }
    };
}

// Simplified Health System for AIConstruct dependencies
namespace Combat {
    enum class DamageType {
        PHYSICAL,
        PLASMA,
        SONIC,
        TEMPORAL,
        PSYCHIC
    };

    struct DamageInfo {
        float amount;
        DamageType type;
        glm::vec3 source;
        bool isCritical;

        DamageInfo(float amount_ = 0.0f, 
                DamageType type_ = DamageType::PHYSICAL,
                const glm::vec3& source_ = glm::vec3(0.0f),
                bool isCritical_ = false)
            : amount(amount_), type(type_), source(source_), isCritical(isCritical_) {}
    };

    // Resilience class for damage resistance
    class Resilience {
    public:
        Resilience() : m_flatReduction(0.0f), m_shield(0.0f) {
            for (int i = 0; i < 5; ++i) {
                m_resistances[i] = 0.0f;
            }
        }
        
        void setResistance(DamageType type, float value) {
            int index = static_cast<int>(type);
            m_resistances[index] = std::max(0.0f, std::min(1.0f, value));
        }
        
        float getResistance(DamageType type) const {
            return m_resistances[static_cast<int>(type)];
        }
        
        void setFlatReduction(float value) {
            m_flatReduction = std::max(0.0f, value);
        }
        
        float getFlatReduction() const {
            return m_flatReduction;
        }
        
        void setShield(float value) {
            m_shield = std::max(0.0f, value);
        }
        
        float getShield() const {
            return m_shield;
        }
        
    private:
        float m_resistances[5];
        float m_flatReduction;
        float m_shield;
    };

    class HealthComponent {
    public:
        HealthComponent(float maxHealth = 100.0f, 
                    std::shared_ptr<Graphics::ParticleSystem> particleSystem = nullptr)
            : m_currentHealth(maxHealth), 
            m_maxHealth(maxHealth),
            m_position(0.0f),
            m_isAlive(true),
            m_particleSystem(particleSystem) {
            std::cout << "[HealthComponent] Created with max health: " << maxHealth << std::endl;
        }

        float applyDamage(const DamageInfo& damage) {
            if (!m_isAlive) return 0.0f;
            
            float actualDamage = damage.amount;
            
            // Apply damage reduction from resilience
            if (damage.type != DamageType::PSYCHIC) { // Psychic damage ignores armor
                float resistance = m_resilience.getResistance(damage.type);
                actualDamage = actualDamage * (1.0f - resistance);
                
                // Apply flat reduction
                actualDamage = std::max(0.0f, actualDamage - m_resilience.getFlatReduction());
            }
            
            m_currentHealth -= actualDamage;
            
            std::cout << "[HealthComponent] Applied " << actualDamage 
                    << " damage, health now: " << m_currentHealth << "/" << m_maxHealth << std::endl;
            
            if (m_currentHealth <= 0.0f) {
                m_currentHealth = 0.0f;
                m_isAlive = false;
                std::cout << "[HealthComponent] Entity died" << std::endl;
                if (m_deathCallback) m_deathCallback();
            }
            
            if (m_damageCallback) m_damageCallback(damage, actualDamage);
            
            return actualDamage;
        }

        float applyHealing(float amount, const glm::vec3& source = glm::vec3(0.0f)) {
            if (!m_isAlive || amount <= 0.0f) return 0.0f;
            
            float actualHealing = std::min(amount, m_maxHealth - m_currentHealth);
            m_currentHealth += actualHealing;
            
            std::cout << "[HealthComponent] Healed for " << actualHealing 
                    << ", health now: " << m_currentHealth << "/" << m_maxHealth << std::endl;
            
            return actualHealing;
        }

        void setPosition(const glm::vec3& position) { m_position = position; }
        glm::vec3 getPosition() const { return m_position; }
        float getCurrentHealth() const { return m_currentHealth; }
        float getMaxHealth() const { return m_maxHealth; }
        bool isAlive() const { return m_isAlive; }
        
        void setDamageCallback(std::function<void(const DamageInfo&, float)> callback) {
            m_damageCallback = callback;
        }
        
        void setDeathCallback(std::function<void()> callback) {
            m_deathCallback = callback;
        }
        
        Resilience& getResilience() { return m_resilience; }

    private:
        float m_currentHealth;
        float m_maxHealth;
        glm::vec3 m_position;
        bool m_isAlive;
        std::shared_ptr<Graphics::ParticleSystem> m_particleSystem;
        std::function<void(const DamageInfo&, float)> m_damageCallback;
        std::function<void()> m_deathCallback;
        Resilience m_resilience;
    };
}

// AIConstruct implementation
namespace AI {
    // Forward declarations
    class AIConstructBehavior;
    class AIConstruct;

    // Target descriptor
    struct TargetInfo {
        std::string id;
        glm::vec3 position;
        float threat;
        float distance;
        bool isVisible;
        
        TargetInfo(const std::string& id_ = "", 
                const glm::vec3& pos = glm::vec3(0.0f),
                float threat_ = 0.0f)
            : id(id_), position(pos), threat(threat_), 
            distance(0.0f), isVisible(true) {}
    };

    // Perception result
    struct PerceptionResult {
        std::vector<TargetInfo> visibleTargets;
        TargetInfo primaryTarget;
        glm::vec3 lastKnownThreatPosition;
        float alertLevel;
        bool heardSound;
        float soundIntensity;
        glm::vec3 soundSource;
        
        PerceptionResult() : alertLevel(0.0f), heardSound(false), 
                            soundIntensity(0.0f) {}
    };

    // Behavior types
    enum class BehaviorType {
        IDLE,
        PATROL,
        INVESTIGATE,
        ATTACK,
        DEFEND,
        FLEE,
        DAMAGED
    };

    // Base class for AI behaviors
    class AIConstructBehavior {
    public:
        AIConstructBehavior(AIConstruct* owner = nullptr) 
            : m_owner(owner), m_timeInState(0.0f) {}
        
        virtual ~AIConstructBehavior() = default;
        
        virtual void enter() {
            m_timeInState = 0.0f;
            std::cout << "[AIBehavior] Entered behavior: " << getName() << std::endl;
        }
        
        virtual void exit() {
            std::cout << "[AIBehavior] Exited behavior: " << getName() << std::endl;
        }
        
        virtual BehaviorType update(float deltaTime, const PerceptionResult& perception) = 0;
        virtual std::string getName() const = 0;
        
        void setOwner(AIConstruct* owner) { m_owner = owner; }
        AIConstruct* getOwner() const { return m_owner; }
        float getTimeInState() const { return m_timeInState; }
        
    protected:
        void incrementTime(float deltaTime) { m_timeInState += deltaTime; }
        float m_timeInState;
        
    private:
        AIConstruct* m_owner;
    };

    // Idle behavior
    class IdleBehavior : public AIConstructBehavior {
    public:
        using AIConstructBehavior::AIConstructBehavior;
        
        BehaviorType update(float deltaTime, const PerceptionResult& perception) override {
            incrementTime(deltaTime);
            
            // Check for threats
            if (!perception.visibleTargets.empty()) {
                std::cout << "[IdleBehavior] Detected " << perception.visibleTargets.size() 
                        << " targets, switching to ATTACK" << std::endl;
                return BehaviorType::ATTACK;
            }
            
            // Check for sounds
            if (perception.heardSound && perception.soundIntensity > 0.5f) {
                std::cout << "[IdleBehavior] Detected sound with intensity " 
                        << perception.soundIntensity << ", switching to INVESTIGATE" << std::endl;
                return BehaviorType::INVESTIGATE;
            }
            
            // Random chance to start patrolling
            if (m_timeInState > 5.0f && (rand() % 100) < 10) {
                std::cout << "[IdleBehavior] Starting patrol after " 
                        << m_timeInState << "s" << std::endl;
                return BehaviorType::PATROL;
            }
            
            return BehaviorType::IDLE;
        }
        
        std::string getName() const override { return "Idle"; }
    };

    // Patrol behavior
    class PatrolBehavior : public AIConstructBehavior {
    public:
        using AIConstructBehavior::AIConstructBehavior;
        
        void enter() override {
            AIConstructBehavior::enter();
            m_currentWaypointIndex = 0;
            m_waitingAtWaypoint = false;
            m_waitTime = 0.0f;
        }
        
        BehaviorType update(float deltaTime, const PerceptionResult& perception) override {
            incrementTime(deltaTime);
            
            // Check for threats
            if (!perception.visibleTargets.empty()) {
                std::cout << "[PatrolBehavior] Detected " << perception.visibleTargets.size() 
                        << " targets, switching to ATTACK" << std::endl;
                return BehaviorType::ATTACK;
            }
            
            // Check for sounds
            if (perception.heardSound && perception.soundIntensity > 0.7f) {
                std::cout << "[PatrolBehavior] Detected sound with intensity " 
                        << perception.soundIntensity << ", switching to INVESTIGATE" << std::endl;
                return BehaviorType::INVESTIGATE;
            }
            
            // Handle waypoint logic
            if (m_waitingAtWaypoint) {
                m_waitTime += deltaTime;
                if (m_waitTime >= m_waypointWaitDuration) {
                    m_waitingAtWaypoint = false;
                    m_currentWaypointIndex = (m_currentWaypointIndex + 1) % m_waypoints.size();
                    std::cout << "[PatrolBehavior] Moving to waypoint " 
                            << m_currentWaypointIndex << std::endl;
                }
            } else {
                // Simulate movement to waypoint
                std::cout << "[PatrolBehavior] Patrolling toward waypoint " 
                        << m_currentWaypointIndex << std::endl;
                
                // Simulate reaching waypoint
                m_waitingAtWaypoint = true;
                m_waitTime = 0.0f;
            }
            
            return BehaviorType::PATROL;
        }
        
        std::string getName() const override { return "Patrol"; }
        
        void setWaypoints(const std::vector<glm::vec3>& waypoints) {
            m_waypoints = waypoints;
            m_currentWaypointIndex = 0;
        }
        
    private:
        std::vector<glm::vec3> m_waypoints = {
            glm::vec3(0.0f, 0.0f, 0.0f),
            glm::vec3(10.0f, 0.0f, 0.0f),
            glm::vec3(10.0f, 0.0f, 10.0f),
            glm::vec3(0.0f, 0.0f, 10.0f)
        };
        int m_currentWaypointIndex = 0;
        bool m_waitingAtWaypoint = false;
        float m_waitTime = 0.0f;
        float m_waypointWaitDuration = 2.0f;
    };

    // Attack behavior
    class AttackBehavior : public AIConstructBehavior {
    public:
        using AIConstructBehavior::AIConstructBehavior;
        
        void enter() override {
            AIConstructBehavior::enter();
            m_attackCooldown = 0.0f;
        }
        
        BehaviorType update(float deltaTime, const PerceptionResult& perception) override {
            incrementTime(deltaTime);
            
            // If no targets, return to patrol
            if (perception.visibleTargets.empty()) {
                std::cout << "[AttackBehavior] No targets visible, switching to PATROL" << std::endl;
                return BehaviorType::PATROL;
            }
            
            // Update attack cooldown
            m_attackCooldown -= deltaTime;
            
            // Attack logic
            if (m_attackCooldown <= 0.0f) {
                const auto& target = perception.primaryTarget;
                std::cout << "[AttackBehavior] Attacking target " << target.id 
                        << " at distance " << target.distance << std::endl;
                
                m_attackCooldown = m_attackRate;
            }
            
            return BehaviorType::ATTACK;
        }
        
        std::string getName() const override { return "Attack"; }
        
        void setAttackRate(float rate) { m_attackRate = rate; }
        float getAttackRate() const { return m_attackRate; }
        
    private:
        float m_attackCooldown = 0.0f;
        float m_attackRate = 1.5f; // Attack every 1.5 seconds
    };

    // Investigate behavior
    class InvestigateBehavior : public AIConstructBehavior {
    public:
        using AIConstructBehavior::AIConstructBehavior;
        
        void enter() override {
            AIConstructBehavior::enter();
            m_investigationComplete = false;
        }
        
        BehaviorType update(float deltaTime, const PerceptionResult& perception) override {
            incrementTime(deltaTime);
            
            // Check for threats
            if (!perception.visibleTargets.empty()) {
                std::cout << "[InvestigateBehavior] Detected " << perception.visibleTargets.size() 
                        << " targets, switching to ATTACK" << std::endl;
                return BehaviorType::ATTACK;
            }
            
            // Simulate investigation
            std::cout << "[InvestigateBehavior] Investigating at position " 
                    << perception.soundSource.x << ", " 
                    << perception.soundSource.y << ", " 
                    << perception.soundSource.z << std::endl;
            
            // Complete investigation after some time
            if (m_timeInState > 5.0f) {
                m_investigationComplete = true;
                std::cout << "[InvestigateBehavior] Investigation complete, returning to PATROL" << std::endl;
                return BehaviorType::PATROL;
            }
            
            return BehaviorType::INVESTIGATE;
        }
        
        std::string getName() const override { return "Investigate"; }
        
    private:
        bool m_investigationComplete = false;
    };

    // Damaged behavior
    class DamagedBehavior : public AIConstructBehavior {
    public:
        using AIConstructBehavior::AIConstructBehavior;
        
        BehaviorType update(float deltaTime, const PerceptionResult& perception) override {
            incrementTime(deltaTime);
            
            // Staggered briefly
            if (m_timeInState > 0.5f) {
                if (!perception.visibleTargets.empty()) {
                    std::cout << "[DamagedBehavior] Recovered from damage, attacking threats" << std::endl;
                    return BehaviorType::ATTACK;
                } else {
                    std::cout << "[DamagedBehavior] Recovered from damage, returning to PATROL" << std::endl;
                    return BehaviorType::PATROL;
                }
            }
            
            return BehaviorType::DAMAGED;
        }
        
        std::string getName() const override { return "Damaged"; }
    };

    // Main AIConstruct class
    enum class ConstructType {
        SENTINEL,
        GUARDIAN,
        ASSASSIN,
        JUGGERNAUT
    };

    class AIConstruct {
    public:
        AIConstruct(ConstructType type = ConstructType::SENTINEL,
                  std::shared_ptr<Combat::HealthComponent> healthComponent = nullptr,
                  std::shared_ptr<Graphics::ParticleSystem> particleSystem = nullptr)
            : m_type(type),
            m_healthComponent(healthComponent),
            m_particleSystem(particleSystem),
            m_position(0.0f),
            m_rotation(0.0f),
            m_currentBehaviorType(BehaviorType::IDLE),
            m_alertLevel(0.0f) {
            
            // Create health component if not provided
            if (!m_healthComponent) {
                m_healthComponent = std::make_shared<Combat::HealthComponent>(
                    getMaxHealthForType(type), particleSystem);
            }
            
            // Set up damage callback
            m_healthComponent->setDamageCallback(
                [this](const Combat::DamageInfo& damage, float actualDamage) {
                    this->onDamaged(damage, actualDamage);
                });
            
            // Set up death callback
            m_healthComponent->setDeathCallback(
                [this]() {
                    this->onDeath();
                });
            
            // Initialize behaviors
            m_behaviors[BehaviorType::IDLE] = std::make_unique<IdleBehavior>(this);
            m_behaviors[BehaviorType::PATROL] = std::make_unique<PatrolBehavior>(this);
            m_behaviors[BehaviorType::INVESTIGATE] = std::make_unique<InvestigateBehavior>(this);
            m_behaviors[BehaviorType::ATTACK] = std::make_unique<AttackBehavior>(this);
            m_behaviors[BehaviorType::DAMAGED] = std::make_unique<DamagedBehavior>(this);
            
            // Customize behaviors based on type
            configureForType(type);
            
            // Start in idle behavior
            changeBehavior(BehaviorType::IDLE);
            
            std::cout << "[AIConstruct] Created " << getTypeName() 
                    << " with " << m_healthComponent->getMaxHealth() << " health" << std::endl;
        }
        
        void update(float deltaTime) {
            if (!m_healthComponent->isAlive()) {
                return;
            }
            
            // Update perception
            PerceptionResult perception = perceiveEnvironment();
            
            // Update current behavior
            auto currentBehavior = m_behaviors[m_currentBehaviorType].get();
            if (currentBehavior) {
                BehaviorType nextBehavior = currentBehavior->update(deltaTime, perception);
                
                // Change behavior if needed
                if (nextBehavior != m_currentBehaviorType) {
                    changeBehavior(nextBehavior);
                }
            }
        }
        
        void onDamaged(const Combat::DamageInfo& damage, float actualDamage) {
            std::cout << "[AIConstruct] " << getTypeName() << " damaged for " 
                    << actualDamage << " points from " << damage.source.x << ", " 
                    << damage.source.y << ", " << damage.source.z << std::endl;
            
            // Increase alert level
            m_alertLevel = std::min(1.0f, m_alertLevel + 0.3f);
            
            // Set last known threat position
            m_lastKnownThreatPosition = damage.source;
            
            // Switch to damaged behavior
            changeBehavior(BehaviorType::DAMAGED);
        }
        
        void onDeath() {
            std::cout << "[AIConstruct] " << getTypeName() << " destroyed!" << std::endl;
            
            // Create death particles if particle system exists
            if (m_particleSystem) {
                m_particleSystem->emitBurst(
                    m_position, 50, 2.0f, 8.0f, 
                    glm::vec4(0.8f, 0.1f, 0.1f, 1.0f), 0.5f, 2.0f);
            }
        }
        
        PerceptionResult perceiveEnvironment() {
            PerceptionResult result;
            
            // Set alert level
            result.alertLevel = m_alertLevel;
            
            // Set last known threat position
            result.lastKnownThreatPosition = m_lastKnownThreatPosition;
            
            // Add any simulated targets to perception
            for (const auto& target : m_simulatedTargets) {
                result.visibleTargets.push_back(target);
            }
            
            // Set primary target if any targets exist
            if (!result.visibleTargets.empty()) {
                result.primaryTarget = *std::max_element(
                    result.visibleTargets.begin(), result.visibleTargets.end(),
                    [](const TargetInfo& a, const TargetInfo& b) {
                        return a.threat < b.threat;
                    });
            }
            
            // Add sound info
            result.heardSound = m_simulatedSoundHeard;
            result.soundIntensity = m_simulatedSoundIntensity;
            result.soundSource = m_simulatedSoundSource;
            
            return result;
        }
        
        void setPosition(const glm::vec3& position) {
            m_position = position;
            m_healthComponent->setPosition(position);
        }
        
        glm::vec3 getPosition() const { return m_position; }
        
        void setRotation(float rotation) { m_rotation = rotation; }
        float getRotation() const { return m_rotation; }
        
        ConstructType getType() const { return m_type; }
        std::string getTypeName() const {
            switch (m_type) {
                case ConstructType::SENTINEL: return "Sentinel";
                case ConstructType::GUARDIAN: return "Guardian";
                case ConstructType::ASSASSIN: return "Assassin";
                case ConstructType::JUGGERNAUT: return "Juggernaut";
                default: return "Unknown";
            }
        }
        
        BehaviorType getCurrentBehaviorType() const { return m_currentBehaviorType; }
        
        std::string getCurrentBehaviorName() const {
            auto it = m_behaviors.find(m_currentBehaviorType);
            if (it != m_behaviors.end() && it->second) {
                return it->second->getName();
            }
            return "Unknown";
        }
        
        std::shared_ptr<Combat::HealthComponent> getHealthComponent() const {
            return m_healthComponent;
        }
        
        float getAlertLevel() const { return m_alertLevel; }
        void setAlertLevel(float level) { m_alertLevel = std::max(0.0f, std::min(1.0f, level)); }
        
        // Test-specific methods to simulate environment
        void simulateTarget(const TargetInfo& target) {
            m_simulatedTargets.push_back(target);
            
            // Update distances
            for (auto& t : m_simulatedTargets) {
                t.distance = glm::distance(m_position, t.position);
            }
        }
        
        void clearSimulatedTargets() {
            m_simulatedTargets.clear();
        }
        
        void simulateSound(float intensity, const glm::vec3& source) {
            m_simulatedSoundHeard = true;
            m_simulatedSoundIntensity = intensity;
            m_simulatedSoundSource = source;
        }
        
        void clearSimulatedSound() {
            m_simulatedSoundHeard = false;
            m_simulatedSoundIntensity = 0.0f;
        }
        
    private:
        void changeBehavior(BehaviorType newBehavior) {
            if (m_behaviors.count(m_currentBehaviorType) > 0) {
                m_behaviors[m_currentBehaviorType]->exit();
            }
            
            m_currentBehaviorType = newBehavior;
            
            if (m_behaviors.count(m_currentBehaviorType) > 0) {
                m_behaviors[m_currentBehaviorType]->enter();
            }
        }
        
        float getMaxHealthForType(ConstructType type) {
            switch (type) {
                case ConstructType::SENTINEL: return 100.0f;
                case ConstructType::GUARDIAN: return 200.0f;
                case ConstructType::ASSASSIN: return 80.0f;
                case ConstructType::JUGGERNAUT: return 300.0f;
                default: return 100.0f;
            }
        }
        
        void configureForType(ConstructType type) {
            // Configure attack rate based on construct type
            auto attackBehavior = dynamic_cast<AttackBehavior*>(m_behaviors[BehaviorType::ATTACK].get());
            if (attackBehavior) {
                switch (type) {
                    case ConstructType::SENTINEL:
                        attackBehavior->setAttackRate(1.5f);
                        break;
                    case ConstructType::GUARDIAN:
                        attackBehavior->setAttackRate(2.0f);
                        break;
                    case ConstructType::ASSASSIN:
                        attackBehavior->setAttackRate(0.8f);
                        break;
                    case ConstructType::JUGGERNAUT:
                        attackBehavior->setAttackRate(3.0f);
                        break;
                }
            }
            
            // Configure resistances based on type
            auto& resilience = m_healthComponent->getResilience();
            switch (type) {
                case ConstructType::SENTINEL:
                    // Balanced resistances
                    resilience.setResistance(Combat::DamageType::PHYSICAL, 0.2f);
                    resilience.setResistance(Combat::DamageType::PLASMA, 0.2f);
                    break;
                case ConstructType::GUARDIAN:
                    // High physical resistance
                    resilience.setResistance(Combat::DamageType::PHYSICAL, 0.5f);
                    resilience.setResistance(Combat::DamageType::PLASMA, 0.1f);
                    break;
                case ConstructType::ASSASSIN:
                    // Low resistances but fast
                    resilience.setResistance(Combat::DamageType::PHYSICAL, 0.1f);
                    resilience.setResistance(Combat::DamageType::PLASMA, 0.3f);
                    break;
                case ConstructType::JUGGERNAUT:
                    // Very high physical resistance
                    resilience.setResistance(Combat::DamageType::PHYSICAL, 0.7f);
                    resilience.setFlatReduction(5.0f);
                    break;
            }
        }
        
        ConstructType m_type;
        std::shared_ptr<Combat::HealthComponent> m_healthComponent;
        std::shared_ptr<Graphics::ParticleSystem> m_particleSystem;
        
        glm::vec3 m_position;
        float m_rotation;
        
        std::unordered_map<BehaviorType, std::unique_ptr<AIConstructBehavior>> m_behaviors;
        BehaviorType m_currentBehaviorType;
        
        float m_alertLevel;
        glm::vec3 m_lastKnownThreatPosition;
        
        // Simulated environment for testing
        std::vector<TargetInfo> m_simulatedTargets;
        bool m_simulatedSoundHeard = false;
        float m_simulatedSoundIntensity = 0.0f;
        glm::vec3 m_simulatedSoundSource;
    };
}

} // namespace TurtleEngine

// Main test function
int main() {
    std::cout << "=== AIConstruct Test ===\n";

    // Create a particle system
    auto particleSystem = std::make_shared<TurtleEngine::Graphics::ParticleSystem>();

    // Test 1: Create different construct types
    std::cout << "\nTest 1: Creating different construct types\n";
    
    auto sentinel = std::make_shared<TurtleEngine::AI::AIConstruct>(
        TurtleEngine::AI::ConstructType::SENTINEL, nullptr, particleSystem);
    
    auto guardian = std::make_shared<TurtleEngine::AI::AIConstruct>(
        TurtleEngine::AI::ConstructType::GUARDIAN, nullptr, particleSystem);
    
    auto assassin = std::make_shared<TurtleEngine::AI::AIConstruct>(
        TurtleEngine::AI::ConstructType::ASSASSIN, nullptr, particleSystem);
    
    auto juggernaut = std::make_shared<TurtleEngine::AI::AIConstruct>(
        TurtleEngine::AI::ConstructType::JUGGERNAUT, nullptr, particleSystem);
    
    bool typesSuccess = (
        sentinel->getType() == TurtleEngine::AI::ConstructType::SENTINEL &&
        guardian->getType() == TurtleEngine::AI::ConstructType::GUARDIAN &&
        assassin->getType() == TurtleEngine::AI::ConstructType::ASSASSIN &&
        juggernaut->getType() == TurtleEngine::AI::ConstructType::JUGGERNAUT
    );
    
    std::cout << "Constructor Test: " << (typesSuccess ? "PASSED" : "FAILED") << std::endl;

    // Test 2: State transitions (Idle -> Patrol -> Attack)
    std::cout << "\nTest 2: Testing state transitions\n";
    
    auto construct = sentinel;
    
    // Initial state should be IDLE
    bool initialStateSuccess = (construct->getCurrentBehaviorType() == TurtleEngine::AI::BehaviorType::IDLE);
    std::cout << "Initial state: " << construct->getCurrentBehaviorName() << std::endl;
    
    // Update a few times, should transition to PATROL eventually
    bool patrolTransitionSuccess = false;
    for (int i = 0; i < 100 && !patrolTransitionSuccess; ++i) {
        construct->update(0.1f);
        if (construct->getCurrentBehaviorType() == TurtleEngine::AI::BehaviorType::PATROL) {
            patrolTransitionSuccess = true;
        }
    }
    
    std::cout << "State after updates: " << construct->getCurrentBehaviorName() << std::endl;
    
    // Simulate a target, should transition to ATTACK
    TurtleEngine::AI::TargetInfo target("Target1", glm::vec3(10.0f, 0.0f, 0.0f), 0.8f);
    construct->simulateTarget(target);
    construct->update(0.1f);
    
    bool attackTransitionSuccess = (construct->getCurrentBehaviorType() == TurtleEngine::AI::BehaviorType::ATTACK);
    std::cout << "State after target detected: " << construct->getCurrentBehaviorName() << std::endl;
    
    // Clear targets, should go back to PATROL
    construct->clearSimulatedTargets();
    construct->update(0.1f);
    
    bool patrolReturnSuccess = (construct->getCurrentBehaviorType() == TurtleEngine::AI::BehaviorType::PATROL);
    std::cout << "State after target lost: " << construct->getCurrentBehaviorName() << std::endl;
    
    bool stateTransitionSuccess = initialStateSuccess && patrolTransitionSuccess && 
                                attackTransitionSuccess && patrolReturnSuccess;
    std::cout << "State Transition Test: " << (stateTransitionSuccess ? "PASSED" : "FAILED") << std::endl;

    // Test 3: Damage response
    std::cout << "\nTest 3: Testing damage response\n";
    
    // Apply damage to the construct
    auto health = construct->getHealthComponent();
    TurtleEngine::Combat::DamageInfo damageInfo(20.0f, TurtleEngine::Combat::DamageType::PLASMA,
                                             glm::vec3(5.0f, 0.0f, 0.0f));
    health->applyDamage(damageInfo);
    
    // Should switch to DAMAGED behavior
    bool damagedStateSuccess = (construct->getCurrentBehaviorType() == TurtleEngine::AI::BehaviorType::DAMAGED);
    std::cout << "State after taking damage: " << construct->getCurrentBehaviorName() << std::endl;
    
    // After a short time, should recover
    for (int i = 0; i < 10; ++i) {
        construct->update(0.1f);
    }
    
    bool recoverySuccess = (construct->getCurrentBehaviorType() != TurtleEngine::AI::BehaviorType::DAMAGED);
    std::cout << "State after recovery: " << construct->getCurrentBehaviorName() << std::endl;
    
    bool damageResponseSuccess = damagedStateSuccess && recoverySuccess;
    std::cout << "Damage Response Test: " << (damageResponseSuccess ? "PASSED" : "FAILED") << std::endl;

    // Test 4: Type-specific behaviors
    std::cout << "\nTest 4: Testing type-specific behaviors\n";
    
    // Test Juggernaut's high resistance
    std::cout << "Testing Juggernaut's high physical resistance..." << std::endl;
    TurtleEngine::Combat::DamageInfo physicalDamage(50.0f, TurtleEngine::Combat::DamageType::PHYSICAL,
                                                 glm::vec3(0.0f));
    
    float juggernautInitialHealth = juggernaut->getHealthComponent()->getCurrentHealth();
    juggernaut->getHealthComponent()->applyDamage(physicalDamage);
    float juggernautDamageTaken = juggernautInitialHealth - 
                                juggernaut->getHealthComponent()->getCurrentHealth();
    
    float sentinelInitialHealth = sentinel->getHealthComponent()->getCurrentHealth();
    sentinel->getHealthComponent()->applyDamage(physicalDamage);
    float sentinelDamageTaken = sentinelInitialHealth - 
                             sentinel->getHealthComponent()->getCurrentHealth();
    
    // Juggernaut should take less damage due to higher resistance
    bool resistanceSuccess = (juggernautDamageTaken < sentinelDamageTaken);
    std::cout << "Juggernaut took " << juggernautDamageTaken << " damage, Sentinel took " 
            << sentinelDamageTaken << " damage" << std::endl;
    std::cout << "Resistance Test: " << (resistanceSuccess ? "PASSED" : "FAILED") << std::endl;

    std::cout << "\n=== AIConstruct Test Complete ===\n";
    return (typesSuccess && stateTransitionSuccess && damageResponseSuccess && resistanceSuccess) ? 0 : 1;
} 