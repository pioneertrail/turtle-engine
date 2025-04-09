#include "csl/GestureRecognizer.hpp"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <random>
#include <chrono> // Include chrono for timing

int main() {
    std::cout << "Starting CSLTest" << std::endl;
    TurtleEngine::CSL::GestureRecognizer recognizer;
    std::cout << "GestureRecognizer constructed" << std::endl;
    if (!recognizer.initialize()) {
        std::cerr << "Failed to initialize GestureRecognizer" << std::endl;
        return -1;
    }
    std::cout << "GestureRecognizer initialized" << std::endl;

    // FURTHER SIMPLIFICATION: Remove Point2f usage - RESTORING
    // std::cout << "Skipping Point2f generation and processSimulatedPoints call" << std::endl;
    // /*
    // Simulated Flammil gesture: right-down swipe, 75px, 0.5s
    std::cout << "Generating flammilPoints" << std::endl;
    std::vector<cv::Point2f> flammilPoints;

    // Comment out random generation and OpenCV usage
    /* // KEEP THIS COMMENTED FOR NOW
    std::random_device rd;
    std::cout << "Random device created" << std::endl;
    // ... (rest of original commented block) ...
    std::cout << "Blank frame created" << std::endl;
    */

    // Hardcode a minimal trajectory (RESTORED)
    std::cout << "Using hardcoded trajectory" << std::endl;
    flammilPoints.push_back(cv::Point2f(640.0f, 216.0f));
    flammilPoints.push_back(cv::Point2f(660.0f, 236.0f)); // Add intermediate points
    flammilPoints.push_back(cv::Point2f(680.0f, 256.0f));
    flammilPoints.push_back(cv::Point2f(700.0f, 276.0f));
    flammilPoints.push_back(cv::Point2f(715.0f, 291.0f)); // >= 5 points now
    std::cout << "Hardcoded points added, size: " << flammilPoints.size() << std::endl;

    // Replace direct access and processFrame (RESTORED call)
    auto startTime = std::chrono::high_resolution_clock::now();
    TurtleEngine::CSL::GestureResult result = recognizer.processSimulatedPoints(flammilPoints);
    auto endTime = std::chrono::high_resolution_clock::now();
    float latency = std::chrono::duration<float>(endTime - startTime).count();

    // Output results (RESTORED)
    std::cout << "Flammil Test:" << std::endl;
    std::cout << "Confidence: " << result.confidence << " (Target: >0.75)" << std::endl;
    std::cout << "Latency: " << latency << "s (Target: <0.5)" << std::endl;
    std::cout << "Gesture Type: ";
    switch(result.type) {
        case TurtleEngine::CSL::GestureType::FLAMMIL: std::cout << "FLAMMIL"; break;
        case TurtleEngine::CSL::GestureType::KHARGAIL: std::cout << "KHARGAIL"; break;
        case TurtleEngine::CSL::GestureType::STASAI: std::cout << "STASAI"; break;
        case TurtleEngine::CSL::GestureType::ANNIHLAT: std::cout << "ANNIHLAT"; break;
        case TurtleEngine::CSL::GestureType::NONE: std::cout << "NONE"; break;
        default: std::cout << "Unexpected (" << static_cast<int>(result.type) << ")"; break;
    }
    std::cout << std::endl;
    std::cout << "Trajectory Points: " << result.trajectory.size() << std::endl;
    std::cout << "Last Position: (" << result.position.x << "," << result.position.y << ")" << std::endl;

    // Check if targets met (optional)
    if (result.type == TurtleEngine::CSL::GestureType::FLAMMIL && result.confidence > 0.75f && latency < 0.5f) {
        std::cout << "Validation: PASSED" << std::endl;
        return 0; // Success exit code
    } else {
        std::cout << "Validation: FAILED" << std::endl;
        return 1; // Failure exit code
    }
    // */ // End of outer comment block

    // std::cout << "Test completed (Point2f/processing skipped)." << std::endl;
    // return 0; // Return success for this simplified test
}