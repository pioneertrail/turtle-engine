#pragma once

#include "GestureRecognizer.hpp"
#include <opencv2/opencv.hpp>
#include <functional>
#include <memory>
#include <queue>
#include <mutex>

namespace TurtleEngine {
namespace CSL {

using GestureCallback = std::function<void(const GestureResult&)>;

class CSLSystem {
public:
    CSLSystem();
    ~CSLSystem();

    // Initialize the CSL system
    bool initialize(int cameraIndex = 0);
    
    // Start/stop the CSL system
    bool start();
    void stop();

    // Register a callback for gesture events
    void registerGestureCallback(GestureCallback callback);

    // Update the CSL system (should be called each frame)
    void update();

    // Get the current camera frame
    cv::Mat getCurrentFrame() const;

    // Get the last gesture result
    GestureResult getLastGestureResult() const;

    // Configuration methods
    void setGestureSensitivity(float sensitivity);
    void setMinGestureConfidence(float confidence);
    void setCameraResolution(int width, int height);

private:
    // Camera capture thread function
    void cameraCaptureThread();
    
    // Process captured frames
    void processFrame(const cv::Mat& frame);

    // Member variables
    std::unique_ptr<GestureRecognizer> m_gestureRecognizer;
    cv::VideoCapture m_camera;
    std::queue<cv::Mat> m_frameQueue;
    std::mutex m_frameMutex;
    std::vector<GestureCallback> m_callbacks;
    bool m_running;
    bool m_initialized;
    int m_cameraIndex;
    cv::Size m_cameraResolution;
    
    // Last gesture result
    mutable std::mutex m_resultMutex;
    GestureResult m_lastGestureResult;
};

} // namespace CSL
} // namespace TurtleEngine 