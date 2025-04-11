/**
 * Simple Temporal Anomaly Demo
 * 
 * A standalone simulation that demonstrates the core concepts of temporal anomalies
 * affecting entities in a 2D space. This simplified version doesn't require any
 * external dependencies and works with standard C++17.
 * 
 * Features:
 * - Six types of temporal anomalies (RIFT, STASIS, DILATION, ACCELERATION, REVERSAL, INSTABILITY)
 * - Entity movement with velocity and time-based physics
 * - ASCII visualization with ANSI colors
 * - Interactive controls: Press ENTER to continue, 'q' to quit, 'p' to pause/resume
 * 
 * Usage: Compile with g++ -std=c++17 simple_temporal_demo.cpp -o simple_temporal_demo.exe
 */

#include <iostream>
#include <vector>
#include <string>
#include <cmath>
#include <random>
#include <algorithm>
#include <memory>
#include <functional>
#include <ctime>
#include <limits>

// ANSI color codes
namespace Colors {
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string BOLD = "\033[1m";
    const std::string BRIGHT_RED = "\033[91m";
    const std::string BRIGHT_GREEN = "\033[92m";
    const std::string BRIGHT_BLUE = "\033[94m";
    const std::string BRIGHT_CYAN = "\033[96m";
}

// Simple 3D vector implementation
struct Vec3 {
    float x, y, z;
    
    Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}
    
    Vec3 operator+(const Vec3& other) const {
        return Vec3(x + other.x, y + other.y, z + other.z);
    }
    
    Vec3 operator-(const Vec3& other) const {
        return Vec3(x - other.x, y - other.y, z - other.z);
    }
    
    Vec3 operator*(float scalar) const {
        return Vec3(x * scalar, y * scalar, z * scalar);
    }
    
    float distance(const Vec3& other) const {
        float dx = x - other.x;
        float dy = y - other.y;
        float dz = z - other.z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};

// Anomaly types
enum AnomalyType {
    RIFT = 0,
    STASIS = 1,
    DILATION = 2,
    ACCELERATION = 3,
    REVERSAL = 4,
    INSTABILITY = 5
};

// Name mapping for anomaly types
const std::string AnomalyTypeNames[] = {
    "Rift",
    "Stasis",
    "Dilation",
    "Acceleration",
    "Reversal",
    "Instability"
};

// Color mapping for anomaly types
const std::string AnomalyTypeColors[] = {
    Colors::BRIGHT_CYAN,     // Rift
    Colors::BRIGHT_BLUE,     // Stasis
    Colors::GREEN,           // Dilation
    Colors::YELLOW,          // Acceleration
    Colors::BRIGHT_RED,      // Reversal
    Colors::MAGENTA          // Instability
};

// Character mapping for anomaly visualization
const char AnomalyTypeChars[] = {
    'R',    // Rift
    'S',    // Stasis
    'D',    // Dilation
    'A',    // Acceleration
    'V',    // Reversal (V for reverse)
    'I'     // Instability
};

// Anomaly effect structure
struct AnomalyEffect {
    AnomalyType type;
    float timeDistortion;
    Vec3 position;
    float radius;
    float duration;
    int sourceId;
    
    AnomalyEffect() : type(STASIS), timeDistortion(0.0f), radius(0.0f), duration(0.0f), sourceId(-1) {}
    
    AnomalyEffect(AnomalyType type, float distortion, const Vec3& pos, float radius, float duration)
        : type(type), timeDistortion(distortion), position(pos), radius(radius), duration(duration), sourceId(-1) {}
};

// Forward declarations
class Entity;

// Temporal anomaly class
class TemporalAnomaly {
private:
    AnomalyEffect effect;
    bool active;
    
public:
    TemporalAnomaly(const AnomalyEffect& effect) : effect(effect), active(true) {}
    
    void update(float deltaTime) {
        effect.duration -= deltaTime;
        if (effect.duration <= 0.0f) {
            active = false;
        }
    }
    
    bool isActive() const { return active; }
    
    const AnomalyEffect& getEffect() const { return effect; }
    
