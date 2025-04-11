#include "../include/GestureRecognizer.hpp"
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
#include <numeric>
#include <future>
#include <thread>

// Conditional cout macro
#ifdef ENABLE_GESTURE_DEBUG_LOGGING
#define GESTURE_DEBUG_LOG(x) std::cout << x << std::endl
#else
#define GESTURE_DEBUG_LOG(x) 
#endif

namespace TurtleEngine {
namespace Input {

GestureRecognizer::GestureRecognizer()
    : m_sensitivity(GestureConstants::DEFAULT_SENSITIVITY)
    , m_minConfidence(GestureConstants::DEFAULT_MIN_CONFIDENCE)
    , m_debugMode(false)
    , m_debugLogLevel(0)
{
    m_gestureThresholds[GestureType::KHARGAIL] = GestureConstants::KHARGAIL_THRESHOLD;
    m_gestureThresholds[GestureType::FLAMMIL] = GestureConstants::FLAMMIL_THRESHOLD;
    m_gestureThresholds[GestureType::STASAI] = GestureConstants::STASAI_THRESHOLD;
    m_gestureThresholds[GestureType::ANNIHLAT] = GestureConstants::ANNIHLAT_THRESHOLD;
}

GestureRecognizer::~GestureRecognizer() = default;

void GestureRecognizer::initialize() {
    // Initialize OpenCV-based gesture recognition
    logDebugInfo("Initializing gesture recognition system...", 1);
}

void GestureRecognizer::setSensitivity(float sensitivity) {
    m_sensitivity = std::max(0.0f, std::min(1.0f, sensitivity));
    logDebugInfo("Sensitivity set to: " + std::to_string(m_sensitivity), 2);
}

void GestureRecognizer::setMinConfidence(float minConfidence) {
    m_minConfidence = std::max(0.0f, std::min(1.0f, minConfidence));
    logDebugInfo("Minimum confidence set to: " + std::to_string(m_minConfidence), 2);
}

void GestureRecognizer::setGestureThreshold(GestureType type, float threshold) {
    m_gestureThresholds[type] = std::max(0.0f, std::min(1.0f, threshold));
    logDebugInfo("Gesture threshold for type " + std::to_string(static_cast<int>(type)) + 
                " set to: " + std::to_string(threshold), 2);
}

float GestureRecognizer::getGestureThreshold(GestureType type) const {
    auto it = m_gestureThresholds.find(type);
    return (it != m_gestureThresholds.end()) ? it->second : GestureConstants::DEFAULT_MIN_CONFIDENCE;
}

GestureResult GestureRecognizer::processSimulatedPoints(
    const std::vector<cv::Point2f>& points, const std::string& testId) {
    
    GestureResult result;
    result.type = GestureType::NONE;
    result.confidence = 0.0f;
    result.debugPoints = points;
    
    if (points.empty()) {
        return result;
    }
    
    // Calculate velocities
    result.velocities = calculateRawVelocities(points);
    result.velocities = normalizeVelocities(result.velocities);
    
    // Try to recognize each gesture type
    std::vector<GestureResult> candidates;
    candidates.push_back(recognizeKhargail(points));
    candidates.push_back(recognizeFlammil(points));
    candidates.push_back(recognizeStasai(points, testId));
    candidates.push_back(recognizeAnnihlat(points));
    
    // Find the best match
    for (const auto& candidate : candidates) {
        if (candidate.confidence > result.confidence) {
            result = candidate;
        }
    }
    
    // Generate debug information
    if (m_debugMode) {
        std::string info = "Test ID: " + testId + "\n";
        info += "Points: " + std::to_string(points.size()) + "\n";
        info += "Detected: " + std::to_string(static_cast<int>(result.type)) + "\n";
        info += "Confidence: " + std::to_string(result.confidence);
        result.debugInfo = info;
        
        // Calculate point-wise confidence for visualization
        result.debugConfidences.resize(points.size());
        for (size_t i = 0; i < points.size(); ++i) {
            result.debugConfidences[i] = static_cast<float>(i) / static_cast<float>(points.size() - 1);
        }
    }
    
    return result;
}

GestureResult GestureRecognizer::processFrame(const cv::Mat& frame) {
    GestureResult result;
    // Implement real-time frame processing here
    return result;
}

GestureResult GestureRecognizer::recognizeKhargail(const std::vector<cv::Point2f>& points) {
    GestureResult result;
    result.type = GestureType::NONE;
    
    if (points.size() < 2) {
        return result;
    }
    
    // Calculate basic metrics
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx) * 180.0f / 3.14159265358979323846f;
    
    // Calculate straightness
    float totalLength = 0.0f;
    for (size_t i = 1; i < points.size(); ++i) {
        cv::Point2f diff = points[i] - points[i-1];
        totalLength += std::sqrt(diff.x * diff.x + diff.y * diff.y);
    }
    float straightness = distance / totalLength;
    
    // Check if it's a horizontal charge
    if (straightness > GestureConstants::SWIPE_STRAIGHTNESS_THRESHOLD && 
        std::abs(angle) < GestureConstants::HORIZONTAL_ANGLE_THRESHOLD) {
        result.type = GestureType::KHARGAIL;
        result.confidence = straightness * m_sensitivity;
        result.position = end;
        result.trajectory = points;
    }
    
