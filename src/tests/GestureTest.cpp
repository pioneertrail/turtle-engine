#include "csl/GestureRecognizer.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <vector>
#include <random>

namespace TurtleEngine {
namespace CSL {
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
    
    // Create a window to display the test
    cv::namedWindow("Gesture Recognition Test", cv::WINDOW_NORMAL);
    cv::resizeWindow("Gesture Recognition Test", 800, 600);
    
    // Test parameters
    const int numTests = 10;
    const int numGestures = 3; // SWIPE_RIGHT, CIRCLE, TAP
    std::vector<int> correctDetections(numGestures, 0);
    std::vector<std::vector<float>> confidenceScores(numGestures);
    
    // Create a blank image for testing
    cv::Mat testImage(480, 640, CV_8UC3, cv::Scalar(0, 0, 0));
    
    // Function to generate a simulated gesture trajectory
    auto generateGestureTrajectory = [](GestureType type, int width, int height) -> std::vector<cv::Point2f> {
        std::vector<cv::Point2f> points;
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<float> noise(-10.0f, 10.0f);
        
        switch (type) {
            case GestureType::SWIPE_RIGHT: {
                // Generate a right swipe with some downward component (for "annihilate")
                float startX = width * 0.2f;
                float startY = height * 0.5f;
                float endX = width * 0.8f;
                float endY = height * 0.7f; // Downward component
                
                for (int i = 0; i <= 30; ++i) {
                    float t = static_cast<float>(i) / 30.0f;
                    float x = startX + t * (endX - startX);
                    float y = startY + t * (endY - startY);
                    points.push_back(cv::Point2f(x + noise(gen), y + noise(gen)));
                }
                break;
            }
            case GestureType::CIRCLE: {
                // Generate a circle
                float centerX = width * 0.5f;
                float centerY = height * 0.5f;
                float radius = std::min(width, height) * 0.2f;
                
                for (int i = 0; i <= 30; ++i) {
                    float angle = static_cast<float>(i) / 30.0f * 2.0f * M_PI;
                    float x = centerX + radius * std::cos(angle);
                    float y = centerY + radius * std::sin(angle);
                    points.push_back(cv::Point2f(x + noise(gen), y + noise(gen)));
                }
                break;
            }
            case GestureType::TAP: {
                // Generate a tap (small movement)
                float x = width * 0.5f;
                float y = height * 0.5f;
                
                for (int i = 0; i <= 5; ++i) {
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
    auto drawGestureTrajectory = [](cv::Mat& image, const std::vector<cv::Point2f>& points) {
        for (size_t i = 1; i < points.size(); ++i) {
            cv::line(image, points[i-1], points[i], cv::Scalar(0, 255, 0), 2);
        }
        
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
                gestureType = GestureType::SWIPE_RIGHT;
                gestureName = "SWIPE_RIGHT (Annihilate)";
                break;
            case 1:
                gestureType = GestureType::CIRCLE;
                gestureName = "CIRCLE";
                break;
            case 2:
                gestureType = GestureType::TAP;
                gestureName = "TAP";
                break;
            default:
                continue;
        }
        
        std::cout << "Testing " << gestureName << " recognition..." << std::endl;
        
        // Run multiple tests for this gesture type
        for (int testIdx = 0; testIdx < numTests; ++testIdx) {
            // Generate a simulated gesture trajectory
            std::vector<cv::Point2f> trajectory = generateGestureTrajectory(gestureType, testImage.cols, testImage.rows);
            
            // Create a simulated frame with the trajectory
            testImage = cv::Scalar(0, 0, 0);
            drawGestureTrajectory(testImage, trajectory);
            
            // Process the frame with the gesture recognizer
            GestureResult result = recognizer.processFrame(testImage);
            
            // Check if the gesture was correctly recognized
            bool correct = (result.type == gestureType);
            if (correct) {
                correctDetections[gestureIdx]++;
                confidenceScores[gestureIdx].push_back(result.confidence);
            }
            
            // Display the result
            std::string resultText = "Test " + std::to_string(testIdx + 1) + "/" + std::to_string(numTests) + 
                                    ": " + gestureName + " - " + (correct ? "CORRECT" : "INCORRECT") + 
                                    " (Confidence: " + std::to_string(static_cast<int>(result.confidence * 100)) + "%)";
            
            cv::putText(testImage, resultText, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
            cv::imshow("Gesture Recognition Test", testImage);
            
            // Wait for a key press
            cv::waitKey(0);
        }
    }
    
    // Calculate and display the results
    std::cout << "\nGesture Recognition Test Results:" << std::endl;
    std::cout << "=================================" << std::endl;
    
    for (int gestureIdx = 0; gestureIdx < numGestures; ++gestureIdx) {
        std::string gestureName;
        switch (gestureIdx) {
            case 0:
                gestureName = "SWIPE_RIGHT (Annihilate)";
                break;
            case 1:
                gestureName = "CIRCLE";
                break;
            case 2:
                gestureName = "TAP";
                break;
            default:
                continue;
        }
        
        float accuracy = static_cast<float>(correctDetections[gestureIdx]) / numTests * 100.0f;
        float avgConfidence = 0.0f;
        
        if (!confidenceScores[gestureIdx].empty()) {
            for (float score : confidenceScores[gestureIdx]) {
                avgConfidence += score;
            }
            avgConfidence /= confidenceScores[gestureIdx].size();
        }
        
        std::cout << gestureName << ":" << std::endl;
        std::cout << "  Accuracy: " << accuracy << "% (" << correctDetections[gestureIdx] << "/" << numTests << ")" << std::endl;
        std::cout << "  Average Confidence: " << (avgConfidence * 100.0f) << "%" << std::endl;
        
        // Log deviations for review
        if (correctDetections[gestureIdx] < numTests) {
            std::cout << "  Deviations: " << (numTests - correctDetections[gestureIdx]) << " tests failed" << std::endl;
        }
    }
    
    // Save the results to a file
    std::ofstream resultsFile("gesture_recognition_results.txt");
    if (resultsFile.is_open()) {
        resultsFile << "Gesture Recognition Test Results" << std::endl;
        resultsFile << "===============================" << std::endl;
        
        for (int gestureIdx = 0; gestureIdx < numGestures; ++gestureIdx) {
            std::string gestureName;
            switch (gestureIdx) {
                case 0:
                    gestureName = "SWIPE_RIGHT (Annihilate)";
                    break;
                case 1:
                    gestureName = "CIRCLE";
                    break;
                case 2:
                    gestureName = "TAP";
                    break;
                default:
                    continue;
            }
            
            float accuracy = static_cast<float>(correctDetections[gestureIdx]) / numTests * 100.0f;
            float avgConfidence = 0.0f;
            
            if (!confidenceScores[gestureIdx].empty()) {
                for (float score : confidenceScores[gestureIdx]) {
                    avgConfidence += score;
                }
                avgConfidence /= confidenceScores[gestureIdx].size();
            }
            
            resultsFile << gestureName << ":" << std::endl;
            resultsFile << "  Accuracy: " << accuracy << "% (" << correctDetections[gestureIdx] << "/" << numTests << ")" << std::endl;
            resultsFile << "  Average Confidence: " << (avgConfidence * 100.0f) << "%" << std::endl;
            
            if (correctDetections[gestureIdx] < numTests) {
                resultsFile << "  Deviations: " << (numTests - correctDetections[gestureIdx]) << " tests failed" << std::endl;
            }
        }
        
        resultsFile.close();
        std::cout << "\nResults saved to 'gesture_recognition_results.txt'" << std::endl;
    }
    
    std::cout << "\nGesture Recognition Test completed." << std::endl;
    cv::destroyAllWindows();
}

} // namespace Test
} // namespace CSL
} // namespace TurtleEngine 