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
#include <numeric> // For std::accumulate if needed later
#include <future>
#include <thread>

// Conditional cout macro
#ifdef ENABLE_GESTURE_DEBUG_LOGGING
#define GESTURE_DEBUG_LOG(x) std::cout << x << std::endl
#else
#define GESTURE_DEBUG_LOG(x) 
#endif

// Fast square root approximation with Newton-Raphson iteration
inline float fastSqrt(float number) {
    if (number <= 0.0f) return 0.0f; // Handle zero/negative
    union { float f; int i; } u;
    u.f = number;
    u.i = 0x5f3759df - (u.i >> 1); // Initial guess for 1/sqrt(x)
    u.f = u.f * (1.5f - (0.5f * number * u.f * u.f)); // Newton-Raphson step for 1/sqrt(x)
    return number * u.f; // Convert 1/sqrt(x) to sqrt(x)
}

// Define initial map contents (if needed, or initialize empty)
// static const std::map<TurtleEngine::CSL::GestureType, float> initialThresholds = { ... };

namespace TurtleEngine {
namespace CSL {

GestureRecognizer::GestureRecognizer()
    : m_sensitivity(1.2f)
    , m_minConfidence(0.70f)
    , m_averageTransitionLatency(0.0f)
    , m_initialized(false)
    , m_circleClosureThreshold(100.0f)
    // Initialize maps in initializer list
    // Reverted Flammil threshold back to original requirement
    , m_gestureThresholds({{GestureType::KHARGAIL, 0.78f}, {GestureType::FLAMMIL, 0.74f}, {GestureType::STASAI, 0.80f}, {GestureType::ANNIHLAT, 0.75f}})
    , m_gestureAttempts({{GestureType::KHARGAIL, 0}, {GestureType::FLAMMIL, 0}, {GestureType::STASAI, 0}, {GestureType::ANNIHLAT, 0}, {GestureType::NONE, 0}, {GestureType::TBD, 0}})
    , m_gestureSuccesses({{GestureType::KHARGAIL, 0}, {GestureType::FLAMMIL, 0}, {GestureType::STASAI, 0}, {GestureType::ANNIHLAT, 0}, {GestureType::NONE, 0}, {GestureType::TBD, 0}})
{
    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: Start");
    m_previousPoints.reserve(30);
    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: Reserved previousPoints");
    auto now = std::chrono::high_resolution_clock::now();
    m_lastGesture = {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, now, now, 0.0f};
    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: Initialized lastGesture");
    m_lastTransition = {GestureType::NONE, GestureType::NONE, 0.0f, 0.0f};
    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: Initialized lastTransition");
    
    // Map initialization moved to initializer list above
    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: Maps initialized via list");

    // Initialize log file
    std::filesystem::create_directories("logs");
    m_logFile.open("logs/gesture_debug.log", std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Failed to open gesture debug log file!" << std::endl;
    } else {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::stringstream ss;
        ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[Profiler] GestureRecognizer initialized with closure threshold: " 
           << std::fixed << std::setprecision(3) << m_circleClosureThreshold << " px" << std::endl;
        
        m_logFile << ss.str();
        m_logFile.flush();
    }

    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: End");
}

void GestureRecognizer::setCircleClosureThreshold(float threshold) {
    float oldThreshold = m_circleClosureThreshold;
    m_circleClosureThreshold = std::max(0.1f, std::min(200.0f, threshold));
    
    // Log threshold adjustment
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::stringstream ss;
        ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[Profiler] Circle Closure Threshold Updated: " << std::fixed << std::setprecision(3)
           << oldThreshold << " px -> " << m_circleClosureThreshold << " px" << std::endl;
        
        m_logFile << ss.str();
        m_logFile.flush();
    }
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
    
    GESTURE_DEBUG_LOG("Initialize: Checking/Creating logs directory...");
    std::filesystem::create_directories("logs");
    GESTURE_DEBUG_LOG("Initialize: Logs directory created or exists.");
    
