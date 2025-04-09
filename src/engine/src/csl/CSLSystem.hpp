#pragma once

#include "csl/GestureRecognizer.hpp"
#include <opencv2/opencv.hpp>
#include <queue>
#include <mutex>
#include <functional>

namespace TurtleEngine {
namespace CSL {

using GestureCallback = std::function<void(const GestureResult&)>;

class CSLSystem {
public:
    CSLSystem();
    ~CSLSystem();

    bool initialize(int cameraIndex = 0);
    bool start();
    void stop();

    void registerGestureCallback(GestureCallback callback);
    void update();

    cv::Mat getCurrentFrame() const;
    GestureResult getLastGestureResult() const;

    void setGestureSensitivity(float sensitivity);
    void setMinGestureConfidence(float confidence);
    void setCameraResolution(int width, int height);

private:
    void cameraCaptureThread();
    void processFrame(const cv::Mat& frame);

    bool m_running;
    bool m_initialized;
    int m_cameraIndex;
    cv::Size m_cameraResolution;
    cv::VideoCapture m_camera;
    std::unique_ptr<GestureRecognizer> m_gestureRecognizer;
    std::vector<GestureCallback> m_callbacks;
    std::queue<cv::Mat> m_frameQueue;
    GestureResult m_lastGestureResult;

    mutable std::mutex m_frameMutex;
    mutable std::mutex m_resultMutex;
};

} // namespace CSL
} // namespace TurtleEngine 