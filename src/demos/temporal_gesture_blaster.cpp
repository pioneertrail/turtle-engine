// Placeholder for Temporal Gesture Blaster demo
// === Includes ===
#include <Engine.hpp>
#include <Window.hpp>
#include <InputManager.hpp>
#include <Renderer.hpp>
#include <ParticleSystem.hpp>
#include <TemporalAnomalySystem.hpp>
#include <AffectedEntity.hpp>
#include <glm/vec2.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // For pi() if needed for circle rendering later
#include <vector>
#include <memory>
#include <iostream>
#include <cstdint>
#include <chrono>
#include <random>
#include <map>
#include <cmath> // For std::abs or glm::clamp in collision checks

// === Constants ===
const float PLAYER_SPEED = 250.0f; // Pixels per second
const float PLAYER_RADIUS = 15.0f;
const float ENEMY_SPEED = 75.0f; // Pixels per second
const float ENEMY_SPAWN_RATE = 1.5f; // Seconds
const float WINDOW_WIDTH = 800.0f;
const float WINDOW_HEIGHT = 600.0f;

// Task 6.1: AABB Collision Check Helper Function (Moved before main)
bool checkAABBCollision(const glm::vec2& pos1, const glm::vec2& size1, const glm::vec2& pos2, const glm::vec2& size2) {
    glm::vec2 halfSize1 = size1 / 2.0f;
    glm::vec2 halfSize2 = size2 / 2.0f;
    glm::vec2 min1 = pos1 - halfSize1;
    glm::vec2 max1 = pos1 + halfSize1;
    glm::vec2 min2 = pos2 - halfSize2;
    glm::vec2 max2 = pos2 + halfSize2;
    bool overlapX = max1.x > min2.x && min1.x < max2.x; // Use > and < for non-inclusive edges if preferred
    bool overlapY = max1.y > min2.y && min1.y < max2.y;
    return overlapX && overlapY;
}

// Overload for Circle (pos, radius) vs Rectangle (pos, size)
// Phase 6 Polish: Explicitly notes the simplification.
bool checkAABBCollision(const glm::vec2& circlePos, float circleRadius, const glm::vec2& rectPos, const glm::vec2& rectSize) {
    // NOTE: Simplified check treats circle as its AABB (a square)
    glm::vec2 circleSize = glm::vec2(circleRadius * 2.0f);
    return checkAABBCollision(circlePos, circleSize, rectPos, rectSize);
}

// === Structs ===

// Task 1.3: Player Struct
struct Player {
    glm::vec2 position = {WINDOW_WIDTH / 2.0f, WINDOW_HEIGHT - 50.0f}; // Start near bottom-center
    float radius = PLAYER_RADIUS;

    // Phase 3 Polish: Player Update Method
    void Update(float deltaTime, TurtleEngine::InputManager* inputManager, float windowWidth) {
        // Movement Input (Task 3.1)
        if (inputManager->isKeyPressed(GLFW_KEY_A) || inputManager->isKeyPressed(GLFW_KEY_LEFT)) {
            position.x -= PLAYER_SPEED * deltaTime;
        }
        if (inputManager->isKeyPressed(GLFW_KEY_D) || inputManager->isKeyPressed(GLFW_KEY_RIGHT)) {
            position.x += PLAYER_SPEED * deltaTime;
        }

        // Screen Clamping (Task 3.2)
        position.x = glm::clamp(position.x, radius, windowWidth - radius);
    }
};

// Task 1.4: Enemy Struct (Adjusted for AffectedEntity Interface & Phase 5)
struct Enemy : public TurtleEngine::Temporal::AffectedEntity {
    int id;
    glm::vec2 position;
    glm::vec2 velocity;
    glm::vec2 size = {20.0f, 20.0f};
    bool active = true;
    bool isStasized = false;
    float stasisTimer = 0.0f;
    glm::vec2 baseVelocity;