    bool isAffecting(const Vec3& position) const {
        return position.distance(effect.position) <= effect.radius;
    }
};

// Entity base class
class Entity {
protected:
    int id;
    std::string name;
    std::string color;
    Vec3 position;
    Vec3 velocity;
    float baseSpeed;
    float timeModifier;
    bool isReversing;
    bool isStasized;
    std::vector<AnomalyEffect> activeEffects;
    
public:
    Entity(int id, const std::string& name, const std::string& color, const Vec3& pos, const Vec3& vel, float speed)
        : id(id), name(name), color(color), position(pos), velocity(vel), baseSpeed(speed),
          timeModifier(1.0f), isReversing(false), isStasized(false) {}
    
    virtual ~Entity() {}
    
    const Vec3& getPosition() const { return position; }
    int getId() const { return id; }
    const std::string& getName() const { return name; }
    const std::string& getColor() const { return color; }
    
    // Apply an anomaly effect to this entity with improved combination logic
    void applyAnomalyEffect(const AnomalyEffect& effect) {
        // Add effect to active effects
        activeEffects.push_back(effect);
        
        // Reset state before applying combined effects
        isStasized = false;
        isReversing = false;
        float combinedTimeModifier = 1.0f;
        
        // Count effects by type for better combination handling
        int stasisCount = 0;
        int reversalCount = 0;
        float dilationSum = 0.0f;
        int dilationCount = 0;
        float accelerationSum = 0.0f;
        int accelerationCount = 0;
        
        // Process all active effects to calculate combined values
        for (const auto& e : activeEffects) {
            switch (e.type) {
                case STASIS:
                    stasisCount++;
                    break;
                    
                case DILATION:
                    dilationSum += e.timeDistortion;
                    dilationCount++;
                    break;
                    
                case ACCELERATION:
                    accelerationSum += e.timeDistortion;
                    accelerationCount++;
                    break;
                    
                case REVERSAL:
                    reversalCount++;
                    break;
                    
                case RIFT:
                    // For Rift, we apply immediate position changes
                    position.x += (std::rand() % 10 - 5) * e.timeDistortion;
                    position.y += (std::rand() % 10 - 5) * e.timeDistortion;
                    break;
                    
                case INSTABILITY:
                    // For Instability, we apply a random time modifier
                    combinedTimeModifier *= ((float)(std::rand() % 200) / 100.0f) * e.timeDistortion;
                    break;
            }
        }
        
        // Apply combined effects in priority order
        if (stasisCount > 0) {
            // Stasis takes precedence - entity stops completely
            isStasized = true;
            timeModifier = 0.0f;
        } else {
            // Apply dilation effects (slowdown)
            if (dilationCount > 0) {
                combinedTimeModifier *= (dilationSum / dilationCount);
            }
            
            // Apply acceleration effects (speedup)
            if (accelerationCount > 0) {
                combinedTimeModifier *= (accelerationSum / accelerationCount);
            }
            
            // Apply reversal if present
            if (reversalCount > 0) {
                isReversing = true;
                combinedTimeModifier *= -1.0f;
            }
            
            // Set the final time modifier
            timeModifier = combinedTimeModifier;
        }
    }
    
    void update(float deltaTime) {
        // Remove expired effects
        activeEffects.erase(
            std::remove_if(activeEffects.begin(), activeEffects.end(),
                [](const AnomalyEffect& effect) { return effect.duration <= 0.0f; }),
            activeEffects.end()
        );
        
        // Reset state if no active effects
        if (activeEffects.empty()) {
            timeModifier = 1.0f;
            isReversing = false;
            isStasized = false;
        }
        
        // Apply movement based on time modifier
        if (!isStasized) {
            if (isReversing) {
                position = position - velocity * baseSpeed * std::abs(timeModifier) * deltaTime;
            } else {
                position = position + velocity * baseSpeed * timeModifier * deltaTime;
            }
            
            // Boundary checking
            const float BOUNDARY = 40.0f;
            if (position.x < -BOUNDARY) {
                position.x = -BOUNDARY;
                velocity.x *= -1;
            }
            if (position.x > BOUNDARY) {
                position.x = BOUNDARY;
                velocity.x *= -1;
            }
            if (position.y < -BOUNDARY) {
                position.y = -BOUNDARY;
                velocity.y *= -1;
            }
            if (position.y > BOUNDARY) {
                position.y = BOUNDARY;
                velocity.y *= -1;
            }
        }
        
        // Update effect durations
        for (auto& effect : activeEffects) {
            effect.duration -= deltaTime;
        }
    }
    