    GESTURE_DEBUG_LOG("Initialize: Opening log file logs/gesture_debug.log...");
    m_logFile.open("logs/gesture_debug.log", std::ios::app);
    if (!m_logFile.is_open()) {
        std::cerr << "Initialize: ERROR - Failed to open gesture debug log file!" << std::endl;
        return false;
    }
    GESTURE_DEBUG_LOG("Initialize: Log file opened successfully.");
    
    m_initialized = true;
    return true;
}

GestureResult GestureRecognizer::processFrame(const cv::Mat& frame) {
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!m_initialized || frame.empty()) {
        return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
                startTime, startTime, 0.0f};
    }

    // Point tracking logic needs to populate currentPoints
    std::vector<cv::Point2f> currentPoints; 
    // --- Placeholder for actual point tracking --- 
    // currentPoints = trackPoints(frame); 
    // --- End Placeholder --- 
    
    // ** Boundary Handling Placeholder: Real tracking should handle this **
    const float screenWidth = 1280.0f; 
    const float screenHeight = 720.0f;
    for (auto& pt : currentPoints) { 
        pt.x = std::max(0.0f, std::min(screenWidth - 1, pt.x));
        pt.y = std::max(0.0f, std::min(screenHeight - 1, pt.y));
    }

    GestureResult result = {GestureType::NONE, 0.0f, cv::Point2f(), currentPoints, {}, 
                           startTime, startTime, 0.0f};

    // Revert to Sequential gesture recognition
    GestureResult bestResult = result; // Start with NONE

    auto khargailResult = recognizeKhargail(currentPoints);
    if (khargailResult.confidence > bestResult.confidence) {
        bestResult = khargailResult;
    }
    if (khargailResult.type != GestureType::NONE) {
        m_gestureAttempts[GestureType::KHARGAIL]++;
    }

    auto flammilResult = recognizeFlammil(currentPoints);
    if (flammilResult.confidence > bestResult.confidence) {
        bestResult = flammilResult;
    }
    if (flammilResult.type != GestureType::NONE) {
        m_gestureAttempts[GestureType::FLAMMIL]++;
    }

    auto stasaiResult = recognizeStasai(currentPoints, "");
    if (stasaiResult.confidence > bestResult.confidence) {
        bestResult = stasaiResult;
    }
    if (stasaiResult.type != GestureType::NONE) {
        m_gestureAttempts[GestureType::STASAI]++;
    }

    auto annihlatResult = recognizeAnnihlat(currentPoints);
    if (annihlatResult.confidence > bestResult.confidence) {
        bestResult = annihlatResult;
    }
    if (annihlatResult.type != GestureType::NONE) {
        m_gestureAttempts[GestureType::ANNIHLAT]++;
    }

    bestResult.trajectory = currentPoints; // Ensure trajectory is set
    result = bestResult;

    // Update transition stats if a gesture was detected
    if (result.type != GestureType::NONE) {
        float threshold = m_gestureThresholds.count(result.type) ? m_gestureThresholds.at(result.type) : m_minConfidence;
        if (result.confidence >= threshold) {
            if (m_gestureSuccesses.count(result.type)) { 
                 // No lock needed for m_gestureSuccesses if only updated here under threshold check?
                 // Re-eval: If multiple threads could potentially write here (unlikely with current structure but possible later), lock needed.
                 // For now, assume threshold check serializes access. If profiling shows contention, add lock. 
                m_gestureSuccesses.at(result.type)++; 
            }
            updateTransitionStats(result, m_lastGesture); // Needs m_lastGesture - is this thread safe?
            // m_lastGesture is only written *after* this block, and read by updateTransitionStats.
            // Assumed safe for now, but needs verification if processFrame itself runs concurrently.
            logGestureResult(result); // Uses m_logFile - ofstream might not be thread-safe by default.
            // Need to protect log file access if processFrame runs concurrently.
            m_lastGesture = result; // Update last gesture *after* processing
        } else {
            // Log failed attempt 
            GestureResult failedLogResult = result; 
            failedLogResult.transitionLatency = 0.0f; 
            if (failedLogResult.endTimestamp == std::chrono::high_resolution_clock::time_point{}) {
                 failedLogResult.endTimestamp = result.timestamp; 
            }
            logGestureResult(failedLogResult); // Potential log file access issue
            result.type = GestureType::NONE; 
        }
    } else {
        // Log if no gesture type had any confidence
        GestureResult failedLogResult = result; 
        failedLogResult.transitionLatency = 0.0f; 
        logGestureResult(failedLogResult); // Potential log file access issue
        result.type = GestureType::NONE;
    }

    auto endTime = std::chrono::high_resolution_clock::now(); // Profiling end
    std::chrono::duration<double, std::milli> duration = endTime - startTime;
    std::cout << "[Profiler] processFrame Duration: " << duration.count() << "ms" << std::endl;

    // Simplified Velocity Calculation Block (remove EMA and normalization)
    result.velocities.clear();
    if (result.trajectory.size() > 1) {
        size_t num_segments = result.trajectory.size() - 1;
        result.velocities.resize(num_segments);
        const float timeStep = (1.0f / 60.0f);
        // Removed: alpha, maxMagnitude, smoothedVelocity initialization

        // Calculate and store raw velocities
        for (size_t i = 0; i < num_segments; ++i) {
            float dx = result.trajectory[i+1].x - result.trajectory[i].x;
            float dy = result.trajectory[i+1].y - result.trajectory[i].y;
            float distSq = dx * dx + dy * dy;
            float velocity = 0.0f;
            if (distSq > 1e-9) {
                float dist = fastSqrt(distSq); // Using fastSqrt
                velocity = dist / timeStep; 
            }
            result.velocities[i] = velocity; // Store raw velocity directly
            // Removed: EMA calculation
            // Removed: maxMagnitude tracking
        }
        
        // Removed: Normalization loop
        GESTURE_DEBUG_LOG("Stored raw velocities.");
    }
    
    // Ensure final position is clamped
    result.position.x = std::max(0.0f, std::min(screenWidth - 1, result.position.x));
    result.position.y = std::max(0.0f, std::min(screenHeight - 1, result.position.y));

    return result;
}