    Enemy(int entityId, glm::vec2 startPos, glm::vec2 startVel)
        : id(entityId), position(startPos), velocity(startVel), baseVelocity(startVel) {}

    // --- AffectedEntity Overrides ---
    int GetId() const override { return id; }
    glm::vec3 GetPosition() const override { return glm::vec3(position.x, position.y, 0.0f); }

    void ApplyTemporalEffect(const TurtleEngine::Temporal::AnomalyEffect& effect) override {
        // std::cout << "Enemy " << id << " received effect " << static_cast<int>(effect.type) << std::endl; // Reduce spam
        switch (effect.type) {
            case TurtleEngine::Temporal::AnomalyType::STASIS:
                if (!isStasized) {
                    isStasized = true;
                    stasisTimer = effect.duration;
                    velocity = glm::vec2(0.0f);
                    // std::cout << "  -> STASIS applied." << std::endl; // Reduce spam
                }
                break;
            case TurtleEngine::Temporal::AnomalyType::RIFT:
                active = false;
                // std::cout << "  -> RIFT encountered, deactivating." << std::endl; // Reduce spam
                break;
            default: break;
        }
    }
    // --- End AffectedEntity Overrides ---

    void Update(float deltaTime) {
        if (isStasized) {
            stasisTimer -= deltaTime;
            if (stasisTimer <= 0.0f) {
                isStasized = false;
                velocity = baseVelocity;
                // std::cout << "Enemy " << id << " STASIS ended." << std::endl; // Reduce spam
            }
        }

        if (!isStasized && active) {
            position += velocity * deltaTime;
        }
        // Off-screen check moved to main loop
    }
};


