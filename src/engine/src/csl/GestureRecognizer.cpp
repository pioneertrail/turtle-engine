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
    , m_gestureThresholds({{GestureType::KHARGAIL, 0.78f}, {GestureType::FLAMMIL, 0.75f}, {GestureType::STASAI, 0.80f}, {GestureType::ANNIHLAT, 0.78f}})
    , m_gestureAttempts({{GestureType::KHARGAIL, 0}, {GestureType::FLAMMIL, 0}, {GestureType::STASAI, 0}, {GestureType::ANNIHLAT, 0}, {GestureType::NONE, 0}, {GestureType::TBD, 0}})
    , m_gestureSuccesses({{GestureType::KHARGAIL, 0}, {GestureType::FLAMMIL, 0}, {GestureType::STASAI, 0}, {GestureType::ANNIHLAT, 0}, {GestureType::NONE, 0}, {GestureType::TBD, 0}})
{
    std::cout << "GestureRecognizer Constructor: Start" << std::endl;
    m_previousPoints.reserve(30);
    std::cout << "GestureRecognizer Constructor: Reserved previousPoints" << std::endl;
    auto now = std::chrono::high_resolution_clock::now();
    m_lastGesture = {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, now, now, 0.0f};
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
    std::cout << "Initialize: Checking/Creating logs directory..." << std::endl;
    std::filesystem::create_directories("logs");
    std::cout << "Initialize: Logs directory created or exists." << std::endl;
    
    // Open log file
    std::cout << "Initialize: Opening log file logs/gesture_debug.log..." << std::endl;
    m_logFile.open("logs/gesture_debug.log", std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Initialize: ERROR - Failed to open gesture debug log file!" << std::endl;
        return false;
    }
    std::cout << "Initialize: Log file opened successfully." << std::endl;
    
    m_initialized = true;
    return true;
}

GestureResult GestureRecognizer::processFrame(const cv::Mat& frame) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!m_initialized || frame.empty()) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                startTime, startTime, 0.0f};
    }

    // Process frame and detect gestures (Placeholder - needs real point tracking)
    // For now, copy simulation logic - replace with actual tracking later
    std::vector<cv::Point2f> currentPoints; // Needs to be populated by tracking
    // ** Boundary Handling Placeholder: Real tracking should handle this **
    const float screenWidth = 1280.0f; // Assuming fixed resolution for now
    const float screenHeight = 720.0f;
    for (auto& pt : currentPoints) { 
        pt.x = std::max(0.0f, std::min(screenWidth - 1, pt.x));
        pt.y = std::max(0.0f, std::min(screenHeight - 1, pt.y));
    }
    GestureResult result = {GestureType::NONE, 0.0f, cv::Point2f(), currentPoints, {}, 
                           startTime, startTime, 0.0f};
    
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
            logGestureResult(result);
            m_lastGesture = result;
        } else {
            // Log failed attempt even if confidence was highest but below threshold
            // Create a temporary struct for logging, copying necessary fields
            GestureResult failedLogResult = result; // Copy base result values
            failedLogResult.transitionLatency = 0.0f; // Ensure latency is 0 for this specific log
            // endTimestamp should be set before velocity calculation, ensure it's valid
            if (failedLogResult.endTimestamp == std::chrono::high_resolution_clock::time_point{}) {
                 failedLogResult.endTimestamp = result.timestamp; // Fallback to start time if end time is unset
            }
            logGestureResult(failedLogResult); 
            result.type = GestureType::NONE; // Reset type if below threshold
        }
    } else {
        // Log if no gesture type had any confidence
        // Create a temporary struct for logging
        GestureResult failedLogResult = result; // Copy base result values
        failedLogResult.transitionLatency = 0.0f; // Ensure latency is 0 for this specific log
        // endTimestamp is already set before velocity calc
        logGestureResult(failedLogResult); 
        result.type = GestureType::NONE;
    }

    auto endTime = std::chrono::high_resolution_clock::now();
    result.endTimestamp = endTime;

    // Calculate and Normalize Velocities (Placeholder)
    result.velocities.clear();
    if (result.trajectory.size() > 1) {
        std::vector<float> rawVelocities(result.trajectory.size() - 1);
        const float timeStep = 1.0f / 60.0f; // Crude assumption
        float maxMagnitudeSq = 0.0f;

        for (size_t i = 0; i < result.trajectory.size() - 1; ++i) {
            cv::Point2f p1 = result.trajectory[i];
            cv::Point2f p2 = result.trajectory[i+1];
            float distSq = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
            float velocitySq = distSq / (timeStep * timeStep);
            rawVelocities[i] = std::sqrt(velocitySq); // Store raw magnitude
            maxMagnitudeSq = std::max(maxMagnitudeSq, velocitySq);
        }
        
        // Apply EMA Smoothing to raw velocities
        const float alpha = 0.2f; // Smoothing factor (adjust as needed)
        std::vector<float> smoothedVelocities = rawVelocities;
        for(size_t i = 1; i < smoothedVelocities.size(); ++i) {
            smoothedVelocities[i] = alpha * rawVelocities[i] + (1.0f - alpha) * smoothedVelocities[i-1];
        }
        
        // Normalize the SMOOTHED velocities
        result.velocities.resize(smoothedVelocities.size());
        float maxSmoothedMagnitude = 0.0f;
        for(const auto& v : smoothedVelocities) {
             maxSmoothedMagnitude = std::max(maxSmoothedMagnitude, v);
        }

        if (maxSmoothedMagnitude > 1e-6) { 
            std::cout << "Normalizing smoothed velocities (max smoothed magnitude: " << maxSmoothedMagnitude << " px/s)" << std::endl;
            for (size_t i = 0; i < smoothedVelocities.size(); ++i) {
                result.velocities[i] = smoothedVelocities[i] / maxSmoothedMagnitude; // Normalize smoothed
            }
        } else {
             std::cout << "Smoothed velocities not normalized (max magnitude was zero)." << std::endl;
             std::fill(result.velocities.begin(), result.velocities.end(), 0.0f);
        }
    }
    
    // Ensure final position is also clamped
    result.position.x = std::max(0.0f, std::min(screenWidth - 1, result.position.x));
    result.position.y = std::max(0.0f, std::min(screenHeight - 1, result.position.y));

    return result;
}

