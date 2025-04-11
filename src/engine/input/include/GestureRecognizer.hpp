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

// --- Constants ---
namespace GestureConstants {
    constexpr float DEFAULT_SENSITIVITY = 1.0f;
    constexpr float DEFAULT_MIN_CONFIDENCE = 0.7f;
    constexpr float KHARGAIL_THRESHOLD = 0.75f;
    constexpr float FLAMMIL_THRESHOLD = 0.80f;
    constexpr float STASAI_THRESHOLD = 0.85f;
    constexpr float ANNIHLAT_THRESHOLD = 0.70f;
    // Angle/Straightness thresholds (Consider making these configurable)
    constexpr float SWIPE_STRAIGHTNESS_THRESHOLD = 0.8f;
    constexpr float HORIZONTAL_ANGLE_THRESHOLD = 30.0f;
    constexpr float FLAMMIL_ANGLE_MIN = 30.0f;
    constexpr float FLAMMIL_ANGLE_MAX = 60.0f;
    constexpr float ANNIHLAT_ANGLE_MIN = -60.0f;
    constexpr float ANNIHLAT_ANGLE_MAX = -30.0f;
    constexpr float CIRCULARITY_THRESHOLD = 0.7f;
}

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
    float getMinConfidence() const { return m_minConfidence; }

    // Combo and transition tracking
    ComboTransition getLastTransition() const { return m_lastTransition; }
    float getAverageTransitionLatency() const { return m_averageTransitionLatency; }
    void resetTransitionStats();
    float getGestureThreshold(GestureType type) const;
    void setGestureThreshold(GestureType type, float threshold);

    // Debug visualization methods
    void setDebugMode(bool enabled) { m_debugMode = enabled; }
    bool isDebugMode() const { return m_debugMode; }
    void setDebugLogLevel(int level) { m_debugLogLevel = level; }
    int getDebugLogLevel() const { return m_debugLogLevel; }
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
    float m_sensitivity;
    float m_minConfidence;
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
    bool m_debugMode = false;
    int m_debugLogLevel = 0;
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