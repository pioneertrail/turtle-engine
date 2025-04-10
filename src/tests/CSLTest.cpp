// Define _USE_MATH_DEFINES at the very top before any includes
#define _USE_MATH_DEFINES

#include "csl/GestureRecognizer.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
// #define _USE_MATH_DEFINES // Moved to top
#include <cmath> // Include cmath for std::sin/cos and M_PI
#include <chrono> // Include chrono for timing
#include <vector> // Include vector for points
#include <functional> // For std::function
#include <iomanip> // For std::setprecision
#include <numeric> // For std::accumulate

#include "csl/CSLSystem.hpp" // Include CSLSystem

// Function to run the combo test with specified duration parameters
bool runComboTest(TurtleEngine::CSL::GestureRecognizer& recognizer, TurtleEngine::CSL::CSLSystem& cslSystem, float khargailDuration, float flammilDuration, const std::string& testCaseId) {
    std::cout << "\n=== Running Combo Test ===" << std::endl;
    std::cout << "Test Case: " << testCaseId << std::endl;
    std::cout << "Khargail Duration: " << khargailDuration << "s" << std::endl;
    std::cout << "Flammil Duration: " << flammilDuration << "s" << std::endl;
    
    // Generate Khargail points (left-to-right swipe)
    std::vector<cv::Point2f> khargailPoints;
    const int numPoints = 30;
    const float startX = 100.0f;
    const float startY = 360.0f;
    const float endX = 700.0f;
    const float endY = 360.0f;
    
    for (int i = 0; i < numPoints; ++i) {
        float t = static_cast<float>(i) / (numPoints - 1);
        float x = startX + t * (endX - startX);
        float y = startY + 10.0f * std::sin(t * M_PI); // Slight curve for realism
        khargailPoints.push_back(cv::Point2f(x, y));
    }
    
    // Process Khargail points
    std::cout << "\nProcessing Khargail points..." << std::endl;
    auto khargailResult = recognizer.processSimulatedPoints(khargailPoints, testCaseId + "_KHARGAIL");
    std::cout << "Khargail Result: " << static_cast<int>(khargailResult.type) 
              << " (Confidence: " << khargailResult.confidence << ")" << std::endl;
    
    // Generate Flammil points (right-to-down swipe)
    std::vector<cv::Point2f> flammilPoints;
    const float flammilStartX = endX;
    const float flammilStartY = endY;
    const float flammilEndX = endX + 100.0f;
    const float flammilEndY = endY + 300.0f;
    
    for (int i = 0; i < numPoints; ++i) {
        float t = static_cast<float>(i) / (numPoints - 1);
        float x = flammilStartX + t * (flammilEndX - flammilStartX);
        float y = flammilStartY + t * (flammilEndY - flammilStartY) + 5.0f * std::sin(t * M_PI); // Slight curve
        flammilPoints.push_back(cv::Point2f(x, y));
    }
    
    // Process Flammil points
    std::cout << "\nProcessing Flammil points..." << std::endl;
    auto flammilResult = recognizer.processSimulatedPoints(flammilPoints, testCaseId + "_FLAMMIL");
    std::cout << "Flammil Result: " << static_cast<int>(flammilResult.type) 
              << " (Confidence: " << flammilResult.confidence << ")" << std::endl;
    
    // Test Stasai (circle) detection
    std::vector<cv::Point2f> stasaiPoints;
    const float centerX = 640.0f;
    const float centerY = 360.0f;
    const float radius = 50.0f;
    
    for (int i = 0; i < numPoints; ++i) {
        float angle = 2.0f * M_PI * static_cast<float>(i) / (numPoints - 1);
        float x = centerX + radius * std::cos(angle);
        float y = centerY + radius * std::sin(angle);
        stasaiPoints.push_back(cv::Point2f(x, y));
    }
    
    // Process Stasai points with test case identifier
    std::cout << "\nProcessing Stasai points..." << std::endl;
    auto stasaiResult = recognizer.processSimulatedPoints(stasaiPoints, testCaseId + "_STASAI");
    std::cout << "Stasai Result: " << static_cast<int>(stasaiResult.type) 
              << " (Confidence: " << stasaiResult.confidence << ")" << std::endl;
    
    // Verify results
    bool khargailSuccess = (khargailResult.type == TurtleEngine::CSL::GestureType::KHARGAIL);
    bool flammilSuccess = (flammilResult.type == TurtleEngine::CSL::GestureType::FLAMMIL);
    bool stasaiSuccess = (stasaiResult.type == TurtleEngine::CSL::GestureType::STASAI);
    
    std::cout << "\nTest Results:" << std::endl;
    std::cout << "Khargail: " << (khargailSuccess ? "PASS" : "FAIL") << std::endl;
    std::cout << "Flammil: " << (flammilSuccess ? "PASS" : "FAIL") << std::endl;
    std::cout << "Stasai: " << (stasaiSuccess ? "PASS" : "FAIL") << std::endl;
    
    return khargailSuccess && flammilSuccess && stasaiSuccess;
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
    std::cout << "CSLSystem constructed for callback test" << std::endl;

    // Add plasma callback with animation simulation
    cslSystem.addPlasmaCallback([&cslSystem](const TurtleEngine::CSL::GestureResult& result) {
        std::cout << "\n=== Plasma Effect Callback ===" << std::endl;
        std::cout << "Gesture Type: " << static_cast<int>(result.type) << std::endl;
        if (result.type == TurtleEngine::CSL::GestureType::FLAMMIL) {
            std::cout << "*** Plasma Effect Triggered for Flammil! ***" << std::endl;
            std::cout << "End Position: (" << std::fixed << std::setprecision(2) 
                      << result.position.x << "," << result.position.y << ")" << std::endl;
            std::cout << "Confidence: " << std::fixed << std::setprecision(3) << result.confidence << std::endl;
            
            // Log raw velocity data
            std::cout << "\nVelocity Analysis:" << std::endl;
            std::cout << "Total velocity segments: " << result.velocities.size() << std::endl;
            if (!result.velocities.empty()) {
                std::cout << "Raw Velocities (px/s):" << std::endl;
                for (size_t i = 0; i < result.velocities.size(); i += 5) {
                    std::cout << "  Segment " << std::setw(3) << i << ": " 
                             << std::fixed << std::setprecision(3) << std::setw(8)
                             << result.velocities[i] << " px/s" << std::endl;
                }

                // Calculate and display velocity statistics
                float maxVelocity = *std::max_element(result.velocities.begin(), result.velocities.end());
                float avgVelocity = std::accumulate(result.velocities.begin(), result.velocities.end(), 0.0f) / result.velocities.size();
                std::cout << "\nVelocity Statistics:" << std::endl;
                std::cout << "  Max Velocity: " << std::fixed << std::setprecision(3) << std::setw(8) 
                         << maxVelocity << " px/s" << std::endl;
                std::cout << "  Avg Velocity: " << std::fixed << std::setprecision(3) << std::setw(8) 
                         << avgVelocity << " px/s" << std::endl;

                // Scale intensity based on raw velocity (1500 px/s as max for full intensity)
                const float targetMaxVelocity = 1500.0f;
                float intensityScale = (targetMaxVelocity > 1e-6) ? 
                    std::min(1.0f, maxVelocity / targetMaxVelocity) : 0.0f;
                std::cout << "  Intensity Scale: " << std::fixed << std::setprecision(3) << std::setw(8) 
                         << intensityScale << std::endl;
            }
        }
        std::cout << "=== Callback execution finished ===\n" << std::endl;
    });

    // --- Run Test Suite --- 
    bool overallResult = true;
    
    // Test 1: Rapid input (0.15s)
    std::cout << "\n=== Test Case 1: Rapid Input (0.15s) ===" << std::endl;
    cslSystem.setPlasmaDuration(0.15f);
    overallResult &= runComboTest(recognizer, cslSystem, 0.15f, 0.15f, "Test1_0.15s");
    
    // Test 2: Standard duration (0.4s, 0.5s)
    std::cout << "\n=== Test Case 2: Standard Duration (0.4s, 0.5s) ===" << std::endl;
    cslSystem.setPlasmaDuration(0.5f);
    overallResult &= runComboTest(recognizer, cslSystem, 0.4f, 0.5f, "Test2_0.4s-0.5s");
    
    // Test 3: Faster duration (0.25s, 0.3s)
    std::cout << "\n=== Test Case 3: Faster Duration (0.25s, 0.3s) ===" << std::endl;
    cslSystem.setPlasmaDuration(0.3f);
    overallResult &= runComboTest(recognizer, cslSystem, 0.25f, 0.3f, "Test3_0.25s-0.3s");
    
    // Test 4: Slower duration (0.6s, 0.7s)
    std::cout << "\n=== Test Case 4: Slower Duration (0.6s, 0.7s) ===" << std::endl;
    cslSystem.setPlasmaDuration(0.7f);
    overallResult &= runComboTest(recognizer, cslSystem, 0.6f, 0.7f, "Test4_0.6s-0.7s");

    std::cout << "\n=== Test Suite Summary ===" << std::endl;
    if (overallResult) {
        std::cout << "Overall Result: PASSED" << std::endl;
        return 0; // Success exit code
    } else {
        std::cout << "Overall Result: FAILED" << std::endl;
        return 1; // Failure exit code
    }
}