// === Main Function ===
int main() {
    std::cout << "Starting Temporal Gesture Blaster Demo..." << std::endl;
    TurtleEngine::Engine engine;
    if (!engine.initialize("Temporal Gesture Blaster", static_cast<int>(WINDOW_WIDTH), static_cast<int>(WINDOW_HEIGHT))) {
        std::cerr << "FATAL: Failed to initialize Turtle Engine!" << std::endl;
        return -1;
    }
    std::cout << "Engine initialized successfully." << std::endl;

    auto* window = engine.getWindow();
    auto* inputManager = engine.getInputManager();
    auto* renderer = engine.getRenderer();
    auto* particleSystem = engine.getParticleSystem();
    if (!window || !inputManager || !renderer || !particleSystem) {
        std::cerr << "FATAL: Failed to retrieve engine subsystems!" << std::endl;
        engine.shutdown();
        return -1;
    }
    std::cout << "Engine subsystems retrieved." << std::endl;

    TurtleEngine::Temporal::TemporalAnomalySystem temporalSystem;
    std::cout << "TemporalAnomalySystem instantiated." << std::endl;

    Player player;
    std::vector<std::shared_ptr<Enemy>> enemies;
    int nextEnemyId = 0;
    float enemySpawnTimer = 0.0f;

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> distribX(PLAYER_RADIUS, WINDOW_WIDTH - PLAYER_RADIUS); // Spawn/Rift X (within playable area)
    std::uniform_real_distribution<float> distribY(50.0f, WINDOW_HEIGHT / 2.0f); // Rift Y position (upper half)

    std::map<int, bool> gestureJustTriggered = {{GLFW_KEY_1, false}, {GLFW_KEY_2, false}, {GLFW_KEY_3, false}};
    bool gameOver = false; // Task 6.4

    auto lastFrameTime = std::chrono::high_resolution_clock::now();
    float deltaTime = 0.0f;

    glm::mat4 projectionMatrix = glm::ortho(0.0f, WINDOW_WIDTH, WINDOW_HEIGHT, 0.0f, -1.0f, 1.0f);
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    renderer->setProjectionMatrix(projectionMatrix);
    std::cout << "Projection matrix set. Entering main loop..." << std::endl;

    // --- Game Loop ---
    while (!window->shouldClose()) {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        deltaTime = std::chrono::duration<float>(currentFrameTime - lastFrameTime).count();
        lastFrameTime = currentFrameTime;

        // Clamp delta time
        deltaTime = std::min(deltaTime, 0.1f); 

        window->pollEvents();

        // --- Input ---
        if (inputManager->isKeyPressed(GLFW_KEY_ESCAPE)) {
            window->setShouldClose(true);
        }

        if (!gameOver) { // Task 6.4: Only process game input if not game over
            player.Update(deltaTime, inputManager, WINDOW_WIDTH); // Phase 3 Polish

            // Gesture Input Detection
            gestureJustTriggered[GLFW_KEY_1] = inputManager->isKeyJustPressed(GLFW_KEY_1);
            gestureJustTriggered[GLFW_KEY_2] = inputManager->isKeyJustPressed(GLFW_KEY_2);
            gestureJustTriggered[GLFW_KEY_3] = inputManager->isKeyJustPressed(GLFW_KEY_3);
        } else {
            // Reset flags if game over
            gestureJustTriggered[GLFW_KEY_1] = false;
            gestureJustTriggered[GLFW_KEY_2] = false;
            gestureJustTriggered[GLFW_KEY_3] = false;
        }

        // --- Updates ---
        // Phase 6 Polish: Encapsulate all game logic updates within !gameOver
        if (!gameOver) { 
            // Task 4.1: Enemy Spawning
            enemySpawnTimer += deltaTime;
            if (enemySpawnTimer >= ENEMY_SPAWN_RATE) {
                enemySpawnTimer -= ENEMY_SPAWN_RATE;
                float spawnX = distribX(gen);
                glm::vec2 spawnPos = {spawnX, -20.0f};
                glm::vec2 spawnVel = {0.0f, ENEMY_SPEED};
                auto newEnemy = std::make_shared<Enemy>(nextEnemyId++, spawnPos, spawnVel);
                enemies.push_back(newEnemy);
                temporalSystem.RegisterEntity(newEnemy);
                 // std::cout << "Spawned & Registered Enemy " << newEnemy->id << std::endl; // Reduce console spam
            }

            // Task 4.3 & 6.3: Enemy Updates & Player Collision Check
            for (const auto& enemy : enemies) {
                if (!enemy->active) continue; 

                enemy->Update(deltaTime); // Update position and stasis

                // Task 6.3: Player-Enemy Collision Check
                if (enemy->active && checkAABBCollision(player.position, player.radius, enemy->position, enemy->size)) {
                    std::cout << "GAME OVER! Player collided with Enemy " << enemy->id << std::endl;
                    gameOver = true;
                    break; // Stop checking enemies for this frame
                }

                 // Deactivate if off-screen (checked after update and collision)
                if (enemy->position.y > WINDOW_HEIGHT + enemy->size.y) { // Check slightly below bottom edge
                     // std::cout << "Enemy " << enemy->id << " off-screen." << std::endl; // Reduce spam
                    enemy->active = false;
                }
            }
        } // End if (!gameOver) for game logic updates

        // --- System Updates (Run regardless of game over state for fade-out effects) ---
        temporalSystem.Update(deltaTime);
        particleSystem->update(deltaTime);

        // --- Effect Triggering & Particle Collision ---
        // Note: Gesture flags are already false if gameOver is true
        glm::vec3 playerPos3D = glm::vec3(player.position.x, player.position.y, 0.0f);
        if (gestureJustTriggered[GLFW_KEY_1]) { // Attack ('1' key)
            // std::cout << "Triggering Attack Particle Burst" << std::endl; // Reduce spam
            particleSystem->spawnBurst(50, playerPos3D, 150.0f, 0.5f, glm::vec4(1.0f, 0.5f, 0.0f, 1.0f));

            // Task 6.2: Simplified Particle-Enemy Collision
            float attackCollisionRadius = PLAYER_RADIUS + 5.0f; // Small radius around player
            for (const auto& enemy : enemies) {
                if (enemy->active && !enemy->isStasized) {
                    if (checkAABBCollision(player.position, attackCollisionRadius, enemy->position, enemy->size)) {
                        // std::cout << "Attack burst hit Enemy " << enemy->id << std::endl; // Reduce spam
                        enemy->active = false; // Deactivate enemy
                    }
                }
            }
        }
        if (gestureJustTriggered[GLFW_KEY_2]) { // Stasis ('2' key)
            float stasisRadius = 75.0f;
            float stasisDuration = 3.0f;
            // std::cout << "Creating STASIS Anomaly at Player" << std::endl; // Reduce spam
            temporalSystem.CreateAnomaly(TurtleEngine::Temporal::AnomalyType::STASIS, 1.0f, playerPos3D, stasisRadius, stasisDuration);
        }
        if (gestureJustTriggered[GLFW_KEY_3]) { // Rift ('3' key)
            float riftRadius = 50.0f;
            float riftDuration = 1.0f;
            glm::vec3 riftPos = {distribX(gen), distribY(gen), 0.0f};
            // std::cout << "Creating RIFT Anomaly at (" << riftPos.x << ", " << riftPos.y << ")" << std::endl; // Reduce spam
            temporalSystem.CreateAnomaly(TurtleEngine::Temporal::AnomalyType::RIFT, 1.0f, riftPos, riftRadius, riftDuration);
        }

        // --- Enemy Removal Pass ---
        // Iterate backwards to safely remove elements
        for (int i = static_cast<int>(enemies.size()) - 1; i >= 0; --i) {
            if (!enemies[i]->active) {
                 // std::cout << "Unregistering & Removing inactive Enemy " << enemies[i]->id << std::endl; // Reduce spam
                temporalSystem.UnregisterEntity(enemies[i]->GetId());
                enemies.erase(enemies.begin() + i);
            }
        }

        // --- Rendering ---
        renderer->clear(0.1f, 0.1f, 0.1f, 1.0f);

        // Render Player
        glm::vec3 playerColor = gameOver ? glm::vec3(0.5f, 0.5f, 0.5f) : glm::vec3(0.0f, 1.0f, 0.0f); // Grey if game over
        renderer->drawCircle(player.position, player.radius, playerColor);

        // Render Enemies (Only active enemies remain in the vector)
        for (const auto& enemy : enemies) {
            glm::vec3 color = enemy->isStasized ? glm::vec3(0.5f, 0.5f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            renderer->drawRectangle(enemy->position, enemy->size, color);
        }

        // Render Temporal Anomalies
        for (const auto& anomaly : temporalSystem.GetActiveAnomalies()) {
            glm::vec2 anomalyPos2D = glm::vec2(anomaly->GetPosition().x, anomaly->GetPosition().y);
             if (anomaly->GetType() == TurtleEngine::Temporal::AnomalyType::STASIS) {
                 renderer->drawCircleOutline(anomalyPos2D, anomaly->GetRadius(), glm::vec3(0.5f, 0.5f, 1.0f));
             } else if (anomaly->GetType() == TurtleEngine::Temporal::AnomalyType::RIFT) {
                 renderer->drawCircleOutline(anomalyPos2D, anomaly->GetRadius(), glm::vec3(1.0f, 0.0f, 1.0f));
             }
        }

        // Render Particles
        particleSystem->render(viewMatrix, projectionMatrix);

        // Task 6.5: Render Game Over Text (Optional - Skipped)
        // if (gameOver) { /* Draw text here */ }

        window->swapBuffers();
    }

    std::cout << "Exited main loop. Shutting down..." << std::endl;
    engine.shutdown();
    std::cout << "Shutdown complete. Exiting." << std::endl;
    return 0;
} 