    std::string getStatus() const {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%s%s (%d)%s | Pos: (%.1f, %.1f) | Speed: %.2f",
                 color.c_str(), name.c_str(), id, Colors::RESET.c_str(),
                 position.x, position.y, baseSpeed * timeModifier);
        
        std::string status = buffer;
        
        if (!activeEffects.empty()) {
            status += " | Effects: ";
            for (const auto& effect : activeEffects) {
                char effectBuffer[64];
                snprintf(effectBuffer, sizeof(effectBuffer), "%s%s(%.1fs)%s ",
                         AnomalyTypeColors[effect.type].c_str(),
                         AnomalyTypeNames[effect.type].c_str(),
                         effect.duration, Colors::RESET.c_str());
                status += effectBuffer;
            }
        }
        
        return status;
    }
    
    std::string getVisualChar() const {
        if (activeEffects.empty()) {
            return color + name.substr(0, 1) + Colors::RESET;
        } else {
            // Use the character representation of the most recent anomaly effect
            return AnomalyTypeColors[activeEffects.back().type] + 
                   name.substr(0, 1) + std::to_string(id) + 
                   Colors::RESET;
        }
    }
};

// World visualizer
class WorldVisualizer {
private:
    int width;
    int height;
    float scale;
    std::vector<std::tuple<Vec3, float, int>> anomalyCircles; // position, radius, type
    
public:
    WorldVisualizer(int width, int height, float scale)
        : width(width), height(height), scale(scale) {}
    
    void addAnomaly(const Vec3& position, float radius, int type) {
        anomalyCircles.emplace_back(position, radius, type);
    }
    
    void clearAnomalies() {
        anomalyCircles.clear();
    }
    
