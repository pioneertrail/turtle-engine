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
#include <map>

// Define initial map contents (if needed, or initialize empty)
// static const std::map<TurtleEngine::CSL::GestureType, float> initialThresholds = { ... };

namespace TurtleEngine {
namespace CSL {

GestureRecognizer::GestureRecognizer()
    : m_sensitivity(1.2f)
    , m_minConfidence(0.70f)
    , m_averageTransitionLatency(0.0f)
    , m_initialized(false)
    // Initialize maps in initializer list
    , m_gestureThresholds({{GestureType::KHARGAIL, 0.80f}, {GestureType::FLAMMIL, 0.75f}, {GestureType::STASAI, 0.80f}, {GestureType::ANNIHLAT, 0.78f}})
    , m_gestureAttempts({{GestureType::KHARGAIL, 0}, {GestureType::FLAMMIL, 0}, {GestureType::STASAI, 0}, {GestureType::ANNIHLAT, 0}, {GestureType::NONE, 0}, {GestureType::TBD, 0}})
    , m_gestureSuccesses({{GestureType::KHARGAIL, 0}, {GestureType::FLAMMIL, 0}, {GestureType::STASAI, 0}, {GestureType::ANNIHLAT, 0}, {GestureType::NONE, 0}, {GestureType::TBD, 0}})
{
    std::cout << "GestureRecognizer Constructor: Start" << std::endl;
    m_previousPoints.reserve(30);
    std::cout << "GestureRecognizer Constructor: Reserved previousPoints" << std::endl;
    m_lastGesture = {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                     std::chrono::high_resolution_clock::now(), 0.0f};
    std::cout << "GestureRecognizer Constructor: Initialized lastGesture" << std::endl;
    m_lastTransition = {GestureType::NONE, GestureType::NONE, 0.0f, 0.0f};
    std::cout << "GestureRecognizer Constructor: Initialized lastTransition" << std::endl;
    
    // Map initialization moved to initializer list above
    std::cout << "GestureRecognizer Constructor: Maps initialized via list" << std::endl;

    std::cout << "GestureRecognizer Constructor: End" << std::endl;
}

GestureRecognizer::~GestureRecognizer() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

bool GestureRecognizer::initialize() {
    if (m_initialized) {
        return true;
    }
    
    // Create logs directory if it doesn't exist
    std::filesystem::create_directories("logs");
    
    // Open log file - COMMENTED OUT FOR DEBUGGING
    // m_logFile.open("logs/gesture_debug.log", std::ios::app);
    // if (!m_logFile.is_open()) {
    //     std::cerr << "Failed to open gesture debug log file" << std::endl;
    //     return false;
    // }
    
    m_initialized = true;
    return true;
}

GestureResult GestureRecognizer::processFrame(const cv::Mat& frame) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!m_initialized || frame.empty()) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                startTime, 0.0f};
    }

    // Process frame and detect gestures
    GestureResult result = {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                           startTime, 0.0f};
    
    // Try each gesture type with gesture-specific thresholds
    auto khargailResult = recognizeKhargail(m_previousPoints);
    if (khargailResult.confidence > result.confidence) {
        result = khargailResult;
        m_gestureAttempts[GestureType::KHARGAIL]++;
    }
    
    auto flammilResult = recognizeFlammil(m_previousPoints);
    if (flammilResult.confidence > result.confidence) {
        result = flammilResult;
        m_gestureAttempts[GestureType::FLAMMIL]++;
    }
    
    auto stasaiResult = recognizeStasai(m_previousPoints);
    if (stasaiResult.confidence > result.confidence) {
        result = stasaiResult;
        m_gestureAttempts[GestureType::STASAI]++;
    }
    
    auto annihlatResult = recognizeAnnihlat(m_previousPoints);
    if (annihlatResult.confidence > result.confidence) {
        result = annihlatResult;
        m_gestureAttempts[GestureType::ANNIHLAT]++;
    }

    // Update transition stats if a gesture was detected
    if (result.type != GestureType::NONE) {
        float threshold = m_gestureThresholds.count(result.type) ? m_gestureThresholds.at(result.type) : m_minConfidence;
        if (result.confidence >= threshold) {
            if (m_gestureSuccesses.count(result.type)) { m_gestureSuccesses.at(result.type)++; }
            updateTransitionStats(result, m_lastGesture);
            m_lastGesture = result;
        }
    }

    return result;
}

