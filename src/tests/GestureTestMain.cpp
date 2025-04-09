#include "GestureTest.hpp"
#include <iostream>

int main() {
    std::cout << "CSL Animation System - Gesture Recognition Test" << std::endl;
    std::cout << "=============================================" << std::endl;
    
    // Run the gesture recognition test
    TurtleEngine::CSL::Test::testGestureRecognition();
    
    return 0;
} 