    return result;
}

GestureResult GestureRecognizer::recognizeFlammil(const std::vector<cv::Point2f>& points) {
    GestureResult result;
    result.type = GestureType::NONE;
    
    if (points.size() < 2) {
        return result;
    }
    
    // Calculate metrics
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx) * 180.0f / 3.14159265358979323846f;
    
    // Calculate straightness
    float totalLength = 0.0f;
    for (size_t i = 1; i < points.size(); ++i) {
        cv::Point2f diff = points[i] - points[i-1];
        totalLength += std::sqrt(diff.x * diff.x + diff.y * diff.y);
    }
    float straightness = distance / totalLength;
    
    // Check if it's a diagonal down-right swipe
    if (straightness > GestureConstants::SWIPE_STRAIGHTNESS_THRESHOLD && 
        angle > GestureConstants::FLAMMIL_ANGLE_MIN && 
        angle < GestureConstants::FLAMMIL_ANGLE_MAX) {
        result.type = GestureType::FLAMMIL;
        result.confidence = straightness * m_sensitivity;
        result.position = end;
        result.trajectory = points;
    }
    
    return result;
}

GestureResult GestureRecognizer::recognizeStasai(
    const std::vector<cv::Point2f>& points, const std::string& testCaseId) {
    
    GestureResult result;
    result.type = GestureType::NONE;
    
    if (points.size() < 3) {
        return result;
    }
    
    // Calculate centroid
    cv::Point2f centroid(0.0f, 0.0f);
    for (const auto& p : points) {
        centroid += p;
    }
    centroid *= (1.0f / points.size());
    
    // Calculate circularity
    float avgRadius = 0.0f;
    float varRadius = 0.0f;
    std::vector<float> radii;
    
    for (const auto& p : points) {
        float r = cv::norm(p - centroid);
        radii.push_back(r);
        avgRadius += r;
    }
    avgRadius /= points.size();
    
    for (float r : radii) {
        float diff = r - avgRadius;
        varRadius += diff * diff;
    }
    varRadius /= points.size();
    
    // Calculate circularity metric
    float circularity = 1.0f / (1.0f + varRadius / (avgRadius * avgRadius));
    
    // Check if it's a circle
    if (circularity > GestureConstants::CIRCULARITY_THRESHOLD) {
        result.type = GestureType::STASAI;
        result.confidence = circularity * m_sensitivity;
        result.position = centroid;
        result.trajectory = points;
        
        if (m_debugMode) {
            result.debugInfo = "Circularity: " + std::to_string(circularity) + 
                             "\nTest case: " + testCaseId;
        }
    }
    
    return result;
}

GestureResult GestureRecognizer::recognizeAnnihlat(const std::vector<cv::Point2f>& points) {
    GestureResult result;
    result.type = GestureType::NONE;
    
    if (points.size() < 2) {
        return result;
    }
    
    // Calculate metrics
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    float dx = end.x - start.x;
    float dy = end.y - start.y;
    float distance = std::sqrt(dx * dx + dy * dy);
    float angle = std::atan2(dy, dx) * 180.0f / 3.14159265358979323846f;
    
    // Calculate straightness
    float totalLength = 0.0f;
    for (size_t i = 1; i < points.size(); ++i) {
        cv::Point2f diff = points[i] - points[i-1];
        totalLength += std::sqrt(diff.x * diff.x + diff.y * diff.y);
    }
    float straightness = distance / totalLength;
    
    // Check if it's an upward diagonal swipe
    if (straightness > GestureConstants::SWIPE_STRAIGHTNESS_THRESHOLD && 
        angle < GestureConstants::ANNIHLAT_ANGLE_MAX && 
        angle > GestureConstants::ANNIHLAT_ANGLE_MIN) {
        result.type = GestureType::ANNIHLAT;
        result.confidence = straightness * m_sensitivity;
        result.position = end;
        result.trajectory = points;
    }
    
    return result;
}

std::vector<float> GestureRecognizer::calculateRawVelocities(
    const std::vector<cv::Point2f>& points) const {
    
    std::vector<float> velocities;
    if (points.size() < 2) return velocities;
    
    velocities.reserve(points.size() - 1);
    for (size_t i = 1; i < points.size(); ++i) {
        cv::Point2f diff = points[i] - points[i-1];
        velocities.push_back(std::sqrt(diff.x * diff.x + diff.y * diff.y));
    }
    
    return velocities;
}

std::vector<float> GestureRecognizer::normalizeVelocities(
    const std::vector<float>& rawVelocities) const {
    
    std::vector<float> normalized = rawVelocities;
    if (normalized.empty()) return normalized;
    
    float maxVel = *std::max_element(normalized.begin(), normalized.end());
    if (maxVel > 0.0f) {
        for (float& v : normalized) {
            v /= maxVel;
        }
    }
    
    return normalized;
}

void GestureRecognizer::logDebugInfo(const std::string& info, int level) {
    if (m_debugMode && level <= m_debugLogLevel) {
        std::cout << "[GestureRecognizer Debug L" << level << "] " << info << std::endl;
    }
}

} // namespace Input
} // namespace TurtleEngine 