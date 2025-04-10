#include "csl/CSLSystem.hpp"
#include <thread>
#include <chrono>

namespace TurtleEngine {
namespace CSL {

CSLSystem::CSLSystem()
    : m_running(false)
    , m_initialized(false)
    , m_cameraIndex(0)
    , m_cameraResolution(640, 480)
    , m_lastGestureResult{GestureType::NONE, 0.0f, cv::Point2f(), std::vector<cv::Point2f>()}
    , m_plasmaDuration(0.1f)
{
    m_gestureRecognizer = std::make_unique<GestureRecognizer>();
}

CSLSystem::~CSLSystem() {
    stop();
}

bool CSLSystem::initialize(int cameraIndex) {
    if (m_initialized) {
        return true;
    }

    m_cameraIndex = cameraIndex;
    m_camera.open(m_cameraIndex);
    
    if (!m_camera.isOpened()) {
        return false;
    }

    m_camera.set(cv::CAP_PROP_FRAME_WIDTH, m_cameraResolution.width);
    m_camera.set(cv::CAP_PROP_FRAME_HEIGHT, m_cameraResolution.height);

    m_initialized = true;
    return true;
}

bool CSLSystem::start() {
    if (!m_initialized || m_running) {
        return false;
    }

    m_running = true;
    std::thread([this]() { cameraCaptureThread(); }).detach();
    return true;
}

void CSLSystem::stop() {
    m_running = false;
    if (m_camera.isOpened()) {
        m_camera.release();
    }
    m_initialized = false;
}

void CSLSystem::registerGestureCallback(GestureCallback callback) {
    m_callbacks.push_back(callback);
}

void CSLSystem::addPlasmaCallback(std::function<void(const GestureResult&)> callback) {
    m_plasmaCallbacks.push_back(callback);
}

void CSLSystem::setPlasmaDuration(float duration) {
    m_plasmaDuration = std::max(0.1f, duration); // Ensure duration is positive
    std::cout << "CSLSystem::setPlasmaDuration: Set to " << m_plasmaDuration << "s" << std::endl;
}

void CSLSystem::triggerPlasmaCallback(const GestureResult& result) {
    // This is primarily for testing the callback mechanism directly.
    // It bypasses the usual processFrame logic.
    if (result.type == GestureType::FLAMMIL) { // Still check type for safety
        float flammilThreshold = m_gestureRecognizer ? m_gestureRecognizer->getGestureThreshold(GestureType::FLAMMIL) : 0.75f; // Default if no recognizer
        if (result.confidence >= flammilThreshold) {
             std::cout << "CSLSystem::triggerPlasmaCallback: Invoking callbacks for Flammil." << std::endl;
            for (const auto& plasmaCallback : m_plasmaCallbacks) {
                plasmaCallback(result);
            }
        } else {
            std::cout << "CSLSystem::triggerPlasmaCallback: Flammil confidence too low (" << result.confidence << " < " << flammilThreshold << "), not invoking callbacks." << std::endl;
        }
    } else {
         std::cout << "CSLSystem::triggerPlasmaCallback: Result type is not Flammil, not invoking callbacks." << std::endl;
    }
}

void CSLSystem::update() {
    if (!m_running) {
        return;
    }

    cv::Mat frame;
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (!m_frameQueue.empty()) {
            frame = m_frameQueue.front();
            m_frameQueue.pop();
        }
    }

    if (!frame.empty()) {
        processFrame(frame);
    }
}

cv::Mat CSLSystem::getCurrentFrame() const {
    std::lock_guard<std::mutex> lock(const_cast<std::mutex&>(m_frameMutex));
    if (!m_frameQueue.empty()) {
        return m_frameQueue.back();
    }
    return cv::Mat();
}

GestureResult CSLSystem::getLastGestureResult() const {
    std::lock_guard<std::mutex> lock(m_resultMutex);
    return m_lastGestureResult;
}

void CSLSystem::setGestureSensitivity(float sensitivity) {
    if (m_gestureRecognizer) {
        m_gestureRecognizer->setSensitivity(sensitivity);
    }
}

void CSLSystem::setMinGestureConfidence(float confidence) {
    if (m_gestureRecognizer) {
        m_gestureRecognizer->setMinConfidence(confidence);
    }
}

void CSLSystem::setCameraResolution(int width, int height) {
    m_cameraResolution = cv::Size(width, height);
    if (m_camera.isOpened()) {
        m_camera.set(cv::CAP_PROP_FRAME_WIDTH, width);
        m_camera.set(cv::CAP_PROP_FRAME_HEIGHT, height);
    }
}

void CSLSystem::cameraCaptureThread() {
    while (m_running) {
        cv::Mat frame;
        if (m_camera.read(frame)) {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            if (m_frameQueue.size() >= 2) { // Keep only the most recent frames
                m_frameQueue.pop();
            }
            m_frameQueue.push(frame);
            
            // Check for potential lag issues
            if (m_frameQueue.size() > 10) {
                std::cerr << "Frame queue overflow - potential lag detected\n";
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(16)); // ~60 FPS
    }
}

void CSLSystem::processFrame(const cv::Mat& frame) {
    if (!m_gestureRecognizer) {
        return;
    }
    // Process frame using the recognizer
    GestureResult result = m_gestureRecognizer->processFrame(frame);
    
    // Store the actual last gesture result from camera input
    {
        std::lock_guard<std::mutex> lock(m_resultMutex);
        m_lastGestureResult = result;
    }
    
    // Invoke callbacks based on confidence etc. from actual recognition
    // Use the default minimum confidence for general callbacks
    if (result.confidence >= m_gestureRecognizer->getMinConfidence()) { 
         invokeCallbacks(result);
    }
}

// Updated method to trigger gesture from code, accepting trigger time
void CSLSystem::triggerGesture(GestureType type, std::chrono::high_resolution_clock::time_point triggerTime) {
    if (type == GestureType::NONE) return;

    // Construct a simulated high-confidence result
    GestureResult result;
    result.type = type;
    result.confidence = 1.0f; // Simulate high confidence
    result.position = cv::Point2f(0, 0); // Dummy position
    result.timestamp = std::chrono::high_resolution_clock::now(); // Time callback is invoked
    result.endTimestamp = result.timestamp; // Instantaneous
    result.transitionLatency = 0.0f; 
    result.triggerTimestamp = triggerTime; // Store the original trigger time

    std::cout << "CSLSystem: Triggering gesture type " << static_cast<int>(type) << std::endl;
    invokeCallbacks(result);
}

// Helper to invoke callbacks (handles general and specific like plasma)
void CSLSystem::invokeCallbacks(const GestureResult& result) {
     // Notify general callbacks
    for (const auto& callback : m_callbacks) {
        callback(result);
    }

    // Specific callbacks (e.g., Plasma for Flammil)
    if (result.type == GestureType::FLAMMIL) {
        float flammilThreshold = m_gestureRecognizer ? m_gestureRecognizer->getGestureThreshold(GestureType::FLAMMIL) : 0.75f; 
        if (result.confidence >= flammilThreshold) {
             for (const auto& plasmaCallback : m_plasmaCallbacks) {
                 plasmaCallback(result);
             }
        }
    }
    // TODO: Add similar specific callback logic for other gestures/effects if needed
}

} // namespace CSL
} // namespace TurtleEngine 