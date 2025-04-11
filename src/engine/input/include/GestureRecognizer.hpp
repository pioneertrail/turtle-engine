#pragma once

#include <opencv2/core.hpp>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <fstream>
#include <optional>

namespace TurtleEngine {
namespace Input {

enum class GestureType {
    NONE,
    KHARGAIL,    // Left-right charge
    FLAMMIL,     // Right-down swipe
    STASAI,      // Tight circle
    ANNIHLAT,    // Right swipe up
    TBD          // To be determined
};

struct GestureResult {
    GestureType type;
    float confidence;
    cv::Point2f position;
    std::vector<cv::Point2f> trajectory;
    std::vector<float> velocities; // Normalized velocities (0.0 to 1.0)
    std::chrono::high_resolution_clock::time_point timestamp;  // Recognition start time
    std::chrono::high_resolution_clock::time_point endTimestamp; // Recognition end time
    float transitionLatency;  // Time since last *recognized* gesture
    
    // Optional field for latency measurement of triggered gestures
    std::optional<std::chrono::high_resolution_clock::time_point> triggerTimestamp;

    // Debug visualization data
    std::vector<cv::Point2f> debugPoints;
    std::vector<float> debugConfidences;
    std::string debugInfo;
};

struct ComboTransition {
    GestureType from;
    GestureType to;
    float latency;
    float confidence;
    std::string debugInfo;
};

class GestureRecognizer {
public:
    GestureRecognizer();
    ~GestureRecognizer();

    // Initialize the gesture recognizer
    void initialize();

    // Process a new frame for gesture recognition
    GestureResult processFrame(const cv::Mat& frame);

    // Set gesture recognition parameters
    void setSensitivity(float sensitivity);
    void setMinConfidence(float minConfidence);
    float getMinConfidence() const { return minConfidence_; }

    // Combo and transition tracking
    ComboTransition getLastTransition() const { return m_lastTransition; }
    float getAverageTransitionLatency() const { return m_averageTransitionLatency; }
    void resetTransitionStats();
    float getGestureThreshold(GestureType type) const;
    void setGestureThreshold(GestureType type, float threshold);

    // Debug visualization methods
    void setDebugMode(bool enabled) { debugMode_ = enabled; }
    bool isDebugMode() const { return debugMode_; }
    void setDebugLogLevel(int level) { debugLogLevel_ = level; }
    int getDebugLogLevel() const { return debugLogLevel_; }
    const std::vector<cv::Point2f>& getDebugPoints() const { return m_debugPoints; }

    // Test automation methods
    GestureResult processSimulatedPoints(const std::vector<cv::Point2f>& points, const std::string& testId);
    void setTestMode(bool enabled) { m_testMode = enabled; }
    bool isTestMode() const { return m_testMode; }

    // Circle closure threshold methods
    void setCircleClosureThreshold(float threshold);
    float getCircleClosureThreshold() const;

private:
    // Internal gesture recognition methods
    GestureResult recognizeKhargail(const std::vector<cv::Point2f>& points);
    GestureResult recognizeFlammil(const std::vector<cv::Point2f>& points);
    GestureResult recognizeStasai(const std::vector<cv::Point2f>& points, const std::string& testCaseId);
    GestureResult recognizeAnnihlat(const std::vector<cv::Point2f>& points);

    // Helper methods
    float calculateSwipeConfidence(const std::vector<cv::Point2f>& points, const cv::Point2f& direction);
    bool isCircle(const std::vector<cv::Point2f>& points, const std::string& testCaseId = "Unknown");
    void updateTransitionStats(const GestureResult& current, const GestureResult& previous);
    void logGestureResult(const GestureResult& result, int level = 0);
    void logDebugInfo(const std::string& message, int level = 0);

    // Velocity calculation helpers
    std::vector<float> calculateRawVelocities(const std::vector<cv::Point2f>& points) const;
    std::vector<float> normalizeVelocities(const std::vector<float>& rawVelocities) const;

    // Member variables
    float sensitivity_;
    float minConfidence_;
    std::vector<cv::Point2f> m_previousPoints;
    GestureResult m_lastGesture;
    GestureResult m_lastRecognizedGesture;
    ComboTransition m_lastTransition;
    float m_averageTransitionLatency;
    std::chrono::high_resolution_clock::time_point m_lastGestureTime;
    bool m_initialized;
    std::ofstream m_logFile;
    float m_circleClosureThreshold;
    
    // Debug and test state
    bool debugMode_ = false;
    int debugLogLevel_ = 0;
    bool m_testMode = false;
    std::vector<cv::Point2f> m_debugPoints;
    std::vector<float> m_debugConfidences;
    
    // Gesture-specific tracking
    std::map<GestureType, float> m_gestureThresholds;
    std::map<GestureType, int> m_gestureAttempts;
    std::map<GestureType, int> m_gestureSuccesses;
    std::map<GestureType, std::vector<float>> m_gestureConfidenceHistory;
};

} // namespace Input
} // namespace TurtleEngine 