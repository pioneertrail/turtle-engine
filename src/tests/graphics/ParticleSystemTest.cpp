#include "../../engine/graphics/include/ParticleSystem.hpp"
#include <gtest/gtest.h>
#include <chrono>
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace TurtleEngine::Graphics;

class ParticleSystemTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize OpenGL context
        // Note: This should be handled by your test framework
        system = std::make_unique<ParticleSystem>(10000);
        system->initialize();
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
};

TEST_F(ParticleSystemTest, RenderTimeTest) {
    const size_t particleCounts[] = {100, 1000, 10000};
    const bool highContrastModes[] = {false, true};
    
    for (size_t count : particleCounts) {
        for (bool highContrast : highContrastModes) {
            system->setMaxParticles(count);
            system->setHighContrastMode(highContrast);
            
            // Emit test particles in a burst
            system->emitBurst(
                glm::vec3(0.0f), // center position
                count,           // particle count
                1.0f,           // min velocity
                2.0f,           // max velocity
                glm::vec4(1.0f), // white color
                1.0f,           // size
                5.0f            // life
            );
            
            // Measure render time
            auto start = std::chrono::high_resolution_clock::now();
            
            // Render multiple frames to get average
            const int FRAMES = 100;
            for (int i = 0; i < FRAMES; ++i) {
                system->update(1.0f / 60.0f);
                system->render();
            }
            
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration<double, std::milli>(end - start);
            double averageRenderTime = duration.count() / FRAMES;
            
            // Log results
            std::string testName = "Particle_" + std::to_string(count) + 
                                 (highContrast ? "_HighContrast" : "_Normal");
            logPerformanceMetrics(testName, averageRenderTime, count, highContrast);
            
            // Visual artifact check
            if (highContrast) {
                system->enableDebugView(true);
                system->render(); // Render one frame with debug view
                // Note: Manual inspection required for visual artifacts
            }
        }
    }
}

TEST_F(ParticleSystemTest, HighContrastVisualTest) {
    system->setMaxParticles(1000);
    system->setHighContrastMode(true);
    
    // Create a test pattern
    const float positions[][3] = {
        {-1.0f, -1.0f, 0.0f},
        { 1.0f, -1.0f, 0.0f},
        { 0.0f,  1.0f, 0.0f}
    };
    
    for (const auto& pos : positions) {
        system->emit(
            glm::vec3(pos[0], pos[1], pos[2]),
            glm::vec3(0.0f),
            glm::vec4(1.0f),
            2.0f,
            10.0f
        );
    }
    
    // Enable debug view for visual inspection
    system->enableDebugView(true);
    system->render();
    
    // Note: This test requires manual inspection of the debug view
    std::cout << "\nHigh Contrast Visual Test:\n"
              << "Please verify:\n"
              << "1. Particles are clearly visible\n"
              << "2. No visual artifacts\n"
              << "3. Contrast is sufficient\n"
              << "4. Depth testing works correctly\n";
}

TEST_F(ParticleSystemTest, BatchSizeOptimization) {
    const size_t batchSizes[] = {100, 500, 1000, 2000};
    const size_t particleCount = 10000;
    
    system->setMaxParticles(particleCount);
    system->setInstancingEnabled(false); // Test batched rendering
    
    for (size_t batchSize : batchSizes) {
        system->setBatchSize(batchSize);
        
        // Emit test particles
        system->emitBurst(
            glm::vec3(0.0f),
            particleCount,
            1.0f,
            2.0f,
            glm::vec4(1.0f),
            1.0f,
            5.0f
        );
        
        // Measure render time
        auto start = std::chrono::high_resolution_clock::now();
        
        const int FRAMES = 100;
        for (int i = 0; i < FRAMES; ++i) {
            system->update(1.0f / 60.0f);
            system->render();
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start);
        double averageRenderTime = duration.count() / FRAMES;
        
        std::string testName = "BatchSize_" + std::to_string(batchSize);
        logPerformanceMetrics(testName, averageRenderTime, particleCount, false);
    }
}

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 