    void visualize(const std::vector<std::shared_ptr<Entity>>& entities) {
        // Create a grid
        std::vector<std::vector<std::string>> grid(height, std::vector<std::string>(width, " "));
        
        // Draw anomaly circles
        for (const auto& anomaly : anomalyCircles) {
            Vec3 pos = std::get<0>(anomaly);
            float radius = std::get<1>(anomaly);
            int type = std::get<2>(anomaly);
            drawCircle(grid, pos, radius, AnomalyTypeColors[type], AnomalyTypeChars[type]);
        }
        
        // Place entities on the grid
        for (const auto& entity : entities) {
            const Vec3& pos = entity->getPosition();
            int gridX = static_cast<int>((pos.x / scale) + (width / 2));
            int gridY = static_cast<int>((pos.y / scale) + (height / 2));
            
            // Bounds checking
            if (gridX >= 0 && gridX < width && gridY >= 0 && gridY < height) {
                grid[gridY][gridX] = entity->getVisualChar();
            }
        }
        
        // Clear screen
        std::cout << "\033[2J\033[H";
        
        // Print the grid
        std::cout << Colors::BOLD << "Simple Temporal Anomaly Simulation" << Colors::RESET << std::endl;
        std::cout << std::string(width + 2, '-') << std::endl;
        
        for (int y = 0; y < height; ++y) {
            std::cout << "|";
            for (int x = 0; x < width; ++x) {
                std::cout << grid[y][x];
            }
            std::cout << "|" << std::endl;
        }
        
        std::cout << std::string(width + 2, '-') << std::endl;
        
        // Print entity status
        std::cout << Colors::BOLD << "Entity Status:" << Colors::RESET << std::endl;
        for (const auto& entity : entities) {
            std::cout << "  " << entity->getStatus() << std::endl;
        }
        
        // Print legend
        std::cout << Colors::BOLD << "Anomaly Types:" << Colors::RESET << std::endl;
        for (int i = 0; i < 6; ++i) {
            std::cout << "  " << AnomalyTypeColors[i] << AnomalyTypeNames[i] << Colors::RESET << " ";
        }
        std::cout << std::endl;
        
        // Print controls
        std::cout << Colors::BOLD << "Controls:" << Colors::RESET << std::endl;
        std::cout << "  Press [Enter] to step, [p] to pause/resume, [q] to quit" << std::endl;
    }
    
private:
    void drawCircle(std::vector<std::vector<std::string>>& grid, const Vec3& center, float radius, 
                    const std::string& color, char typeChar) {
        int centerX = static_cast<int>((center.x / scale) + (width / 2));
        int centerY = static_cast<int>((center.y / scale) + (height / 2));
        int radiusGrid = static_cast<int>(radius / scale);
        
        for (int y = std::max(0, centerY - radiusGrid); y < std::min(height, centerY + radiusGrid + 1); ++y) {
            for (int x = std::max(0, centerX - radiusGrid); x < std::min(width, centerX + radiusGrid + 1); ++x) {
                if (x < 0 || x >= width || y < 0 || y >= height) continue;
                
                float distance = std::sqrt(std::pow(x - centerX, 2) + std::pow(y - centerY, 2));
                if (distance <= radiusGrid && grid[y][x] == " ") {
                    if (x == centerX && y == centerY) {
                        // Center of anomaly shows type character
                        grid[y][x] = color + typeChar + Colors::RESET;
                    } else if (distance >= radiusGrid - 1) {
                        // Edge of anomaly
                        grid[y][x] = color + "°" + Colors::RESET;
                    } else if (distance >= radiusGrid - 2) {
                        // Inner edge
                        grid[y][x] = color + "·" + Colors::RESET;
                    } else {
                        // Interior
                        grid[y][x] = color + " " + Colors::RESET;
                    }
                }
            }
        }
    }
};

// Simple delay function to replace thread-based sleep
void delay(int ms) {
    clock_t start = clock();
    while (clock() < start + ms * CLOCKS_PER_SEC / 1000);
}

// Temporal anomaly system
class TemporalAnomalySystem {
private:
    std::vector<std::shared_ptr<TemporalAnomaly>> anomalies;
    std::vector<std::shared_ptr<Entity>> entities;
    std::function<void(const TemporalAnomaly*)> anomalyCreatedCallback;
    std::function<void()> anomalyRemovedCallback;
    
public:
    TemporalAnomalySystem() {
        std::cout << "[TemporalAnomalySystem] Initialized" << std::endl;
    }
    
    void registerEntity(const std::shared_ptr<Entity>& entity) {
        if (!entity) {
            std::cerr << "[ERROR] Cannot register null entity" << std::endl;
            return;
        }
        
        entities.push_back(entity);
        std::cout << "[TemporalAnomalySystem] Registered entity: " << entity->getName() 
                  << ", total entities: " << entities.size() << std::endl;
    }
    
    void createAnomaly(const AnomalyEffect& effect) {
        // Validate anomaly effect parameters
        if (effect.radius <= 0.0f) {
            std::cerr << "[ERROR] Invalid anomaly radius: " << effect.radius << std::endl;
            return;
        }
        
        if (effect.duration <= 0.0f) {
            std::cerr << "[ERROR] Invalid anomaly duration: " << effect.duration << std::endl;
            return;
        }
        
        if (std::isnan(effect.timeDistortion) || std::isinf(effect.timeDistortion) || 
            std::abs(effect.timeDistortion) > 100.0f) {
            std::cerr << "[ERROR] Invalid time distortion: " << effect.timeDistortion 
                      << " (must be finite and |value| <= 100)" << std::endl;
            return;
        }
        
        // Create and add the anomaly
        auto anomaly = std::make_shared<TemporalAnomaly>(effect);
        anomalies.push_back(anomaly);
        
        std::cout << "[TemporalAnomalySystem] Created new " << AnomalyTypeNames[effect.type] 
                  << " anomaly at (" << effect.position.x << ", " << effect.position.y << "), radius: " 
                  << effect.radius << ", distortion: " << effect.timeDistortion 
                  << ", total: " << anomalies.size() << std::endl;
        
        if (anomalyCreatedCallback) {
            anomalyCreatedCallback(anomaly.get());
        }
    }
    