GestureResult GestureRecognizer::processSimulatedPoints(const std::vector<cv::Point2f>& points) {
    std::cout << "Entering processSimulatedPoints, points size: " << points.size() << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!m_initialized || points.empty()) {
        std::cout << "Not initialized or empty points, returning NONE" << std::endl;
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(),
                startTime, 0.0f};
    }

    // Test all gesture types, as processFrame does
    GestureResult result = {GestureType::NONE, 0.0f, cv::Point2f(), points, startTime, 0.0f};
    std::cout << "Starting gesture recognition" << std::endl;

    auto khargailResult = recognizeKhargail(points);
    std::cout << "Khargail confidence: " << khargailResult.confidence << std::endl;
    if (khargailResult.confidence > result.confidence) {
        result = khargailResult;
        m_gestureAttempts[GestureType::KHARGAIL]++;
    }

    auto flammilResult = recognizeFlammil(points);
    std::cout << "Flammil confidence: " << flammilResult.confidence << std::endl;
    if (flammilResult.confidence > result.confidence) {
        result = flammilResult;
        m_gestureAttempts[GestureType::FLAMMIL]++;
    }

    auto stasaiResult = recognizeStasai(points);
    std::cout << "Stasai confidence: " << stasaiResult.confidence << std::endl;
    if (stasaiResult.confidence > result.confidence) {
        result = stasaiResult;
        m_gestureAttempts[GestureType::STASAI]++;
    }

    auto annihlatResult = recognizeAnnihlat(points);
    std::cout << "Annihlat confidence: " << annihlatResult.confidence << std::endl;
    if (annihlatResult.confidence > result.confidence) {
        result = annihlatResult;
        m_gestureAttempts[GestureType::ANNIHLAT]++;
    }

    std::cout << "Finished recognition, result type: " << static_cast<int>(result.type) << std::endl;

    // Apply gesture-specific threshold and update stats
    if (result.type != GestureType::NONE) {
        float threshold = m_gestureThresholds.count(result.type) ? m_gestureThresholds.at(result.type) : m_minConfidence;
        if (result.confidence >= threshold) {
            if (m_gestureSuccesses.count(result.type)) { m_gestureSuccesses.at(result.type)++; }
            updateTransitionStats(result, m_lastGesture);
            result.position = points.back();
            m_lastGesture = result;
        } else {
            result.type = GestureType::NONE;
        }
    } else {
        // Log if no gesture type had any confidence
        // logGestureResult(result); // Already commented
    }

    // Update transition latency in result (using start time of this function)
    auto endTime = std::chrono::high_resolution_clock::now();
    float calculatedLatency = std::chrono::duration<float>(endTime - startTime).count();
    // Note: The GestureResult struct's transitionLatency field is updated by updateTransitionStats
    // We might want to store the processing time separately if needed.
    // result.transitionLatency = calculatedLatency; // This overwrites the combo latency

    return result;
}

GestureResult GestureRecognizer::recognizeKhargail(const std::vector<cv::Point2f>& points) {
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }

    // Calculate direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Khargail is a left-right charge (60px, 0.4s)
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 0.0f));
    
    if (distance >= 60.0f && confidence >= m_minConfidence) {
        return {GestureType::KHARGAIL, confidence, end, points, 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
            std::chrono::high_resolution_clock::now(), 0.0f};
}

GestureResult GestureRecognizer::recognizeFlammil(const std::vector<cv::Point2f>& points) {
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }

    // Calculate direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Flammil is a right-down swipe (75px, 0.5s)
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 1.0f));
    
    std::cout << "Flammil Debug: Distance=" << distance << ", Confidence=" << confidence 
              << ", Start=(" << start.x << "," << start.y << "), End=(" << end.x << "," << end.y << ")" << std::endl;
    
    if (distance >= 75.0f && confidence >= m_minConfidence) {
        return {GestureType::FLAMMIL, confidence, end, points, 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
            std::chrono::high_resolution_clock::now(), 0.0f};
}

