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
    // Correctly initialize m_lastGesture using default + assignment
    m_lastGesture = GestureResult(); // Default construction
    m_lastGesture.type = GestureType::NONE;
    m_lastGesture.timestamp = now;
    m_lastGesture.endTimestamp = now;
    // Initialize m_lastRecognizedGesture similarly
    m_lastRecognizedGesture = GestureResult();
    m_lastRecognizedGesture.type = GestureType::NONE;
    m_lastRecognizedGesture.timestamp = now;
    m_lastRecognizedGesture.endTimestamp = now;
    GESTURE_DEBUG_LOG("GestureRecognizer Constructor: Initialized lastGesture & lastRecognizedGesture");
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
        // Use default constructor and minimal assignment for early exit
        GestureResult earlyExitResult;
        earlyExitResult.type = GestureType::NONE;
        earlyExitResult.confidence = 0.0f;
        // gestureName defaults to UNKNOWN
        earlyExitResult.position = cv::Point2f();
        // trajectory defaults to empty
        // velocities defaults to empty
        // debug_velocity defaults to 0.0f
        earlyExitResult.timestamp = startTime;
        earlyExitResult.endTimestamp = startTime;
        earlyExitResult.transitionLatency = 0.0f;
        // triggerTimestamp defaults to nullopt
        return earlyExitResult;
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

    // Use default constructor and assign relevant fields
    GestureResult result;
    result.type = GestureType::NONE;
    // gestureName defaults to UNKNOWN
    result.confidence = 0.0f;
    result.position = !currentPoints.empty() ? currentPoints.back() : cv::Point2f();
    result.trajectory = currentPoints;
    // velocities defaults to empty (assigned later)
    // debug_velocity defaults to 0.0f (assigned later)
    result.timestamp = startTime;
    // endTimestamp assigned later
    result.transitionLatency = 0.0f; // Calculated later if gesture recognized
    // triggerTimestamp defaults to nullopt (not applicable here)

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
            // Use default constructor and copy relevant data, then mark failure
            GestureResult failedLogResult = result; // Copy base data
            failedLogResult.type = GestureType::NONE; // Reset type to NONE for logging failure
            failedLogResult.gestureName = "FAILED_RECOGNITION"; // Indicate failure reason
            // Ensure endTimestamp is set (even if start time)
            if (failedLogResult.endTimestamp == std::chrono::high_resolution_clock::time_point{}) {
                 failedLogResult.endTimestamp = result.timestamp; 
            }
            failedLogResult.transitionLatency = 0.0f; // Not applicable for failed
            logGestureResult(failedLogResult); // Log the failure details
            result.type = GestureType::NONE; // Reset the main result type 
        }
    } else {
        // Log if no gesture type had any confidence
        // Use default constructor and mark as NONE
        GestureResult failedLogResult; // Default construction
        failedLogResult.type = GestureType::NONE;
        failedLogResult.gestureName = "NONE";
        failedLogResult.confidence = 0.0f; 
        failedLogResult.position = result.position; // Copy position if available
        failedLogResult.trajectory = result.trajectory; // Copy trajectory if available
        failedLogResult.timestamp = result.timestamp; // Copy start time
        // Ensure endTimestamp is set (even if start time)
        if (result.endTimestamp == std::chrono::high_resolution_clock::time_point{}) {
            failedLogResult.endTimestamp = result.timestamp; 
        } else {
             failedLogResult.endTimestamp = result.endTimestamp;
        }
        failedLogResult.transitionLatency = 0.0f; // Not applicable for failed
        logGestureResult(failedLogResult); // Log the fact that nothing was detected
        result.type = GestureType::NONE; // Ensure main result is NONE
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
    auto startTime = std::chrono::high_resolution_clock::now();

    if (!m_initialized || points.empty()) {
        // Use default constructor and minimal assignment
        GestureResult earlyExitResult;
        earlyExitResult.type = GestureType::NONE;
        earlyExitResult.confidence = 0.0f;
        earlyExitResult.timestamp = startTime;
        earlyExitResult.endTimestamp = startTime;
        return earlyExitResult;
    }

    // Use default constructor and assign relevant fields
    GestureResult result; 
    result.type = GestureType::NONE;
    result.confidence = 0.0f;
    result.position = !points.empty() ? points.back() : cv::Point2f();
    result.trajectory = points;
    result.timestamp = startTime;
    // endTimestamp assigned later

    // Simplified recognition logic for simulation
    // --- Run Recognizers Sequentially --- 
    GestureResult bestResult = result; // Start with NONE

    auto khargailResult = recognizeKhargail(points);
    if (khargailResult.confidence > bestResult.confidence) {
        bestResult = khargailResult;
    }

    auto flammilResult = recognizeFlammil(points);
    if (flammilResult.confidence > bestResult.confidence) {
        bestResult = flammilResult;
    }

    auto stasaiResult = recognizeStasai(points, testCaseId);
    if (stasaiResult.confidence > bestResult.confidence) {
        bestResult = stasaiResult;
    }

    auto annihlatResult = recognizeAnnihlat(points);
    if (annihlatResult.confidence > bestResult.confidence) {
        bestResult = annihlatResult;
    }
    // --- End Sequential Recognition ---

    result = bestResult;
    result.endTimestamp = std::chrono::high_resolution_clock::now(); // Set end time after recognition

    // Set gesture name based on recognized type
    switch (result.type) {
        case GestureType::KHARGAIL: result.gestureName = "Khargail"; break;
        case GestureType::FLAMMIL: result.gestureName = "Flammil"; break;
        case GestureType::STASAI: result.gestureName = "Stasai"; break;
        case GestureType::ANNIHLAT: result.gestureName = "Annihlat"; break;
        default: result.gestureName = (result.confidence > 0.0f ? "FAILED_RECOGNITION" : "NONE"); break;
    }

    // Calculate velocities
    result.velocities = calculateRawVelocities(points);
    result.velocities = normalizeVelocities(result.velocities);
    
    // Calculate debug_velocity (e.g., average magnitude)
    if (!result.velocities.empty()) {
        float sum_sq_vel = 0.0f;
        for(float v : result.velocities) { sum_sq_vel += v * v; } // Using normalized here
        result.debug_velocity = fastSqrt(sum_sq_vel / result.velocities.size());
    } else {
        result.debug_velocity = 0.0f;
    }

    // Update transition stats (if needed for testing)
    if (result.type != GestureType::NONE) {
        float threshold = m_gestureThresholds.count(result.type) ? m_gestureThresholds.at(result.type) : m_minConfidence;
        if (result.confidence >= threshold) {
            updateTransitionStats(result, m_lastRecognizedGesture); // Use m_lastRecognizedGesture for sim
            m_lastRecognizedGesture = result;
        }
    }

    // Log simulation result
    logGestureResult(result);

    return result;
}

