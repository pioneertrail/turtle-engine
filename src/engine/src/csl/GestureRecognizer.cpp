#include "csl/GestureRecognizer.hpp"
#include <opencv2/imgproc.hpp>
#include <opencv2/video/tracking.hpp>
#include <cmath>
#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <filesystem>

namespace TurtleEngine {
namespace CSL {

GestureRecognizer::GestureRecognizer()
    : m_sensitivity(1.1f)  // Increased from 1.0f to 1.1f for testing as requested by Anya
                           // Will be evaluated at 10:00 AM UTC - if confidence < 0.7f for any strike,
                           // revert to 1.0f by 11:00 AM UTC and update this comment
                           // Testing process:
                           // 1. Run CSLTest with three "Annihlat" gestures
                           // 2. If ANY confidence < 0.7f, revert to 1.0f by 11:00 AM UTC
                           // 3. Add comment: "Reverted post-test due to low confidence"
                           // 4. Log decision in gesture_recognition_results.txt
    , m_minConfidence(0.7f)
    , m_lastTapTime(0.0)
    , m_initialized(false)
{
    m_previousPoints.reserve(30); // Store up to 30 points for gesture recognition
}

GestureRecognizer::~GestureRecognizer() {
    // Clean up any resources if needed
}

bool GestureRecognizer::initialize() {
    if (m_initialized) {
        return true;
    }
    
    m_initialized = true;
    return true;
}

GestureResult GestureRecognizer::processFrame(const cv::Mat& frame) {
    // Record start time for latency tracking
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!m_initialized || frame.empty()) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>()};
    }
    
    // Convert frame to grayscale for motion detection
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    
    // Apply Gaussian blur to reduce noise
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    
    // Detect motion using frame differencing
    cv::Mat motionMask;
    static cv::Mat previousFrame;  // Static to persist between calls
    if (!previousFrame.empty()) {
        cv::absdiff(gray, previousFrame, motionMask);
        cv::threshold(motionMask, motionMask, 25, 255, cv::THRESH_BINARY);
    }
    
    // Find contours in the motion mask
    std::vector<std::vector<cv::Point>> contours;
    if (!motionMask.empty()) {
        cv::findContours(motionMask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    }
    
    // Find the largest contour (likely the hand)
    cv::Point2f center;
    if (!contours.empty()) {
        auto largestContour = std::max_element(contours.begin(), contours.end(),
            [](const auto& c1, const auto& c2) {
                return cv::contourArea(c1) < cv::contourArea(c2);
            });
        
        if (largestContour != contours.end()) {
            // Calculate the center of the contour
            cv::Moments m = cv::moments(*largestContour);
            if (m.m00 != 0) {
                center = cv::Point2f(m.m10 / m.m00, m.m01 / m.m00);
                
                // Add the point to our trajectory
                m_previousPoints.push_back(center);
                
                // Keep only the most recent points (last 30 frames)
                if (m_previousPoints.size() > 30) {
                    m_previousPoints.erase(m_previousPoints.begin());
                }
            }
        }
    }
    
    // Store the current frame for the next iteration
    gray.copyTo(previousFrame);
    
    // If we have enough points, try to recognize gestures
    GestureResult result = {GestureType::NONE, 0.0f, center, m_previousPoints};
    
    if (m_previousPoints.size() >= 5) {
        // Try to recognize the "Annihlat" gesture first (swipe right)
        GestureResult swipeResult = recognizeSwipe(m_previousPoints);
        if (swipeResult.confidence > m_minConfidence) {
            result = swipeResult;
        }
        
        // Try to recognize a circle gesture
        GestureResult circleResult = recognizeCircle(m_previousPoints);
        if (circleResult.confidence > m_minConfidence && circleResult.confidence > result.confidence) {
            result = circleResult;
        }
        
        // Try to recognize a tap gesture
        GestureResult tapResult = recognizeTap(m_previousPoints);
        if (tapResult.confidence > m_minConfidence && tapResult.confidence > result.confidence) {
            result = tapResult;
        }
        
        // Debug overlay - log gesture confidence live with timestamp
        if (result.type != GestureType::NONE) {
            // Calculate processing time for latency tracking
            auto endTime = std::chrono::high_resolution_clock::now();
            auto processingTime = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count() / 1000.0f;
            
            // Get current time for timestamp
            auto now = std::chrono::system_clock::now();
            auto nowTimeT = std::chrono::system_clock::to_time_t(now);
            auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            
            std::stringstream ss;
            ss << std::put_time(std::localtime(&nowTimeT), "%H:%M:%S") << '.' 
               << std::setfill('0') << std::setw(3) << nowMs.count();
            
            std::string gestureName;
            switch (result.type) {
                case GestureType::SWIPE_RIGHT:
                    gestureName = "Annihlat";
                    break;
                case GestureType::SWIPE_LEFT:
                    gestureName = "Khargail";
                    break;
                case GestureType::SWIPE_UP:
                    gestureName = "Stasai";
                    break;
                case GestureType::SWIPE_DOWN:
                    gestureName = "Flammil";
                    break;
                case GestureType::CIRCLE:
                    gestureName = "Stasai";
                    break;
                case GestureType::TAP:
                    gestureName = "Tap";
                    break;
                case GestureType::DOUBLE_TAP:
                    gestureName = "Double Tap";
                    break;
                default:
                    gestureName = "Unknown";
                    break;
            }
            
            // Format the log message
            std::string logMessage = "[" + ss.str() + "] Gesture: " + gestureName + 
                                    ", Confidence: " + std::to_string(result.confidence) + 
                                    ", Position: (" + std::to_string(result.position.x) + ", " + 
                                    std::to_string(result.position.y) + ")" +
                                    ", Latency: " + std::to_string(processingTime) + "ms";
            
            // Log to console
            std::cout << logMessage << std::endl;
            
            // Log to file
            try {
                // Ensure the logs directory exists
                std::filesystem::create_directories("logs");
                
                // Open the log file in append mode
                std::ofstream logFile("logs/gesture_debug.log", std::ios::app);
                if (logFile.is_open()) {
                    logFile << logMessage << std::endl;
                    logFile.close();
                } else {
                    std::cerr << "Failed to open gesture_debug.log for writing" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error writing to log file: " << e.what() << std::endl;
            }
        }
    }
    
    return result;
}

GestureResult GestureRecognizer::recognizeSwipe(const std::vector<cv::Point2f>& points) {
    if (points.size() < 2) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), points};
    }

    // Calculate the direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Calculate the magnitude of the swipe
    float magnitude = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    
    // Calculate the normalized direction
    cv::Point2f normalizedDirection;
    if (magnitude > 0) {
        normalizedDirection = direction / magnitude;
    } else {
        return {GestureType::NONE, 0.0f, end, points};
    }
    
    // Determine the swipe direction based on the normalized direction
    GestureType swipeType = GestureType::NONE;
    float confidence = 0.0f;
    
    // Tuned parameters for "Annihlat" gesture based on Anya's keyframe specifications
    // Keyframe 0.0s: Arms raised (x: 20% width, y: 20% height)
    // Keyframe 0.3s: Right arm sweeps down (x: 60% width, y: 50% height)
    // Keyframe 0.5s: Fists meet (x: 50% width, y: 70% height), 75-pixel arc total
    
    // Add timing check - ensure gesture duration is within expected range (0.4s to 0.6s)
    float frameTime = 0.016f; // 16ms per frame (~60 FPS)
    float duration = points.size() * frameTime;
    if (duration < 0.4f || duration > 0.6f) {
        return {GestureType::NONE, 0.0f, end, points};
    }
    
    // Check for right swipe (Annihlat gesture)
    if (normalizedDirection.x > 0.3f) { // Significant rightward component
        // Check for downward component (y > 0.45f for downward heft) - increased from 0.4f
        if (normalizedDirection.y > 0.45f) {
            // Calculate confidence based on magnitude and direction
            // Using 75-pixel arc as reference from keyframes
            float magnitudeConfidence = std::min(1.0f, magnitude / 75.0f);
            float directionConfidence = (normalizedDirection.x + normalizedDirection.y) / 2.0f;
            confidence = magnitudeConfidence * directionConfidence;
            
            // Apply sensitivity adjustment
            confidence *= m_sensitivity;
            
            if (confidence >= m_minConfidence) {
                swipeType = GestureType::SWIPE_RIGHT;
            }
        }
    }
    // Check for left swipe (Khargail gesture)
    else if (normalizedDirection.x < -0.3f) {
        confidence = std::abs(normalizedDirection.x) * (magnitude / 50.0f);
        confidence *= m_sensitivity;
        
        if (confidence >= m_minConfidence) {
            swipeType = GestureType::SWIPE_LEFT;
        }
    }
    // Check for upward swipe (Stasai gesture)
    else if (normalizedDirection.y < -0.3f) {
        confidence = std::abs(normalizedDirection.y) * (magnitude / 50.0f);
        confidence *= m_sensitivity;
        
        if (confidence >= m_minConfidence) {
            swipeType = GestureType::SWIPE_UP;
        }
    }
    // Check for downward swipe (Flammil gesture)
    else if (normalizedDirection.y > 0.3f) {
        confidence = std::abs(normalizedDirection.y) * (magnitude / 50.0f);
        confidence *= m_sensitivity;
        
        if (confidence >= m_minConfidence) {
            swipeType = GestureType::SWIPE_DOWN;
        }
    }
    
    return {swipeType, confidence, end, points};
}