GestureResult GestureRecognizer::processSimulatedPoints(const std::vector<cv::Point2f>& points, const std::string& testCaseId) {
    auto profiler_start = std::chrono::high_resolution_clock::now();
    GestureResult finalResult = {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, profiler_start, profiler_start, 0.0f}; // Initialize result

    if (points.size() < 3) {
        auto profiler_end = std::chrono::high_resolution_clock::now();
        float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
        if (m_logFile.is_open()) {
            auto log_now = std::chrono::system_clock::now(); // Use system_clock for wall time
            auto time = std::chrono::system_clock::to_time_t(log_now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log_now.time_since_epoch()) % 1000;

            std::tm timeInfo;
            localtime_s(&timeInfo, &time);
            std::stringstream ss;
            ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
               << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
               << "[Profiler] processSimulatedPoints Early Exit (Points): " << points.size() << " < 3"
               << ", TestCase: " << testCaseId << ", Duration: " << profiler_duration << " ms" << std::endl;

            m_logFile << ss.str();
            m_logFile.flush(); // Ensure log is written
        }
        finalResult.endTimestamp = profiler_end; // Set end time even on early exit
        return finalResult; // Return initialized result
    }

    // --- Recognize All Gestures Sequentially for Simulation --- 
    GestureResult bestResult = {GestureType::NONE, 0.0f}; // Start with NONE

    // Recognize Khargail
    auto khargailResult = recognizeKhargail(points);
    if (khargailResult.confidence > bestResult.confidence) {
        bestResult = khargailResult;
    }

    // Recognize Flammil
    auto flammilResult = recognizeFlammil(points);
    if (flammilResult.confidence > bestResult.confidence) {
        bestResult = flammilResult;
    }

    // Recognize Stasai
    auto stasaiResult = recognizeStasai(points, testCaseId); // Pass testCaseId
    if (stasaiResult.confidence > bestResult.confidence) {
        bestResult = stasaiResult;
    }

    // Recognize Annihlat
    auto annihlatResult = recognizeAnnihlat(points);
    if (annihlatResult.confidence > bestResult.confidence) {
        bestResult = annihlatResult;
    }
    // --- End Sequential Recognition --- 

    // Now process the best result found
    if (bestResult.type != GestureType::NONE) { // Check if any gesture was recognized
       // Use the threshold defined for this specific gesture type, or the minimum confidence
        float requiredConfidence = getGestureThreshold(bestResult.type);

        if (bestResult.confidence >= requiredConfidence) {
             // Gesture recognized successfully
            auto profiler_end = std::chrono::high_resolution_clock::now(); // Use high_resolution_clock for duration
            float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
            // Assign timestamps and duration to the successful result
            bestResult.timestamp = profiler_start;
            bestResult.endTimestamp = profiler_end;
            bestResult.transitionLatency = profiler_duration; // Using latency field for duration in simulation
            bestResult.trajectory = points; // Ensure trajectory is included

            if (m_logFile.is_open()) {
                auto log_now = std::chrono::system_clock::now(); // Use system_clock for wall time logging
                auto time = std::chrono::system_clock::to_time_t(log_now);
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(log_now.time_since_epoch()) % 1000;

                std::tm timeInfo;
                localtime_s(&timeInfo, &time);
                std::stringstream ss;
                 // Log recognized gesture
                ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S")
                   << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
                   << "[Profiler] Gesture Recognized (Simulated): " << static_cast<int>(bestResult.type)
                   << ", Confidence: " << std::fixed << std::setprecision(3) << bestResult.confidence
                   << ", TestCase: " << testCaseId << ", Total Duration: " << profiler_duration << " ms" << std::endl;

                m_logFile << ss.str();
                m_logFile.flush(); // Ensure log is written
            }

            // Update state for successful recognition
            m_lastRecognizedGesture = bestResult;
            m_lastGestureTime = profiler_end; // Use the end time of processing
            finalResult = bestResult; // Update the final result to return
        } else {
             // Log gesture attempt that failed confidence check
             // (Optional: Add logging here if needed for low-confidence attempts)
             finalResult.type = GestureType::NONE; // Ensure result type is NONE if confidence too low
             finalResult.endTimestamp = std::chrono::high_resolution_clock::now();
        }
    } else {
         // No gesture was recognized by any recognizer
         finalResult.type = GestureType::NONE;
         finalResult.endTimestamp = std::chrono::high_resolution_clock::now();
    }

    return finalResult;
}

