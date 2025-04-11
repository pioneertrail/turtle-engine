#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp>

int main() {
    std::cout << "=== GLM Include Test ===" << std::endl;
    
    // Create a vector
    glm::vec3 testVector(1.0f, 2.0f, 3.0f);
    
    // Print the vector
    std::cout << "Test vector: " 
              << glm::to_string(testVector) << std::endl;
    
    // Test basic GLM functionality
    glm::vec3 secondVector(3.0f, 2.0f, 1.0f);
    float distance = glm::distance(testVector, secondVector);
    
    std::cout << "Distance between vectors: " << distance << std::endl;
    
    // Test smoothstep
    float factor = glm::smoothstep(0.0f, 1.0f, 0.5f);
    std::cout << "Smoothstep result: " << factor << std::endl;
    
    return 0;
}
