#pragma once

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <memory>

namespace TurtleEngine {
namespace CSL {

enum class GestureType {
    NONE,
    SWIPE_LEFT,
    SWIPE_RIGHT,
    SWIPE_UP,
    SWIPE_DOWN,
    CIRCLE,
    TAP,
    DOUBLE_TAP
};

struct GestureResult {
    GestureType type;
    float confidence;
    cv::Point2f position;
    std::vector<cv::Point2f> trajectory;
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

private:
    // Internal gesture recognition methods
    GestureResult recognizeSwipe(const std::vector<cv::Point2f>& points);
    GestureResult recognizeCircle(const std::vector<cv::Point2f>& points);
    GestureResult recognizeTap(const std::vector<cv::Point2f>& points);

    // Helper methods
    float calculateSwipeConfidence(const std::vector<cv::Point2f>& points, const cv::Point2f& direction);
    bool isCircle(const std::vector<cv::Point2f>& points);
    bool isTap(const std::vector<cv::Point2f>& points);

    // Member variables
    float m_sensitivity;
    float m_minConfidence;
    std::vector<cv::Point2f> m_previousPoints;
    cv::Point2f m_lastTapPosition;
    double m_lastTapTime;
    bool m_initialized;
};

} // namespace CSL
} // namespace TurtleEngine 