    void update(float deltaTime) {
        // Update anomalies and remove inactive ones
        size_t initialSize = anomalies.size();
        anomalies.erase(
            std::remove_if(anomalies.begin(), anomalies.end(),
                [deltaTime](std::shared_ptr<TemporalAnomaly>& anomaly) {
                    anomaly->update(deltaTime);
                    return !anomaly->isActive();
                }),
            anomalies.end()
        );
        
        // Notify if anomalies were removed
        if (anomalies.size() < initialSize && anomalyRemovedCallback) {
            anomalyRemovedCallback();
        }
        
        // Apply anomaly effects to entities
        for (const auto& entity : entities) {
            for (const auto& anomaly : anomalies) {
                if (anomaly->isAffecting(entity->getPosition())) {
                    entity->applyAnomalyEffect(anomaly->getEffect());
                }
            }
        }
    }
    
    const std::vector<std::shared_ptr<TemporalAnomaly>>& getAnomalies() const {
        return anomalies;
    }
    
    void setAnomalyCreatedCallback(const std::function<void(const TemporalAnomaly*)>& callback) {
        anomalyCreatedCallback = callback;
    }
    
    void setAnomalyRemovedCallback(const std::function<void()>& callback) {
        anomalyRemovedCallback = callback;
    }
};

// Simulation class
class TemporalSimulation {
private:
    TemporalAnomalySystem anomalySystem;
    std::vector<std::shared_ptr<Entity>> entities;
    WorldVisualizer visualizer;
    std::mt19937 rng;
    float timeStep;
    float simulationTime;
    bool isPaused;
    
    // Callbacks
    void onAnomalyCreated(const TemporalAnomaly* anomaly) {
        const auto& effect = anomaly->getEffect();
        visualizer.addAnomaly(effect.position, effect.radius, effect.type);
    }
    
    void onAnomalyRemoved() {
        visualizer.clearAnomalies();
        for (const auto& anomaly : anomalySystem.getAnomalies()) {
            const auto& effect = anomaly->getEffect();
            visualizer.addAnomaly(effect.position, effect.radius, effect.type);
        }
    }
    
public:
    TemporalSimulation() 
        : visualizer(60, 20, 1.0f), 
          rng(std::random_device{}()),
          timeStep(0.25f),
          simulationTime(0.0f),
          isPaused(false) {
        
        // Set callbacks
        anomalySystem.setAnomalyCreatedCallback(
            std::bind(&TemporalSimulation::onAnomalyCreated, this, std::placeholders::_1)
        );
        
        anomalySystem.setAnomalyRemovedCallback(
            std::bind(&TemporalSimulation::onAnomalyRemoved, this)
        );
    }
    
    void addEntity(const std::shared_ptr<Entity>& entity) {
        entities.push_back(entity);
        anomalySystem.registerEntity(entity);
    }
    
    void createAnomaly(const Vec3& position, float radius, AnomalyType type, float timeDistortion, float duration) {
        AnomalyEffect effect(type, timeDistortion, position, radius, duration);
        anomalySystem.createAnomaly(effect);
    }
    
    void setupScene() {
        // Create entities
        addEntity(std::make_shared<Entity>(1, "Alpha", Colors::RED, Vec3(-20, -10, 0), Vec3(1, 0.5, 0), 5.0f));
        addEntity(std::make_shared<Entity>(2, "Beta", Colors::GREEN, Vec3(15, 5, 0), Vec3(-0.5, 0.1, 0), 4.0f));
        addEntity(std::make_shared<Entity>(3, "Gamma", Colors::BLUE, Vec3(0, 20, 0), Vec3(0.2, -1, 0), 6.0f));
        addEntity(std::make_shared<Entity>(4, "Delta", Colors::YELLOW, Vec3(-15, 15, 0), Vec3(0.7, -0.7, 0), 3.5f));
    }
    