GestureResult GestureRecognizer::recognizeCircle(const std::vector<cv::Point2f>& points) {
    if (points.size() < 10) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), points};
    }
    
    // Check if the points form a circle
    if (isCircle(points)) {
        // Calculate the center of the circle
        cv::Point2f center(0, 0);
        for (const auto& point : points) {
            center += point;
        }
        center /= static_cast<float>(points.size());
        
        return {GestureType::CIRCLE, 0.8f, center, points};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), points};
}

GestureResult GestureRecognizer::recognizeTap(const std::vector<cv::Point2f>& points) {
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), points};
    }
    
    // Check if the points form a tap
    if (isTap(points)) {
        // Get the position of the tap
        cv::Point2f position = points.back();
        
        // Check if this is a double tap
        double currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count() / 1000.0;
        
        if (currentTime - m_lastTapTime < 0.5 && 
            std::abs(position.x - m_lastTapPosition.x) < 30.0f &&
            std::abs(position.y - m_lastTapPosition.y) < 30.0f) {
            // This is a double tap
            m_lastTapTime = 0.0; // Reset the last tap time
            return {GestureType::DOUBLE_TAP, 0.9f, position, points};
        } else {
            // This is a single tap
            m_lastTapTime = currentTime;
            m_lastTapPosition = position;
            return {GestureType::TAP, 0.8f, position, points};
        }
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), points};
}

