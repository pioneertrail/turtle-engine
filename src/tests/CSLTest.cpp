#include "csl/GestureRecognizer.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <chrono> // Include chrono for timing
#include <vector> // Include vector for points
#include <functional> // For std::function
#include <iomanip> // For std::setprecision

#include "csl/CSLSystem.hpp" // Include CSLSystem

// Function to run the combo test with specified duration parameters
bool runComboTest(TurtleEngine::CSL::GestureRecognizer& recognizer, TurtleEngine::CSL::CSLSystem& cslSystem, float khargailDuration, float flammilDuration) {
    std::cout << "\n--- Running Combo Test (Khargail: " << khargailDuration << "s, Flammil: " << flammilDuration << "s) ---" << std::endl;
    recognizer.resetTransitionStats(); // Reset stats for each test run

    // --- Khargail Simulation ---
    std::cout << "Generating Khargail points" << std::endl;
    std::vector<cv::Point2f> khargailPoints;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> noise(-2.0f, 2.0f);
    const int frameWidth = 1280;
    const int frameHeight = 720;
    const float startXKhargail = frameWidth * 0.4f;
    const float startYKhargail = frameHeight * 0.5f;
    const float khargailDist = 65.0f; // Keep distance consistent
    const float endXKhargail = startXKhargail + khargailDist;
    const float endYKhargail = startYKhargail;
    const int khargailNumPoints = static_cast<int>(khargailDuration * 60.0f); // Points based on duration

    for (int i = 0; i < khargailNumPoints; ++i) {
        float t = (khargailNumPoints > 1) ? static_cast<float>(i) / (khargailNumPoints - 1) : 1.0f;
        float x = startXKhargail + t * (endXKhargail - startXKhargail) + noise(gen);
        float y = startYKhargail + t * (endYKhargail - startYKhargail) + noise(gen);
        khargailPoints.push_back(cv::Point2f(x, y));
    }
    std::cout << "Khargail points generated, size: " << khargailPoints.size() << std::endl;

    // Process Khargail
    auto startTime = std::chrono::high_resolution_clock::now();
    TurtleEngine::CSL::GestureResult khargailResult = recognizer.processSimulatedPoints(khargailPoints);
    auto endTime = std::chrono::high_resolution_clock::now();
    float khargailProcessingLatency = std::chrono::duration<float>(endTime - startTime).count();
    std::cout << "Khargail Test:" << std::endl;
    std::cout << "  Confidence: " << khargailResult.confidence << " (Threshold: " << recognizer.getGestureThreshold(TurtleEngine::CSL::GestureType::KHARGAIL) << ")" << std::endl;
    std::cout << "  Processing Latency: " << khargailProcessingLatency << "s" << std::endl;
    std::cout << "  Gesture Type: " << (khargailResult.type == TurtleEngine::CSL::GestureType::KHARGAIL ? "KHARGAIL" : "Unexpected") << std::endl;

    // --- Flammil Simulation ---
    std::cout << "Generating Flammil points" << std::endl;
    std::vector<cv::Point2f> flammilPoints;
    const float startXFlammil = frameWidth * 0.5f;
    const float startYFlammil = frameHeight * 0.3f;
    const float flammilDistX = 75.0f;
    const float flammilDistY = 75.0f;
    const float endXFlammil = startXFlammil + flammilDistX;
    const float endYFlammil = startYFlammil + flammilDistY;
    const int flammilNumPoints = static_cast<int>(flammilDuration * 60.0f); // Points based on duration

    for (int i = 0; i < flammilNumPoints; ++i) {
        float t = (flammilNumPoints > 1) ? static_cast<float>(i) / (flammilNumPoints - 1) : 1.0f;
        float x = startXFlammil + t * (endXFlammil - startXFlammil) + noise(gen);
        float y = startYFlammil + t * (endYFlammil - startYFlammil) + noise(gen);
        flammilPoints.push_back(cv::Point2f(x, y));
    }
    std::cout << "Flammil points generated, size: " << flammilPoints.size() << std::endl;

    // Process Flammil
    startTime = std::chrono::high_resolution_clock::now();
    TurtleEngine::CSL::GestureResult flammilResult = recognizer.processSimulatedPoints(flammilPoints);
    endTime = std::chrono::high_resolution_clock::now();
    float flammilProcessingLatency = std::chrono::duration<float>(endTime - startTime).count();
    float transitionLatency = recognizer.getLastTransition().latency;
    std::cout << "Flammil Test:" << std::endl;
    std::cout << "  Confidence: " << flammilResult.confidence << " (Threshold: " << recognizer.getGestureThreshold(TurtleEngine::CSL::GestureType::FLAMMIL) << ")" << std::endl;
    std::cout << "  Processing Latency: " << flammilProcessingLatency << "s" << std::endl;
    std::cout << "  Gesture Type: " << (flammilResult.type == TurtleEngine::CSL::GestureType::FLAMMIL ? "FLAMMIL" : "Unexpected") << std::endl;
    std::cout << "Transition Latency (Khargail->Flammil): " << transitionLatency << "s (Target: <0.1)" << std::endl;

    // Manually trigger the plasma callback using the test result
    std::cout << "Manually triggering CSLSystem plasma callback with Flammil result..." << std::endl;
    if (flammilResult.type == TurtleEngine::CSL::GestureType::FLAMMIL) {
        cslSystem.triggerPlasmaCallback(flammilResult);
    } else {
        std::cout << "Skipping manual trigger: Flammil was not the detected type." << std::endl;
    }

    // Validate Combo
    bool khargailPassed = khargailResult.type == TurtleEngine::CSL::GestureType::KHARGAIL &&
                           khargailResult.confidence >= recognizer.getGestureThreshold(TurtleEngine::CSL::GestureType::KHARGAIL);
    bool flammilPassed = flammilResult.type == TurtleEngine::CSL::GestureType::FLAMMIL &&
                          flammilResult.confidence >= recognizer.getGestureThreshold(TurtleEngine::CSL::GestureType::FLAMMIL);
    bool transitionPassed = transitionLatency < 0.1f;

    std::cout << "--- Validation (Duration: " << flammilDuration << "s) ---" << std::endl;
    std::cout << "Khargail Valid: " << (khargailPassed ? "PASS" : "FAIL") << std::endl;
    std::cout << "Flammil Valid: " << (flammilPassed ? "PASS" : "FAIL") << std::endl;
    std::cout << "Transition (<0.1s) Valid: " << (transitionPassed ? "PASS" : "FAIL") << std::endl;

    if (khargailPassed && flammilPassed && transitionPassed) {
        std::cout << "Combo Validation: PASSED" << std::endl;
        return true;
    } else {
        std::cout << "Combo Validation: FAILED" << std::endl;
        return false;
    }
}

