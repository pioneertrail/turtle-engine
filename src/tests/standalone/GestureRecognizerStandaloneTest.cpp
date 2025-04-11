#include <iostream>
#include <vector>
#include <string>
#include "../../engine/input/include/GestureRecognizer.hpp"

using namespace TurtleEngine::Input;

// Simple point structure to replace cv::Point2f
struct Point2D {
    float x, y;
    Point2D(float x_, float y_) : x(x_), y(y_) {}
};

// Mock cv::Point2f for compatibility
namespace cv {
    using Point2f = Point2D;
}

int main() {
    std::cout << "=== GestureRecognizer Standalone Test ===\n";

    // Create a GestureRecognizer instance
    GestureRecognizer recognizer;
    recognizer.initialize();

    // Set parameters
    recognizer.setSensitivity(GestureConstants::DEFAULT_SENSITIVITY);
    recognizer.setMinConfidence(GestureConstants::DEFAULT_MIN_CONFIDENCE);
    recognizer.setDebugMode(true);
    recognizer.setDebugLogLevel(2);

    // Test 1: Simulate a Khargail gesture (left-right charge)
    std::cout << "\nTest 1: Simulating Khargail gesture (left-right charge)\n";
    std::vector<cv::Point2f> khargailPoints;
    for (int i = 0; i <= 30; ++i) {
        float t = static_cast<float>(i) / 30.0f;
        float x = 100.0f + t * 400.0f; // Moving from x=100 to x=500
        float y = 300.0f; // Constant y
        khargailPoints.emplace_back(x, y);
    }
    GestureResult khargailResult = recognizer.processSimulatedPoints(khargailPoints, "KhargailTest");
    bool khargailSuccess = (khargailResult.type == GestureType::KHARGAIL && 
                            khargailResult.confidence >= GestureConstants::KHARGAIL_THRESHOLD);
    std::cout << "Khargail Test: " << (khargailSuccess ? "PASSED" : "FAILED") 
              << " (Confidence: " << khargailResult.confidence << ")\n";

    // Test 2: Simulate a Flammil gesture (right-down swipe)
    std::cout << "\nTest 2: Simulating Flammil gesture (right-down swipe)\n";
    std::vector<cv::Point2f> flammilPoints;
    for (int i = 0; i <= 30; ++i) {
        float t = static_cast<float>(i) / 30.0f;
        float x = 100.0f + t * 400.0f; // Moving from x=100 to x=500
        float y = 100.0f + t * 300.0f; // Moving from y=100 to y=400
        flammilPoints.emplace_back(x, y);
    }
    GestureResult flammilResult = recognizer.processSimulatedPoints(flammilPoints, "FlammilTest");
    bool flammilSuccess = (flammilResult.type == GestureType::FLAMMIL && 
                           flammilResult.confidence >= GestureConstants::FLAMMIL_THRESHOLD);
    std::cout << "Flammil Test: " << (flammilSuccess ? "PASSED" : "FAILED") 
              << " (Confidence: " << flammilResult.confidence << ")\n";

    std::cout << "\n=== GestureRecognizer Test Complete ===\n";
    return khargailSuccess && flammilSuccess ? 0 : 1;
} 