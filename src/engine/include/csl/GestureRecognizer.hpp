#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>
#include <chrono>
#include <map>
#include <fstream>

namespace TurtleEngine {
namespace CSL {

enum class GestureType {
    NONE,
    KHARGAIL,    // Left-right charge
    FLAMMIL,     // Right-down swipe
    STASAI,      // Tight circle
    ANNIHLAT,    // Right swipe down
    TBD          // To be determined
};

struct GestureResult {
    GestureType type;
    float confidence;
    cv::Point2f position;
    std::vector<cv::Point2f> trajectory;
    std::vector<float> velocities; // Normalized velocities (0.0 to 1.0)
    std::chrono::high_resolution_clock::time_point timestamp;  // Start time
    std::chrono::high_resolution_clock::time_point endTimestamp;
    float transitionLatency;  // Time since last gesture
};

struct ComboTransition {
    GestureType from;
    GestureType to;
    float latency;
    float confidence;
};

class GestureRecognizer {
public:
    GestureRecognizer();
    ~GestureRecognizer();

    // Initialize the gesture recognizer
    bool initialize();

    // Process a new frame for gesture recognition
    GestureResult processFrame(const cv::Mat& frame);

    // Set gesture recognition parameters
    void setSensitivity(float sensitivity);
    void setMinConfidence(float confidence);
    float getMinConfidence() const { return m_minConfidence; }

    // Combo and transition tracking
    ComboTransition getLastTransition() const { return m_lastTransition; }
    float getAverageTransitionLatency() const { return m_averageTransitionLatency; }
    void resetTransitionStats();
    float getGestureThreshold(GestureType type) const;

    // Add for test simulation
    GestureResult processSimulatedPoints(const std::vector<cv::Point2f>& points, const std::string& testCaseId = "Unknown");

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
    void logGestureResult(const GestureResult& result);

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
    std::ofstream m_logFile; // Uncommented
    float m_circleClosureThreshold; // Added circle closure threshold
    
    // New member variables for gesture-specific tracking
    std::map<GestureType, float> m_gestureThresholds;
    std::map<GestureType, int> m_gestureAttempts;
    std::map<GestureType, int> m_gestureSuccesses;
};

} // namespace CSL
} // namespace TurtleEngine 