GestureResult GestureRecognizer::recognizeKhargail(const std::vector<cv::Point2f>& points) {
    auto profiler_start = std::chrono::high_resolution_clock::now(); // Profiling start
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

    if (distance >= 60.0f) {
        return {GestureType::KHARGAIL, confidence, end, points, {}, 
                callTime, callTime, 0.0f};
    }
    
    auto profiler_end = std::chrono::high_resolution_clock::now(); // Profiling end
    float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
    std::cout << "[Profiler] recognizeKhargail Duration: " << profiler_duration << "ms" << std::endl;

    return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
            callTime, callTime, 0.0f};
}

GestureResult GestureRecognizer::recognizeFlammil(const std::vector<cv::Point2f>& points) {
    auto profiler_start = std::chrono::high_resolution_clock::now(); // Profiling start
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
              << ", MinConfidence=" << m_minConfidence << std::endl;
    
    if (distance >= 75.0f) {
        return {GestureType::FLAMMIL, confidence, end, points, {}, 
                callTime, callTime, 0.0f};
    }
    
    auto profiler_end = std::chrono::high_resolution_clock::now(); // Profiling end
    float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
    std::cout << "[Profiler] recognizeFlammil Duration: " << profiler_duration << "ms" << std::endl;

    return {GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, 
            callTime, callTime, 0.0f};
}

