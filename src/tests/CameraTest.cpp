#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open camera" << std::endl;
        return -1;
    }
    
    std::cout << "Camera opened successfully" << std::endl;
    std::cout << "Press ESC to exit" << std::endl;
    
    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not capture frame" << std::endl;
            break;
        }
        
        cv::imshow("Camera Test", frame);
        char c = (char)cv::waitKey(1);
        if (c == 27) // ESC key
            break;
        
        // Check if window was closed by user clicking X
        if (cv::getWindowProperty("Camera Test", cv::WND_PROP_VISIBLE) < 1) {
            std::cout << "Window closed by user" << std::endl;
            break;
        }
    }
    
    cap.release();
    cv::destroyAllWindows();
    
    // Ensure the program exits completely
    std::cout << "Camera test exiting" << std::endl;
    cv::waitKey(100); // Small delay to ensure cleanup
    return 0;
} 