GestureResult GestureRecognizer::recognizeStasai(const std::vector<cv::Point2f>& points) {
    if (points.size() < 8) {  // Need enough points for circle detection
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }

    // Stasai is a tight circle (50px, 0.6s)
    if (isCircle(points)) {
        float confidence = 0.8f;  // Base confidence for circle detection
        return {GestureType::STASAI, confidence, points.back(), points, 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
            std::chrono::high_resolution_clock::now(), 0.0f};
}

GestureResult GestureRecognizer::recognizeAnnihlat(const std::vector<cv::Point2f>& points) {
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }

    // Calculate direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Annihlat is a right swipe down
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, -1.0f));
    
    if (distance >= 60.0f && confidence >= m_minConfidence) {
        return {GestureType::ANNIHLAT, confidence, end, points, 
                std::chrono::high_resolution_clock::now(), 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>(), 
            std::chrono::high_resolution_clock::now(), 0.0f};
}

void GestureRecognizer::updateTransitionStats(const GestureResult& current, const GestureResult& previous) {
    if (previous.type != GestureType::NONE) {
        auto now = std::chrono::high_resolution_clock::now();
        float latency = std::chrono::duration<float>(now - previous.timestamp).count();
        
        if (latency > 1.0f) {  // Reset if gap exceeds 1s
            m_lastTransition = {GestureType::NONE, current.type, 0.0f, current.confidence};
            return;
        }
        
        m_lastTransition = {previous.type, current.type, latency, 
                          std::min(previous.confidence, current.confidence)};
        m_averageTransitionLatency = (m_averageTransitionLatency * 0.9f) + (latency * 0.1f);
    }
}

void GestureRecognizer::logGestureResult(const GestureResult& result) {
    if (!m_logFile.is_open()) {
        std::cerr << "Log file not open!" << std::endl;
        // Fallback to console
        std::cout << "[LOG FALLBACK] Gesture: " << static_cast<int>(result.type) 
                  << ", Confidence: " << result.confidence << std::endl;
        return;
    }

    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
    
    // Replace localtime with localtime_s for Windows safety
    std::tm timeInfo;
    localtime_s(&timeInfo, &time);  // Safer alternative to localtime
    std::stringstream ss;
    ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
       << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
       << "Gesture: " << static_cast<int>(result.type) 
       << ", Confidence: " << std::fixed << std::setprecision(2) << result.confidence
       << ", Position: (" << result.position.x << "," << result.position.y << ")"
       << ", Latency: " << result.transitionLatency << "s"
       << ", Attempts: " << (m_gestureAttempts.count(result.type) ? m_gestureAttempts.at(result.type) : 0)
       << ", Success Rate: " << std::fixed << std::setprecision(2) 
       << (m_gestureAttempts.count(result.type) && m_gestureAttempts.at(result.type) > 0 ? 
           (static_cast<float>(m_gestureSuccesses.count(result.type) ? m_gestureSuccesses.at(result.type) : 0) / 
            static_cast<float>(m_gestureAttempts.at(result.type)) * 100.0f) : 0.0f) << "%"
       << ", Avg Transition Latency: " << std::fixed << std::setprecision(2) 
       << m_averageTransitionLatency << "s"
       << std::endl;
    
    m_logFile << ss.str();
    m_logFile.flush(); // Force flush to ensure data is written
}

void GestureRecognizer::resetTransitionStats() {
    m_lastTransition = {GestureType::NONE, GestureType::NONE, 0.0f, 0.0f};
    m_averageTransitionLatency = 0.0f;
    
    for (auto const& [key, val] : m_gestureAttempts) {
        m_gestureAttempts[key] = 0;
    }
    for (auto const& [key, val] : m_gestureSuccesses) {
        m_gestureSuccesses[key] = 0;
    }
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

void GestureRecognizer::setSensitivity(float sensitivity) {
    m_sensitivity = std::max(0.1f, std::min(2.0f, sensitivity));
}

void GestureRecognizer::setMinConfidence(float confidence) {
    m_minConfidence = std::max(0.1f, std::min(1.0f, confidence));
}

} // namespace CSL
} // namespace TurtleEngine 