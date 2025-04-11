#include "../include/GestureRecognizer.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>
#include <cmath>
#include <numeric>
#define _USE_MATH_DEFINES

namespace TurtleEngine {
namespace Input {
namespace Test {

// Test function to validate gesture recognition accuracy
void testGestureRecognition() {
    std::cout << "Starting Gesture Recognition Accuracy Test..." << std::endl;
    
    // Create a gesture recognizer
    GestureRecognizer recognizer;
    recognizer.initialize();
    
    // Set sensitivity and confidence thresholds
    recognizer.setSensitivity(1.0f);
    recognizer.setMinConfidence(0.7f);
    recognizer.setDebugMode(true);
    recognizer.setDebugLogLevel(2);

    // Set gesture-specific thresholds from main branch
    recognizer.setGestureThreshold(GestureType::KHARGAIL, 0.75f);
    recognizer.setGestureThreshold(GestureType::FLAMMIL, 0.80f);
    recognizer.setGestureThreshold(GestureType::STASAI, 0.85f);
    recognizer.setGestureThreshold(GestureType::ANNIHLAT, 0.70f);
    
    // Create windows for visualization
    cv::namedWindow("Gesture Recognition Test", cv::WINDOW_NORMAL);
    cv::namedWindow("Debug Visualization", cv::WINDOW_NORMAL);
    cv::resizeWindow("Gesture Recognition Test", 800, 600);
    cv::resizeWindow("Debug Visualization", 800, 600);
    
    // Test parameters
    const int numTests = 10;
    const int numGestures = 4; // KHARGAIL, FLAMMIL, STASAI, ANNIHLAT
    std::vector<int> correctDetections(numGestures, 0);
    std::vector<std::vector<float>> confidenceScores(numGestures);
    
    // Create blank images for testing
    cv::Mat testImage(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    cv::Mat debugImage(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    
    // Function to generate a simulated gesture trajectory
    auto generateGestureTrajectory = [](GestureType type, int width, int height) -> std::vector<cv::Point2f> {
        std::vector<cv::Point2f> points;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> noise(-10.0f, 10.0f);
        
        switch (type) {
            case GestureType::KHARGAIL: {
                // Generate a left-right charge
                float startX = width * 0.2f;
                float startY = height * 0.5f;
                float endX = width * 0.8f;
                float endY = height * 0.5f;
                
                for (int i = 0; i <= 30; ++i) {
                    float t = static_cast<float>(i) / 30.0f;
                    float x = startX + t * (endX - startX);
                    float y = startY + t * (endY - startY);
                    points.push_back(cv::Point2f(x + noise(gen), y + noise(gen)));
                }
                break;
            }
            case GestureType::FLAMMIL: {
                // Generate a right-down swipe
                float startX = width * 0.2f;
                float startY = height * 0.3f;
                float endX = width * 0.8f;
                float endY = height * 0.7f;
                
                constexpr float pi = 3.14159265358979323846f;
                for (int i = 0; i <= 30; ++i) {
                    float t = static_cast<float>(i) / 30.0f;
                    float x = startX + t * (endX - startX);
                    float y = startY + t * (endY - startY);
                    points.push_back(cv::Point2f(x + noise(gen), y + noise(gen)));
                }
                break;
            }
            case GestureType::STASAI: {
                // Generate a tight circle
                float centerX = width * 0.5f;
                float centerY = height * 0.5f;
                float radius = std::min(width, height) * 0.15f;
                
                constexpr float pi = 3.14159265358979323846f;
                for (int i = 0; i <= 30; ++i) {
                    float angle = static_cast<float>(i) * 2.0f * pi / static_cast<float>(30);
                    float x = centerX + radius * std::cos(angle);
                    float y = centerY + radius * std::sin(angle);
                    points.push_back(cv::Point2f(x + noise(gen), y + noise(gen)));
                }
                break;
            }
            case GestureType::ANNIHLAT: {
                // Generate a right swipe up
                float startX = width * 0.2f;
                float startY = height * 0.7f;
                float endX = width * 0.8f;
                float endY = height * 0.3f;
                
                for (int i = 0; i <= 30; ++i) {
                    float t = static_cast<float>(i) / 30.0f;
                    float x = startX + t * (endX - startX);
                    float y = startY + t * (endY - startY);
                    points.push_back(cv::Point2f(x + noise(gen), y + noise(gen)));
                }
                break;
            }
            default:
                break;
        }
        
        return points;
    };
    
    // Function to draw a gesture trajectory
    auto drawGestureTrajectory = [](cv::Mat& image, const std::vector<cv::Point2f>& points, 
                                   const std::vector<float>& confidences = std::vector<float>()) {
        // Draw trajectory lines
        for (size_t i = 1; i < points.size(); ++i) {
            cv::Scalar color;
            if (!confidences.empty()) {
                // Color based on confidence (red->yellow->green)
                float conf = confidences[i-1];
                color = cv::Scalar(0, 255 * conf, 255 * (1.0f - conf));
            } else {
                color = cv::Scalar(0, 255, 0);
            }
            cv::line(image, points[i-1], points[i], color, 2);
        }
        
        // Draw current position
        if (!points.empty()) {
            cv::circle(image, points.back(), 5, cv::Scalar(0, 0, 255), -1);
        }
    };
    
    // Run tests for each gesture type
    for (int gestureIdx = 0; gestureIdx < numGestures; ++gestureIdx) {
        GestureType gestureType;
        std::string gestureName;
        
        switch (gestureIdx) {
            case 0:
                gestureType = GestureType::KHARGAIL;
                gestureName = "KHARGAIL (Charge)";
                break;
            case 1:
                gestureType = GestureType::FLAMMIL;
                gestureName = "FLAMMIL (Flame)";
                break;
            case 2:
                gestureType = GestureType::STASAI;
                gestureName = "STASAI (Circle)";
                break;
            case 3:
                gestureType = GestureType::ANNIHLAT;
                gestureName = "ANNIHLAT (Annihilate)";
                break;
            default:
                continue;
        }
        
        std::cout << "Testing " << gestureName << " recognition..." << std::endl;
        
        // Run multiple tests for this gesture type
        for (int testIdx = 0; testIdx < numTests; ++testIdx) {
            // Generate a simulated gesture trajectory
            std::vector<cv::Point2f> trajectory = generateGestureTrajectory(gestureType, testImage.cols, testImage.rows);
            
            // Process the frame with the gesture recognizer
            GestureResult result = recognizer.processSimulatedPoints(trajectory, 
                gestureName + "_Test" + std::to_string(testIdx));
            
            // Create visualization frames
            testImage = cv::Scalar(0, 0, 0);
            debugImage = cv::Scalar(0, 0, 0);
            
            // Draw trajectories
            drawGestureTrajectory(testImage, trajectory);
            drawGestureTrajectory(debugImage, result.debugPoints, result.debugConfidences);
            
            // Check if the gesture was correctly recognized
            bool correct = (result.type == gestureType);
            if (correct) {
                correctDetections[gestureIdx]++;
                confidenceScores[gestureIdx].push_back(result.confidence);
            }
            
            // Display results
            std::string resultText = "Test " + std::to_string(testIdx + 1) + "/" + std::to_string(numTests) + 
                                   ": " + gestureName + " - " + (correct ? "CORRECT" : "INCORRECT") + 
                                   " (Confidence: " + std::to_string(static_cast<int>(result.confidence * 100)) + "%)";
            
            cv::putText(testImage, resultText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
            cv::putText(debugImage, "Debug View: " + result.debugInfo, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
            
            cv::imshow("Gesture Recognition Test", testImage);
            cv::imshow("Debug Visualization", debugImage);
            
            cv::waitKey(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Slow down for visibility
        }
    }
    
    // Calculate and display the results
    std::cout << "\nGesture Recognition Test Results:" << std::endl;
    std::cout << "=================================" << std::endl;
    
    for (int gestureIdx = 0; gestureIdx < numGestures; ++gestureIdx) {
        std::string gestureName;
        switch (gestureIdx) {
            case 0:
                gestureName = "KHARGAIL (Charge)";
                break;
            case 1:
                gestureName = "FLAMMIL (Flame)";
                break;
            case 2:
                gestureName = "STASAI (Circle)";
                break;
            case 3:
                gestureName = "ANNIHLAT (Annihilate)";
                break;
            default:
                continue;
        }
        
        float accuracy = static_cast<float>(correctDetections[gestureIdx]) / numTests * 100.0f;
        float avgConfidence = 0.0f;
        
        if (!confidenceScores[gestureIdx].empty()) {
            avgConfidence = std::accumulate(confidenceScores[gestureIdx].begin(), 
                                         confidenceScores[gestureIdx].end(), 0.0f) / 
                                         confidenceScores[gestureIdx].size();
        }
        
        std::cout << gestureName << ":" << std::endl;
        std::cout << "  Accuracy: " << accuracy << "% (" << correctDetections[gestureIdx] 
                  << "/" << numTests << ")" << std::endl;
        std::cout << "  Average Confidence: " << (avgConfidence * 100.0f) << "%" << std::endl;
        
        if (correctDetections[gestureIdx] < numTests) {
            std::cout << "  Deviations: " << (numTests - correctDetections[gestureIdx]) 
                      << " tests failed" << std::endl;
        }
        std::cout << std::endl;
    }
    
    cv::destroyAllWindows();
}

} // namespace Test
} // namespace Input
} // namespace TurtleEngine 