    void run() {
        std::cout << Colors::BOLD << Colors::BRIGHT_CYAN 
                  << "Starting Simple Temporal Anomaly Simulation" 
                  << Colors::RESET << std::endl;
        
        std::cout << "Press Enter to start simulation...";
        std::cin.get();
        
        int step = 0;
        const int MAX_STEPS = 100;  // Limit to prevent infinite loops
        
        // Test invalid anomaly creation
        std::cout << "Testing invalid anomaly creation..." << std::endl;
        createAnomaly(Vec3(0, 0, 0), 5.0f, ACCELERATION, std::numeric_limits<float>::quiet_NaN(), 10.0f);
        createAnomaly(Vec3(0, 0, 0), 5.0f, DILATION, 200.0f, 10.0f);
        std::cout << "Active anomalies after invalid attempts: " << anomalySystem.getAnomalies().size() << std::endl;
        
        // Create initial anomalies
        createAnomaly(Vec3(-10, 0, 0), 10.0f, STASIS, 0.0f, 10.0f);
        createAnomaly(Vec3(10, 10, 0), 8.0f, DILATION, 0.5f, 15.0f);
        
        // Main simulation loop
        while (step < MAX_STEPS) {
            // Show current state
            visualizer.visualize(entities);
            
            // Show info
            std::cout << "Simulation Time: " << simulationTime << "s | Step: " << step << "/" 
                      << MAX_STEPS << " | Active Anomalies: " << anomalySystem.getAnomalies().size() 
                      << (isPaused ? " | PAUSED" : "") << std::endl;
            
            // Get user input for interactive control
            std::cout << "Command: ";
            char input = 0;
            if (std::cin.peek() != '\n') {
                std::cin >> input;
            }
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            
            // Process input
            if (input == 'q') {
                std::cout << "Exiting simulation..." << std::endl;
                break;
            } else if (input == 'p') {
                isPaused = !isPaused;
                std::cout << (isPaused ? "Simulation paused." : "Simulation resumed.") << std::endl;
                continue;
            }
            
            // Skip updates if paused
            if (isPaused) {
                continue;
            }
            
            // Update entities
            for (auto& entity : entities) {
                entity->update(timeStep);
            }
            
            // Update anomaly system
            anomalySystem.update(timeStep);
            
            // Create new anomalies occasionally
            if (step % 10 == 0 && step > 0) {
                // Random position
                float x = std::uniform_real_distribution<float>(-30.0f, 30.0f)(rng);
                float y = std::uniform_real_distribution<float>(-30.0f, 30.0f)(rng);
                
                // Random anomaly type
                AnomalyType type = static_cast<AnomalyType>(
                    std::uniform_int_distribution<int>(0, 5)(rng)
                );
                
                // Parameters based on type
                float radius = std::uniform_real_distribution<float>(5.0f, 15.0f)(rng);
                float timeDistortion = std::uniform_real_distribution<float>(0.1f, 2.0f)(rng);
                float duration = std::uniform_real_distribution<float>(5.0f, 15.0f)(rng);
                
                createAnomaly(Vec3(x, y, 0), radius, type, timeDistortion, duration);
            }
            
            // Update time
            simulationTime += timeStep;
            step++;
            
            // Add a small delay for smoother visualization
            delay(100);
        }
        
        std::cout << Colors::BOLD << Colors::BRIGHT_CYAN 
                  << "Simulation Complete!" 
                  << Colors::RESET << std::endl;
    }
};

int main() {
    // Enable ANSI colors on Windows
    #ifdef _WIN32
    system("color");
    #endif
    
    // Seed random number generator
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    
    // Create and run simulation
    TemporalSimulation simulation;
    simulation.setupScene();
    simulation.run();
    
    return 0;
} 