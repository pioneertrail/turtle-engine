#include "../../engine/include/ParticleSystem.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace TurtleEngine;

class ParticleSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize OpenGL context
        // Note: This should be handled by your test framework
        system = std::make_unique<ParticleSystem>(10000);
        system->initialize();
        
        // Create view and projection matrices for rendering
        view = glm::lookAt(
            glm::vec3(0.0f, 0.0f, 10.0f),  // Camera position
            glm::vec3(0.0f, 0.0f, 0.0f),   // Look at origin
            glm::vec3(0.0f, 1.0f, 0.0f)    // Up vector
        );
        
        projection = glm::perspective(
            glm::radians(45.0f),  // FOV
            16.0f / 9.0f,         // Aspect ratio
            0.1f,                 // Near plane
            100.0f                // Far plane
        );
    }

    void logPerformanceMetrics(const std::string& testName, 
                             double renderTimeMs,
                             size_t particleCount,
                             bool highContrastEnabled) {
        std::ofstream log("performance_log.txt", std::ios::app);
        log << "=== " << testName << " ===\n"
            << "Hardware: Intel HD Graphics 4000\n"
            << "Particle Count: " << particleCount << "\n"
            << "High Contrast: " << (highContrastEnabled ? "Enabled" : "Disabled") << "\n"
            << "Render Time: " << std::fixed << std::setprecision(4) << renderTimeMs << " ms\n"
            << "Target: 0.01 ms\n"
            << "Status: " << (renderTimeMs <= 0.01 ? "PASS" : "NEEDS OPTIMIZATION") << "\n\n";
    }

    std::unique_ptr<ParticleSystem> system;
    glm::mat4 view;
    glm::mat4 projection;
};

TEST_F(ParticleSystemTest, RenderTimeTest) {
    const size_t particleCounts[] = {100, 1000, 10000};
    const bool highContrastModes[] = {false, true}; // We'll test with and without high contrast
    
    for (size_t count : particleCounts) {
        for (bool highContrast : highContrastModes) {
            // Create a new system with the desired particle count
            system = std::make_unique<ParticleSystem>(count);
            system->initialize();
            
            // Emit test particles in a burst
            system->spawnBurst(
                count,           // particle count
                glm::vec3(0.0f), // center position
                2.0f,           // initial velocity
                5.0f,           // life
                highContrast ? glm::vec4(1.0f, 1.0f, 1.0f, 1.0f) : glm::vec4(1.0f, 0.5f, 0.0f, 1.0f) // color (white for high contrast)
            );
            
            // Measure render time
            auto start = std::chrono::high_resolution_clock::now();
            
            // Render multiple frames to get average
            const int FRAMES = 100;
            for (int i = 0; i < FRAMES; ++i) {
                system->update(1.0f / 60.0f);
                system->render(view, projection);
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(end - start);
            double averageRenderTime = duration.count() / FRAMES;
            
            // Log results
            std::string testName = "Particle_" + std::to_string(count) + 
                                 (highContrast ? "_HighContrast" : "_Normal");
            logPerformanceMetrics(testName, averageRenderTime, count, highContrast);
        }
    }
}

TEST_F(ParticleSystemTest, VisualTest) {
    system = std::make_unique<ParticleSystem>(1000);
    system->initialize();
    
    // Create a test pattern
    const float positions[][3] = {
        {-1.0f, -1.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f},
        { 0.0f,  1.0f, 0.0f}
    };
    
    for (const auto& pos : positions) {
        Particle p(
            glm::vec3(pos[0], pos[1], pos[2]),  // position
            glm::vec3(0.0f),                    // velocity
            glm::vec4(1.0f),                    // color (white)
            10.0f                               // life
        );
        system->spawnParticle(p);
    }
    
    // Render for visual inspection
    system->render(view, projection);
    
    // Note: This test requires manual inspection
    std::cout << "\nVisual Test:\n"
              << "Please verify:\n"
              << "1. Particles are clearly visible\n"
              << "2. No visual artifacts\n"
              << "3. Contrast is sufficient\n"
              << "4. Depth testing works correctly\n";
}

TEST_F(ParticleSystemTest, PerformanceTest) {
    const size_t particleCounts[] = {100, 1000, 10000};
    const size_t particleCount = 10000;
    
    for (size_t count : particleCounts) {
        system = std::make_unique<ParticleSystem>(count);
        system->initialize();
        
        // Emit test particles
        system->spawnBurst(
            count,
            glm::vec3(0.0f),
            1.0f,
            5.0f,
            glm::vec4(1.0f)
        );
        
        // Measure render time
        auto start = std::chrono::high_resolution_clock::now();
        
        const int FRAMES = 100;
        for (int i = 0; i < FRAMES; ++i) {
            system->update(1.0f / 60.0f);
            system->render(view, projection);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start);
        double averageRenderTime = duration.count() / FRAMES;
        
        std::string testName = "ParticleCount_" + std::to_string(count);
        logPerformanceMetrics(testName, averageRenderTime, count, false);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 