int main() {
    std::cout << "Starting CSLTest" << std::endl;
    TurtleEngine::CSL::GestureRecognizer recognizer;
    std::cout << "GestureRecognizer constructed" << std::endl;
    // Recognizer should only be initialized once
    if (!recognizer.initialize()) {
        std::cerr << "Failed to initialize GestureRecognizer" << std::endl;
        return -1;
    }
    std::cout << "GestureRecognizer initialized" << std::endl;

    TurtleEngine::CSL::CSLSystem cslSystem;
    // CSLSystem now uses the *same* recognizer instance implicitly via its constructor
    // We no longer need to initialize CSLSystem separately for this test structure.
    // if (!cslSystem.initialize(0)) { ... }
    std::cout << "CSLSystem constructed for callback test" << std::endl;

    // Add plasma callback with animation simulation
    cslSystem.addPlasmaCallback([&cslSystem](const TurtleEngine::CSL::GestureResult& result) { // Capture cslSystem
        std::cout << "*** Plasma Effect Triggered for Flammil! *** End Pos: (" 
                   << result.position.x << "," << result.position.y << ") Confidence: " 
                   << result.confidence << std::endl;
        // Log velocities for Elena's reference
        bool velocitiesAvailable = !result.velocities.empty(); // Check if not empty
        std::cout << "Velocities vector size: " << result.velocities.size() << std::endl; // Debug size
        for (size_t i = 0; i < result.velocities.size(); ++i) {
            if (i % 5 == 0) { // Log every 5th velocity for brevity
                // result.velocities[i] is now the normalized magnitude (float)
                std::cout << "Normalized Velocity at segment " << i << ": " 
                          << std::fixed << std::setprecision(3) << result.velocities[i] << std::endl;
            }
        }
        // Simulate plasma animation along trajectory using configured duration
        float plasmaDuration = cslSystem.getPlasmaDuration();
        const int totalFrames = static_cast<int>(plasmaDuration * 60.0f);
        const int keyframeCount = 24;
        const float frameDuration = 1.0f / 60.0f;
        if (result.trajectory.empty()) {
            std::cerr << "Plasma Callback Error: Trajectory is empty!" << std::endl;
            return;
        }
        cv::Point2f start = result.trajectory.front();
        cv::Point2f end = result.trajectory.back();
        for (int frame = 0; frame < totalFrames; ++frame) {
            float t = (totalFrames > 1) ? static_cast<float>(frame) / (totalFrames - 1) : 1.0f;
            float x = start.x + t * (end.x - start.x);
            float y = start.y + t * (end.y - start.y);
            int keyframeIndex = (frame % keyframeCount);
            std::cout << "Plasma Frame " << frame << " (keyframe " << keyframeIndex << "): Pos=(" 
                       << x << "," << y << ")";
            if (velocitiesAvailable && frame < result.velocities.size()) {
                // float normVelMag = cv::norm(result.velocities[frame]); // No longer Point2f
                float normVelMag = result.velocities[frame]; // Directly use the stored float
                std::cout << " NormVelMag: " << std::fixed << std::setprecision(3) << normVelMag;
            }
            std::cout << std::endl;
        }
    });

    // --- Run Test Suite --- 
    bool overallResult = true;
    // Test 1: Standard duration (Khargail ~0.4s, Flammil 0.5s)
    overallResult &= runComboTest(recognizer, cslSystem, 0.4f, 0.5f);
    // Test 2: Faster duration (Khargail ~0.25s, Flammil 0.3s)
    overallResult &= runComboTest(recognizer, cslSystem, 0.25f, 0.3f);
    // Test 3: Slower duration (Khargail ~0.6s, Flammil 0.7s)
    cslSystem.setPlasmaDuration(0.7f); // Adjust plasma duration for the longer test
    overallResult &= runComboTest(recognizer, cslSystem, 0.6f, 0.7f);

    std::cout << "\n--- Test Suite Summary ---" << std::endl;
    if (overallResult) {
        std::cout << "Overall Result: PASSED" << std::endl;
        return 0; // Success exit code
    } else {
        std::cout << "Overall Result: FAILED" << std::endl;
        return 1; // Failure exit code
    }
}