GestureResult GestureRecognizer::recognizeStasai(const std::vector<cv::Point2f>& points, const std::string& testCaseId) {
    auto profiler_start = std::chrono::high_resolution_clock::now();
    const size_t minPoints = 8;
    float confidence = 0.0f;

    if (points.size() < minPoints) {
        auto profiler_end = std::chrono::high_resolution_clock::now();
        float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
        if (m_logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            
            std::tm timeInfo;
            localtime_s(&timeInfo, &time);
            std::stringstream ss;
            ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
               << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
               << "[Profiler] recognizeStasai Early Exit (Points): " << points.size() << " < " << minPoints 
               << ", TestCase: " << testCaseId << ", Duration: " << profiler_duration << " ms" << std::endl;
            
            m_logFile << ss.str();
        }
        return { GestureType::NONE, 0.0f, cv::Point2f(), {}, {}, profiler_start, profiler_end, profiler_duration };
    }

    // Check if points form a circle
    bool isCircleResult = isCircle(points, testCaseId);
    confidence = isCircleResult ? 1.0f : 0.0f;

    auto profiler_end = std::chrono::high_resolution_clock::now();
    float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
    
    // Log final result with test case ID
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::stringstream ss;
        ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[Profiler] recognizeStasai Result: " << (isCircleResult ? "true" : "false")
           << ", Confidence: " << std::fixed << std::setprecision(3) << confidence
           << ", TestCase: " << testCaseId << ", Duration: " << profiler_duration << " ms" << std::endl;
        
        m_logFile << ss.str();
    }

    return {
        isCircleResult ? GestureType::STASAI : GestureType::NONE,
        confidence,
        points.empty() ? cv::Point2f() : points.back(),
        points,
        {},
        profiler_start,
        profiler_end,
        profiler_duration
    };
}

GestureResult GestureRecognizer::recognizeAnnihlat(const std::vector<cv::Point2f>& points) {
    auto profiler_start = std::chrono::high_resolution_clock::now(); // Profiling start
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

    if (distance >= 60.0f) {
        return {GestureType::ANNIHLAT, confidence, end, points, {}, 
                callTime, callTime, 0.0f};
    }
    
    auto profiler_end = std::chrono::high_resolution_clock::now(); // Profiling end
    float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
    std::cout << "[Profiler] recognizeAnnihlat Duration: " << profiler_duration << "ms" << std::endl;

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
        GESTURE_DEBUG_LOG("UpdateTransitionStats Debug: PrevType=" << static_cast<int>(previous.type)
                         << ", CurrType=" << static_cast<int>(current.type)
                         << ", Latency=" << latency << "s"
                         << ", AvgLatency=" << m_averageTransitionLatency << "s");
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
    auto profiler_start = std::chrono::high_resolution_clock::now(); // Profiling start
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
    
    auto profiler_end = std::chrono::high_resolution_clock::now(); // Profiling end
    float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
    std::cout << "[Profiler] calculateSwipeConfidence Duration: " << profiler_duration << "ms" << std::endl;

    return confidence;
}