GestureResult GestureRecognizer::recognizeKhargail(const std::vector<cv::Point2f>& points) {
    auto startTime = std::chrono::high_resolution_clock::now();
    GestureResult result; // Use default constructor
    result.type = GestureType::NONE;
    result.timestamp = startTime;
    result.position = !points.empty() ? points.back() : cv::Point2f();
    result.trajectory = points;

    if (points.size() < 3) {
        result.endTimestamp = std::chrono::high_resolution_clock::now();
        return result;
    }

    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 0.0f));
    if (confidence >= m_gestureThresholds.at(GestureType::KHARGAIL)) {
        result.type = GestureType::KHARGAIL;
        result.gestureName = "Khargail";
        result.confidence = confidence;
    } else {
        result.confidence = confidence; // Store low confidence for logging/debugging
    }
    
    result.endTimestamp = std::chrono::high_resolution_clock::now();
    return result;
}

GestureResult GestureRecognizer::recognizeFlammil(const std::vector<cv::Point2f>& points) {
    auto startTime = std::chrono::high_resolution_clock::now();
    GestureResult result; // Use default constructor
    result.type = GestureType::NONE;
    result.timestamp = startTime;
    result.position = !points.empty() ? points.back() : cv::Point2f();
    result.trajectory = points;

    if (points.size() < 3) {
        result.endTimestamp = std::chrono::high_resolution_clock::now();
        return result;
    }

    // Corrected direction for Flammil (Right-Down -> (1,1) ? assuming normalized)
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 1.0f)); 
    if (confidence >= m_gestureThresholds.at(GestureType::FLAMMIL)) {
        result.type = GestureType::FLAMMIL;
        result.gestureName = "Flammil";
        result.confidence = confidence;
    } else {
        result.confidence = confidence; // Log low confidence
    }
    
    result.endTimestamp = std::chrono::high_resolution_clock::now();
    return result;
}

GestureResult GestureRecognizer::recognizeStasai(const std::vector<cv::Point2f>& points, const std::string& testCaseId) {
    auto startTime = std::chrono::high_resolution_clock::now();
    GestureResult result; // Use default constructor
    result.type = GestureType::NONE;
    result.timestamp = startTime;
    result.position = !points.empty() ? points.back() : cv::Point2f();
    result.trajectory = points;

    if (isCircle(points, testCaseId)) {
        result.type = GestureType::STASAI;
        result.gestureName = "Stasai";
        // Placeholder confidence - real circle detection would provide a metric
        result.confidence = 0.9f; 
    } else {
        result.confidence = 0.1f; // Log low confidence if not circle
    }
    
    result.endTimestamp = std::chrono::high_resolution_clock::now();
    return result;
}

GestureResult GestureRecognizer::recognizeAnnihlat(const std::vector<cv::Point2f>& points) {
    auto startTime = std::chrono::high_resolution_clock::now();
    GestureResult result; // Use default constructor
    result.type = GestureType::NONE;
    result.timestamp = startTime;
    result.position = !points.empty() ? points.back() : cv::Point2f();
    result.trajectory = points;

    if (points.size() < 3) {
        result.endTimestamp = std::chrono::high_resolution_clock::now();
        return result;
    }

    // Assuming Annihlat is Right-Swipe-Down (matches Flammil direction? Check definition)
    float confidence = calculateSwipeConfidence(points, cv::Point2f(1.0f, 1.0f)); // Using Right-Down like Flammil for now
    if (confidence >= m_gestureThresholds.at(GestureType::ANNIHLAT)) {
        result.type = GestureType::ANNIHLAT;
        result.gestureName = "Annihlat";
        result.confidence = confidence;
    } else {
        result.confidence = confidence; // Log low confidence
    }
    
    result.endTimestamp = std::chrono::high_resolution_clock::now();
    return result;
}

