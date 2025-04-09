#include "csl/CSLSystem.hpp"
#include <iostream>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <chrono>
#include <thread>
#include <cmath>

// Simple visual feedback for gesture detection
void drawGestureFeedback(cv::Mat& frame, const TurtleEngine::CSL::GestureResult& result) {
    // Draw the gesture trajectory
    for (size_t i = 1; i < result.trajectory.size(); ++i) {
        cv::line(frame, result.trajectory[i-1], result.trajectory[i], cv::Scalar(0, 255, 0), 2);
    }
    
    // Draw the current position
    if (!result.trajectory.empty()) {
        cv::circle(frame, result.trajectory.back(), 5, cv::Scalar(0, 0, 255), -1);
    }
    
    // Draw gesture type and confidence
    std::string gestureText;
    switch (result.type) {
        case TurtleEngine::CSL::GestureType::SWIPE_RIGHT:
            gestureText = "SWIPE_RIGHT";
            break;
        case TurtleEngine::CSL::GestureType::SWIPE_LEFT:
            gestureText = "SWIPE_LEFT";
            break;
        case TurtleEngine::CSL::GestureType::SWIPE_UP:
            gestureText = "SWIPE_UP";
            break;
        case TurtleEngine::CSL::GestureType::SWIPE_DOWN:
            gestureText = "SWIPE_DOWN";
            break;
        case TurtleEngine::CSL::GestureType::CIRCLE:
            gestureText = "CIRCLE";
            break;
        case TurtleEngine::CSL::GestureType::TAP:
            gestureText = "TAP";
            break;
        case TurtleEngine::CSL::GestureType::DOUBLE_TAP:
            gestureText = "DOUBLE_TAP";
            break;
        default:
            gestureText = "NONE";
            break;
    }
    
    // Add confidence to the text
    std::string text = gestureText + " (" + std::to_string(static_cast<int>(result.confidence * 100)) + "%)";
    
    // Draw the text
    cv::putText(frame, text, cv::Point(10, 30), cv::FONT_HERSHEY_SIMPLEX, 0.7, cv::Scalar(255, 255, 255), 2);
    
    // Special handling for "annihilate" gesture (SWIPE_RIGHT with high confidence)
    // Only show flare when gesture is nearly complete (at least 25 points, ~0.4s)
    if (result.type == TurtleEngine::CSL::GestureType::SWIPE_RIGHT && 
        result.confidence >= 0.7f && 
        result.trajectory.size() >= 25) {
        
        // Calculate flare intensity based on confidence
        int intensity = static_cast<int>(result.confidence * 255);
        
        // Calculate normalized flare intensity (0-1 from 0.7-1.0)
        float flareIntensity = (result.confidence - 0.7f) / 0.3f;
        flareIntensity = std::max(0.0f, std::min(1.0f, flareIntensity));
        
        // Draw a red circle at the end of the trajectory
        if (!result.trajectory.empty()) {
            int radius = static_cast<int>(result.confidence * 50);
            cv::circle(frame, result.trajectory.back(), radius, cv::Scalar(0, 0, intensity), -1);
            
            // Add a glow effect
            for (int i = 1; i <= 3; ++i) {
                cv::circle(frame, result.trajectory.back(), radius + i * 5, 
                          cv::Scalar(0, 0, intensity / (i + 1)), 2);
            }
            
            // Add a subtle green pulse along the trajectory (Flammyx's fiery path)
            // Create a copy of the frame for the overlay
            cv::Mat overlay;
            frame.copyTo(overlay);
            
            // Get current time for animation
            auto currentTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::system_clock::now().time_since_epoch()).count();
            
            // Draw the green pulse along the trajectory
            for (size_t i = 0; i < result.trajectory.size(); ++i) {
                // Calculate pulse intensity based on position in trajectory
                float t = static_cast<float>(i) / static_cast<float>(result.trajectory.size());
                
                // Create a pulsing effect with slower sine wave (5.0f instead of 10.0f)
                // and slower time component (400.0f instead of 200.0f) for gentler rhythm
                float pulse = 0.5f + 0.5f * std::sin(t * 5.0f + currentTime / 400.0f);
                
                // Scale pulse by confidence
                pulse *= result.confidence;
                
                // Apply pulse cohesion - fade the pulse as the flare peaks
                // This creates a visual effect where the pulse transitions into the flare
                // NOTE: After testing with Elena's plasma effects (April 12), we may need to adjust this
                // to pulse *= (1.0f - flareIntensity * 1.2f) if the pulse lingers too much
                pulse *= (1.0f - flareIntensity);
                
                // Draw a green circle with alpha blending
                int alpha = static_cast<int>(pulse * 128); // Max alpha of 0.5
                int size = static_cast<int>(5 + pulse * 10); // Size varies with pulse
                
                cv::circle(overlay, result.trajectory[i], size, cv::Scalar(0, 255, 0), -1);
            }
            
            // Apply the overlay with alpha blending
            cv::addWeighted(overlay, 0.5, frame, 0.5, 0, frame);
        }
    }
}

int main() {
    std::cout << "Initializing CSL Animation System Test Application..." << std::endl;
    
    // Create and initialize the CSL system
    TurtleEngine::CSL::CSLSystem csl;
    if (!csl.initialize(0)) {
        std::cerr << "Failed to initialize CSL system. Please check your camera connection." << std::endl;
        return -1;
    }
    
    std::cout << "CSL System initialized successfully." << std::endl;
    
    // Register a callback for gesture detection
    csl.registerGestureCallback([](const TurtleEngine::CSL::GestureResult& result) {
        if (result.type == TurtleEngine::CSL::GestureType::SWIPE_RIGHT && result.confidence >= 0.7f) {
            std::cout << "ANNIHILATE detected! Confidence: " << result.confidence << std::endl;
            // In the full implementation, this will trigger Elena's plasma effect
        }
    });
    
    // Start the CSL system
    if (!csl.start()) {
        std::cerr << "Failed to start CSL system." << std::endl;
        return -1;
    }
    
    std::cout << "CSL System started. Perform the 'annihilate' gesture (right swipe down)..." << std::endl;
    std::cout << "Press ESC to exit." << std::endl;
    
    // Create a window to display the camera feed
    cv::namedWindow("CSL Animation System Test", cv::WINDOW_NORMAL);
    cv::resizeWindow("CSL Animation System Test", 800, 600);
    
    // Main loop
    while (true) {
        // Update the CSL system
        csl.update();
        
        // Get the current frame
        cv::Mat frame = csl.getCurrentFrame();
        if (!frame.empty()) {
            // Draw gesture feedback on the frame
            TurtleEngine::CSL::GestureResult result = csl.getLastGestureResult();
            drawGestureFeedback(frame, result);
            
            // Display the frame
            cv::imshow("CSL Animation System Test", frame);
        }
        
        // Check for ESC key to exit
        int key = cv::waitKey(1);
        if (key == 27) { // ESC key
            break;
        }
        
        // Add a small delay to reduce CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    // Stop the CSL system
    csl.stop();
    std::cout << "CSL System stopped." << std::endl;
    
    // Close the window
    cv::destroyAllWindows();
    
    return 0;
} 