GestureResult GestureRecognizer::processSimulatedPoints(const std::vector<cv::Point2f>& points) {
    std::cout << "Entering processSimulatedPoints, points size: " << points.size() << std::endl;
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!m_initialized || points.empty()) {
        std::cout << "Not initialized or empty points, returning NONE" << std::endl;
        auto now = std::chrono::high_resolution_clock::now();
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                now, now, 0.0f};
    }

    // Make a mutable copy for potential clamping
    std::vector<cv::Point2f> processedPoints = points;

    // ** Boundary Handling: Clamp points to a typical screen space (e.g., 1280x720) **
    const float screenWidth = 1280.0f;
    const float screenHeight = 720.0f;
    for (auto& pt : processedPoints) {
        pt.x = std::max(0.0f, std::min(screenWidth - 1, pt.x));
        pt.y = std::max(0.0f, std::min(screenHeight - 1, pt.y));
    }

    // Correct initial result initializer list with std::vector<float> for velocities
    GestureResult result = {GestureType::NONE, 0.0f, cv::Point2f(), processedPoints, {}, 
                           startTime, startTime, 0.0f};
    std::cout << "Starting gesture recognition" << std::endl;

    // --- REVISED RECOGNITION LOGIC --- 
    GestureResult bestResult = result;

    auto khargailResult = recognizeKhargail(processedPoints);
    std::cout << "Khargail confidence: " << khargailResult.confidence << std::endl;
    if (khargailResult.confidence > bestResult.confidence) {
        bestResult = khargailResult;
    }
    if (khargailResult.type != GestureType::NONE) m_gestureAttempts[GestureType::KHARGAIL]++;

    auto flammilResult = recognizeFlammil(processedPoints);
    std::cout << "Flammil confidence: " << flammilResult.confidence << std::endl;
    if (flammilResult.confidence > bestResult.confidence) {
        bestResult = flammilResult;
    }
    if (flammilResult.type != GestureType::NONE) m_gestureAttempts[GestureType::FLAMMIL]++;

    auto stasaiResult = recognizeStasai(processedPoints);
    std::cout << "Stasai confidence: " << stasaiResult.confidence << std::endl;
    if (stasaiResult.confidence > bestResult.confidence) {
        bestResult = stasaiResult;
    }
    if (stasaiResult.type != GestureType::NONE) m_gestureAttempts[GestureType::STASAI]++;

    auto annihlatResult = recognizeAnnihlat(processedPoints);
    std::cout << "Annihlat confidence: " << annihlatResult.confidence << std::endl;
    if (annihlatResult.confidence > bestResult.confidence) {
        bestResult = annihlatResult;
    }
    if (annihlatResult.type != GestureType::NONE) m_gestureAttempts[GestureType::ANNIHLAT]++;
    
    // Assign the best result found
    result = bestResult; 

    std::cout << "Finished recognition, best result type: " << static_cast<int>(result.type) << " with confidence: " << result.confidence << std::endl;

    // Calculate, Smooth, and Normalize Velocities (Simulation)
    result.velocities.clear();
    if (processedPoints.size() > 1) {
        std::vector<float> rawVelocities(processedPoints.size() - 1);
        const float timeStep = 1.0f / 60.0f; // Assume 60 FPS simulation
        float maxMagnitudeSq = 0.0f; 

        for (size_t i = 0; i < processedPoints.size() - 1; ++i) {
            cv::Point2f p1 = processedPoints[i];
            cv::Point2f p2 = processedPoints[i+1];
            float distSq = (p2.x - p1.x) * (p2.x - p1.x) + (p2.y - p1.y) * (p2.y - p1.y);
            float velocitySq = distSq / (timeStep * timeStep);
            rawVelocities[i] = std::sqrt(velocitySq); 
            maxMagnitudeSq = std::max(maxMagnitudeSq, velocitySq);
        }

        // Apply EMA Smoothing to raw velocities
        const float alpha = 0.2f; // Smoothing factor
        std::vector<float> smoothedVelocities = rawVelocities;
        if (!smoothedVelocities.empty()) {
             for(size_t i = 1; i < smoothedVelocities.size(); ++i) {
                 smoothedVelocities[i] = alpha * rawVelocities[i] + (1.0f - alpha) * smoothedVelocities[i-1];
             }
        }
       
        // Normalize the SMOOTHED velocities
        result.velocities.resize(smoothedVelocities.size());
        float maxSmoothedMagnitude = 0.0f;
        for(const auto& v : smoothedVelocities) {
             maxSmoothedMagnitude = std::max(maxSmoothedMagnitude, v);
        }

        if (maxSmoothedMagnitude > 1e-6) { 
            std::cout << "Normalizing smoothed velocities (max smoothed magnitude: " << maxSmoothedMagnitude << " px/s)" << std::endl;
            for (size_t i = 0; i < smoothedVelocities.size(); ++i) {
                result.velocities[i] = smoothedVelocities[i] / maxSmoothedMagnitude; // Normalize smoothed
            }
        } else {
             std::cout << "Smoothed velocities not normalized (max magnitude was zero)." << std::endl;
             std::fill(result.velocities.begin(), result.velocities.end(), 0.0f);
        }
    }

    // Apply gesture-specific threshold and update stats for the final best result
    if (result.type != GestureType::NONE) {
        float threshold = m_gestureThresholds.count(result.type) ? m_gestureThresholds.at(result.type) : m_minConfidence;
        if (result.confidence >= threshold) {
            if (m_gestureSuccesses.count(result.type)) { m_gestureSuccesses.at(result.type)++; }
            updateTransitionStats(result, m_lastGesture);
            logGestureResult(result);
            result.position = processedPoints.back();
            result.position.x = std::max(0.0f, std::min(screenWidth - 1, result.position.x));
            result.position.y = std::max(0.0f, std::min(screenHeight - 1, result.position.y));
            m_lastGesture = result;
        } else {
            // Log failed attempt even if confidence was highest but below threshold
            // Create a temporary struct for logging, copying necessary fields
            GestureResult failedLogResult = result; // Copy base result values
            failedLogResult.transitionLatency = 0.0f; // Ensure latency is 0 for this specific log
            // endTimestamp should be set before velocity calculation, ensure it's valid
            if (failedLogResult.endTimestamp == std::chrono::high_resolution_clock::time_point{}) {
                 failedLogResult.endTimestamp = result.timestamp; // Fallback to start time if end time is unset
            }
            logGestureResult(failedLogResult); 
            result.type = GestureType::NONE; // Reset type if below threshold
        }
    } else {
        // Log if no gesture type had any confidence
        // Create a temporary struct for logging
        GestureResult failedLogResult = result; // Copy base result values
        failedLogResult.transitionLatency = 0.0f; // Ensure latency is 0 for this specific log
        // endTimestamp is already set before velocity calc
        logGestureResult(failedLogResult); 
        result.type = GestureType::NONE;
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
    auto callTime = std::chrono::high_resolution_clock::now();
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                callTime, callTime, 0.0f};
    }

    // Calculate direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Khargail is a left-right charge (60px, 0.4s)
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 0.0f));
    
    // DEBUG OUTPUT
    std::cout << "Khargail Recognize Debug: Distance=" << distance << ", Confidence=" << confidence 
              << ", MinConfidence=" << m_minConfidence << std::endl;

    if (distance >= 60.0f && confidence >= m_minConfidence) {
        return {GestureType::KHARGAIL, confidence, end, points, {}, 
                callTime, callTime, 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
            callTime, callTime, 0.0f};
}