void GestureRecognizer::updateTransitionStats(const GestureResult& current, const GestureResult& previous) {
    // Use high_resolution_clock consistently for latency calculations
    // Note: previous.timestamp is already high_resolution_clock::time_point
    if (previous.type != GestureType::NONE && 
        previous.endTimestamp != std::chrono::high_resolution_clock::time_point{}) { // Ensure previous timestamp is valid
        
        auto now = std::chrono::high_resolution_clock::now(); // Use high_res_clock matching timestamps
        // Ensure current.timestamp is valid before calculating duration
        auto startTimeToUse = (current.timestamp != std::chrono::high_resolution_clock::time_point{}) ? 
                               current.timestamp : now; // Use 'now' as fallback if current.timestamp is invalid
                               
        // Calculate latency using the endTimestamp of the previous gesture and the start timestamp of the current one
        float latency = std::chrono::duration<float>(startTimeToUse - previous.endTimestamp).count();
        
        // Keep the reset logic if gap is too large
        if (latency < 0.0f || latency > 1.0f) { // Also check for negative latency (clock skew?)
            m_lastTransition = {GestureType::NONE, current.type, 0.0f, current.confidence};
            m_averageTransitionLatency = 0.0f; // Reset average on large gap or error
            return;
        }
        
        m_lastTransition = {previous.type, current.type, latency, 
                          std::min(previous.confidence, current.confidence)};
        m_averageTransitionLatency = (m_averageTransitionLatency * 0.9f) + (latency * 0.1f); // EMA update

        // DEBUG OUTPUT FOR TRANSITION
        GESTURE_DEBUG_LOG("UpdateTransitionStats Debug: PrevType=" << static_cast<int>(previous.type)
                         << ", CurrType=" << static_cast<int>(current.type)
                         << ", Latency=" << latency << "s"
                         << ", AvgLatency=" << m_averageTransitionLatency << "s");
    } else if (current.type != GestureType::NONE) {
        // If previous was NONE or invalid, set transition from NONE
        m_lastTransition = {GestureType::NONE, current.type, 0.0f, current.confidence};
        // Do not reset average latency here, wait for a valid transition pair
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

    // Sample every sixth point for better performance
    const size_t step = 6;
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

    // Pre-calculate squared distances
    std::vector<float> squared_distances(sampled_count);
    float avgRadiusSquared = 0.0f;

    // Revert to scalar loop
    for (size_t i = 0; i < sampled_count; ++i) {
        float dx = sampled_points[i].x - center.x;
        float dy = sampled_points[i].y - center.y;
        float distSq = dx * dx + dy * dy;
        squared_distances[i] = distSq;
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

// --- Velocity Calculation Helper Implementations ---

// Calculates raw velocities assuming a fixed time step between points.
// TODO: Consider using actual timestamps if available for more accuracy.
std::vector<float> GestureRecognizer::calculateRawVelocities(const std::vector<cv::Point2f>& points) const {
    std::vector<float> rawVelocities;
    if (points.size() < 2) {
        return rawVelocities; // Need at least two points to calculate velocity
    }

    // Assuming 60 FPS for simulation time step
    const float timeStep = 1.0f / 60.0f; 
    rawVelocities.reserve(points.size() - 1);

    for (size_t i = 1; i < points.size(); ++i) {
        cv::Point2f displacement = points[i] - points[i - 1];
        float distance = fastSqrt(displacement.x * displacement.x + displacement.y * displacement.y);
        float velocity = distance / timeStep;
        rawVelocities.push_back(velocity);
    }

    // Handle the first point potentially? Or assume velocity starts from the second point.
    // For now, the vector has size N-1 compared to N points.
    return rawVelocities;
}

// Normalizes raw velocities to the range [0.0, 1.0]
std::vector<float> GestureRecognizer::normalizeVelocities(const std::vector<float>& rawVelocities) const {
    std::vector<float> normalizedVelocities;
    normalizedVelocities.reserve(rawVelocities.size());

    const float targetMaxVelocity = 1500.0f; // As per Marcus's requirement

    for (float rawVelocity : rawVelocities) {
        if (targetMaxVelocity <= 1e-6) { // Avoid division by zero
            normalizedVelocities.push_back(0.0f);
        } else {
            float normalized = rawVelocity / targetMaxVelocity;
            // Clamp the value to [0.0, 1.0]
            normalizedVelocities.push_back(std::max(0.0f, std::min(1.0f, normalized)));
        }
    }

    return normalizedVelocities;
}

// --- End Velocity Helpers ---

} // namespace CSL
} // namespace TurtleEngine 