bool GestureRecognizer::isCircle(const std::vector<cv::Point2f>& points, const std::string& testCaseId) {
    auto profiler_start = std::chrono::high_resolution_clock::now();
    const size_t minPointsForCircle = 8;
    const float varianceThresholdSquared = 5000.0f; // Increased from 2500.0f

    if (points.size() < minPointsForCircle) {
        auto profiler_end = std::chrono::high_resolution_clock::now();
        float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
        if (m_logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            
            std::tm timeInfo;
            localtime_s(&timeInfo, &time);
            std::stringstream ss;
            ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
               << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
               << "[Profiler] isCircle Early Exit (Points): " << points.size() << " < " << minPointsForCircle 
               << ", TestCase: " << testCaseId << ", Duration: " << profiler_duration << " ms" << std::endl;
            
            m_logFile << ss.str();
        }
        return false;
    }

    // Calculate rough closure distance (start to end point)
    cv::Point2f start = points.front();
    cv::Point2f end = points.back();
    float roughClosureDistance = std::sqrt(
        (end.x - start.x) * (end.x - start.x) + 
        (end.y - start.y) * (end.y - start.y)
    );

    // Log closure distance with test case ID
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::stringstream ss;
        ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[Profiler] isCircle Closure Distance: " << std::fixed << std::setprecision(3) 
           << roughClosureDistance << " px (Threshold: " << m_circleClosureThreshold << " px), "
           << "TestCase: " << testCaseId << ", Points: " << points.size() << std::endl;
        
        m_logFile << ss.str();
    }
    
    // If start and end points are too far apart, it's probably not a circle
    if (roughClosureDistance > m_circleClosureThreshold) {
        auto profiler_end = std::chrono::high_resolution_clock::now();
        float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
        if (m_logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            
            std::tm timeInfo;
            localtime_s(&timeInfo, &time);
            std::stringstream ss;
            ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
               << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
               << "[Profiler] isCircle Early Exit (Closure): " << profiler_duration << " ms, "
               << "TestCase: " << testCaseId << std::endl;
            
            m_logFile << ss.str();
        }
        return false;
    }

    // Sample every third point for better performance
    const size_t step = 3;
    size_t sampled_count = 0;
    std::vector<cv::Point2f> sampled_points;
    sampled_points.reserve(points.size() / step + 1);

    // Calculate center using sampled points
    cv::Point2f center(0, 0);
    for (size_t i = 0; i < points.size(); i += step) {
        sampled_points.push_back(points[i]);
        center += points[i];
        sampled_count++;
    }

    // Log sampled points count with test case ID
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::stringstream ss;
        ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[Profiler] isCircle Sampled Points: " << sampled_count << " (from " << points.size() << " total), "
           << "TestCase: " << testCaseId << std::endl;
        
        m_logFile << ss.str();
    }

    if (sampled_count < minPointsForCircle / step) {
        auto profiler_end = std::chrono::high_resolution_clock::now();
        float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
        if (m_logFile.is_open()) {
            auto now = std::chrono::system_clock::now();
            auto time = std::chrono::system_clock::to_time_t(now);
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
            
            std::tm timeInfo;
            localtime_s(&timeInfo, &time);
            std::stringstream ss;
            ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
               << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
               << "[Profiler] isCircle Early Exit (Sample Count): " << profiler_duration << " ms, "
               << "TestCase: " << testCaseId << std::endl;
            
            m_logFile << ss.str();
        }
        return false;
    }

    center /= static_cast<float>(sampled_count);

    // Pre-calculate squared distances for SIMD-friendly processing
    std::vector<float> squared_distances;
    squared_distances.reserve(sampled_count);
    float avgRadiusSquared = 0.0f;

    for (const auto& point : sampled_points) {
        float dx = point.x - center.x;
        float dy = point.y - center.y;
        float distSq = dx * dx + dy * dy;
        squared_distances.push_back(distSq);
        avgRadiusSquared += distSq;
    }
    avgRadiusSquared /= static_cast<float>(sampled_count);

    // --- New Approach: Check individual point deviations from average radius --- 
    float avgRadius = fastSqrt(avgRadiusSquared);
    const float radiusTolerance = 15.0f; // Tolerance in pixels
    bool allPointsWithinTolerance = true;

    for (float distSq : squared_distances) {
        float distance = fastSqrt(distSq);
        if (std::abs(distance - avgRadius) > radiusTolerance) {
            allPointsWithinTolerance = false;
            break; // Exit early if one point is outside tolerance
        }
    }

    // bool result = radiusVariance < varianceThresholdSquared; // Old check
    bool result = allPointsWithinTolerance; // New check based on tolerance

    auto profiler_end = std::chrono::high_resolution_clock::now();
    float profiler_duration = std::chrono::duration<float, std::milli>(profiler_end - profiler_start).count();
    
    // Log final result with test case ID
    if (m_logFile.is_open()) {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::tm timeInfo;
        localtime_s(&timeInfo, &time);
        std::stringstream ss;
        ss << "[" << std::put_time(&timeInfo, "%Y-%m-%d %H:%M:%S") 
           << "." << std::setfill('0') << std::setw(3) << ms.count() << "] "
           << "[Profiler] isCircle Result: " << (result ? "true" : "false") 
           << ", Duration: " << profiler_duration << " ms" << std::endl;
        
        m_logFile << ss.str();
    }

    return result;
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

float GestureRecognizer::getCircleClosureThreshold() const { return m_circleClosureThreshold; }

} // namespace CSL
} // namespace TurtleEngine 