GestureResult GestureRecognizer::recognizeFlammil(const std::vector<cv::Point2f>& points) {
    auto callTime = std::chrono::high_resolution_clock::now();
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                callTime, callTime, 0.0f};
    }

    // Calculate direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Flammil is a right-down swipe (75px, 0.5s)
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 1.0f));
    
    std::cout << "Flammil Debug: Distance=" << distance << ", Confidence=" << confidence 
              << ", Start=(" << start.x << "," << start.y << "), End=(" << end.x << "," << end.y << ")"
              << ", MinConfidence=" << m_minConfidence << std::endl; // Added MinConfidence here too for comparison
    
    if (distance >= 75.0f && confidence >= m_minConfidence) {
        return {GestureType::FLAMMIL, confidence, end, points, {}, 
                callTime, callTime, 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
            callTime, callTime, 0.0f};
}

GestureResult GestureRecognizer::recognizeStasai(const std::vector<cv::Point2f>& points) {
    auto callTime = std::chrono::high_resolution_clock::now();
    if (points.size() < 8) {  // Need enough points for circle detection
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                callTime, callTime, 0.0f};
    }

    // Stasai is a tight circle (50px, 0.6s)
    // ADD DEBUG FOR STASAI
    bool is_circle = isCircle(points);
    float confidence = is_circle ? 0.8f : 0.0f; // Base confidence for circle detection or 0
    std::cout << "Stasai Recognize Debug: IsCircle=" << (is_circle ? "true" : "false") << ", Confidence=" << confidence << std::endl;

    if (is_circle) { // No separate confidence check here, relies on isCircle
        //float confidence = 0.8f;  // Base confidence for circle detection
        return {GestureType::STASAI, confidence, points.back(), points, {}, 
                callTime, callTime, 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
            callTime, callTime, 0.0f};
}