float GestureRecognizer::calculateSwipeConfidence(const std::vector<cv::Point2f>& points, const cv::Point2f& direction) {
    if (points.size() < 5) {
        return 0.0f;
    }
    
    // Calculate how straight the swipe is
    float totalDeviation = 0.0f;
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    float totalDistance = std::sqrt(
        (end.x - start.x) * (end.x - start.x) + 
        (end.y - start.y) * (end.y - start.y));
    
    if (totalDistance < 1.0f) {
        return 0.0f;
    }
    
    // Calculate the expected position for each point along the line
    for (size_t i = 1; i < points.size() - 1; ++i) {
        float t = static_cast<float>(i) / static_cast<float>(points.size() - 1);
        cv::Point2f expectedPoint = start + t * (end - start);
        
        // Calculate the deviation from the expected point
        float deviation = std::sqrt(
            (points[i].x - expectedPoint.x) * (points[i].x - expectedPoint.x) + 
            (points[i].y - expectedPoint.y) * (points[i].y - expectedPoint.y));
        
        totalDeviation += deviation;
    }
    
    // Calculate the average deviation
    float averageDeviation = totalDeviation / static_cast<float>(points.size() - 2);
    
    // Calculate the confidence based on the average deviation
    // The smaller the deviation, the higher the confidence
    float confidence = 1.0f - std::min(1.0f, averageDeviation / (50.0f * m_sensitivity));
    
    return confidence;
}

bool GestureRecognizer::isCircle(const std::vector<cv::Point2f>& points) {
    if (points.size() < 10) {
        return false;
    }
    
    // Calculate the center of the points
    cv::Point2f center(0, 0);
    for (const auto& point : points) {
        center += point;
    }
    center /= static_cast<float>(points.size());
    
    // Calculate the average radius
    float avgRadius = 0.0f;
    for (const auto& point : points) {
        avgRadius += std::sqrt(
            (point.x - center.x) * (point.x - center.x) + 
            (point.y - center.y) * (point.y - center.y));
    }
    avgRadius /= static_cast<float>(points.size());
    
    // Calculate the standard deviation of the radius
    float radiusStdDev = 0.0f;
    for (const auto& point : points) {
        float radius = std::sqrt(
            (point.x - center.x) * (point.x - center.x) + 
            (point.y - center.y) * (point.y - center.y));
        radiusStdDev += (radius - avgRadius) * (radius - avgRadius);
    }
    radiusStdDev = std::sqrt(radiusStdDev / static_cast<float>(points.size()));
    
    // Check if the standard deviation is small enough (indicating a circle)
    return radiusStdDev < 30.0f * m_sensitivity;
}

bool GestureRecognizer::isTap(const std::vector<cv::Point2f>& points) {
    if (points.size() < 3) {
        return false;
    }
    
    // Calculate the total distance traveled
    float totalDistance = 0.0f;
    for (size_t i = 1; i < points.size(); ++i) {
        totalDistance += std::sqrt(
            (points[i].x - points[i-1].x) * (points[i].x - points[i-1].x) + 
            (points[i].y - points[i-1].y) * (points[i].y - points[i-1].y));
    }
    
    // A tap should have a small total distance
    return totalDistance < 30.0f * m_sensitivity;
}

void GestureRecognizer::setSensitivity(float sensitivity) {
    m_sensitivity = std::max(0.1f, std::min(2.0f, sensitivity));
}

void GestureRecognizer::setMinConfidence(float confidence) {
    m_minConfidence = std::max(0.1f, std::min(1.0f, confidence));
}

} // namespace CSL
} // namespace TurtleEngine 