GestureResult GestureRecognizer::recognizeAnnihlat(const std::vector<cv::Point2f>& points) {
    auto callTime = std::chrono::high_resolution_clock::now();
    if (points.size() < 3) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                callTime, callTime, 0.0f};
    }

    // Calculate direction vector
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f direction = end - start;
    
    // Annihlat is a right swipe down
    float distance = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, -1.0f));
    
    // DEBUG OUTPUT
    std::cout << "Annihlat Recognize Debug: Distance=" << distance << ", Confidence=" << confidence 
              << ", MinConfidence=" << m_minConfidence << std::endl;

    if (distance >= 60.0f && confidence >= m_minConfidence) {
        return {GestureType::ANNIHLAT, confidence, end, points, {}, 
                callTime, callTime, 0.0f};
    }
    
    return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
            callTime, callTime, 0.0f};
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

        // DEBUG OUTPUT FOR TRANSITION
        std::cout << "UpdateTransitionStats Debug: PrevType=" << static_cast<int>(previous.type)
                  << ", CurrType=" << static_cast<int>(current.type)
                  << ", Latency=" << latency << "s"
                  << ", AvgLatency=" << m_averageTransitionLatency << "s"
                  << std::endl;
    }
}

void GestureRecognizer::logGestureResult(const GestureResult& result) {
    if (!m_logFile.is_open()) {
        std::cerr << "[Log Error] Log file not open! Falling back to console." << std::endl;
        // Fallback to console (expanded details)
        std::cout << "[LOG FALLBACK] Gesture: " << static_cast<int>(result.type) 
                  << ", Confidence: " << result.confidence 
                  << ", Position: (" << result.position.x << "," << result.position.y << ")" 
                  << ", Latency: " << result.transitionLatency << "s" << std::endl;
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
    m_logFile.flush(); // Ensure data is written
    if (!m_logFile.good()) {
        std::cerr << "[Log Error] Log file write failed! Stream state: " << m_logFile.rdstate() << std::endl;
    }
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

float GestureRecognizer::calculateSwipeConfidence(const std::vector<cv::Point2f>& points, const cv::Point2f& expectedDirection) {
    if (points.size() < 5) { // Keep minimum point requirement
        return 0.0f;
    }
    
    // Basic confidence based on some trajectory analysis (placeholder)
    // A more sophisticated approach would analyze curvature, speed consistency etc.
    float confidence = 0.8f; // Base confidence - lowered slightly to allow penalty room

    // Calculate actual direction and length
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    cv::Point2f actualDirection = end - start;
    float actualLength = std::sqrt(actualDirection.x * actualDirection.x + actualDirection.y * actualDirection.y);

    // Normalize expected direction (assuming it might not be normalized)
    cv::Point2f normalizedExpectedDirection = expectedDirection;
    float expectedLength = std::sqrt(normalizedExpectedDirection.x * normalizedExpectedDirection.x + normalizedExpectedDirection.y * normalizedExpectedDirection.y);
    if (expectedLength > 1e-6) { // Avoid division by zero
         normalizedExpectedDirection.x /= expectedLength;
         normalizedExpectedDirection.y /= expectedLength;
    }

    // Calculate direction deviation penalty
    if (actualLength > 1e-6) { // Avoid division by zero for actual direction
        cv::Point2f normalizedActualDirection = actualDirection / actualLength;
        // Dot product gives cosine of angle between directions (1 for parallel, 0 for perpendicular, -1 for opposite)
        float dotProduct = normalizedActualDirection.x * normalizedExpectedDirection.x + normalizedActualDirection.y * normalizedExpectedDirection.y;
        // Penalize confidence based on deviation. Clamp dot product to [0, 1] as penalty factor.
        // We want high confidence only if directions align (dotProduct close to 1).
        confidence *= std::max(0.0f, dotProduct);
    } else {
        // Very short or zero length trajectory, low confidence
        confidence = 0.0f;
    }

    // Add other factors? (e.g., penalty for excessive deviation from straight line)
    // For now, focus on direction.
    
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
    
    // *** ADD CLOSURE CHECK ***
    cv::Point2f startPoint = points.front();
    cv::Point2f endPoint = points.back();
    float closureDistance = std::sqrt(
        (endPoint.x - startPoint.x) * (endPoint.x - startPoint.x) +
        (endPoint.y - startPoint.y) * (endPoint.y - startPoint.y));

    // Allow closure distance up to, e.g., 50% of the average radius
    // Adjust this threshold as needed
    float maxClosure = 0.5f * avgRadius; 
    if (closureDistance > maxClosure) {
        return false; // Not closed enough for a circle
    }
    // *** END CLOSURE CHECK ***

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

float GestureRecognizer::getGestureThreshold(GestureType type) const {
    if (m_gestureThresholds.count(type)) {
        return m_gestureThresholds.at(type);
    }
    // Return default minimum if specific threshold not found
    return m_minConfidence; 
}

} // namespace CSL
} // namespace TurtleEngine 