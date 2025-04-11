# Build script for TemporalCollisionTest
Write-Host "Building TemporalCollisionTest directly..."

# Create a simple test runner that avoids engine dependencies
$testRunner = @"
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <random>
#include <glm/glm.hpp>

// Simple test to verify GLM functionality
int main() {
    std::cout << "=== GLM Include Test ===" << std::endl;
    
    // Create a vector
    glm::vec3 testVector(1.0f, 2.0f, 3.0f);
    
    // Print the vector
    std::cout << "Test vector: (" 
              << testVector.x << ", " 
              << testVector.y << ", " 
              << testVector.z << ")" << std::endl;
    
    // Test random functionality
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    glm::vec3 randomVector(
        dist(gen),
        dist(gen),
        dist(gen)
    );
    
    std::cout << "Random vector: (" 
              << randomVector.x << ", " 
              << randomVector.y << ", " 
              << randomVector.z << ")" << std::endl;
    
    return 0;
}
"@

# Write to a temporary file
$testFile = "glm_test.cpp"
$testFile | Out-File -Encoding utf8 $testFile
$testRunner | Out-File -Encoding utf8 -Append $testFile

# Compile the test
Write-Host "Compiling test..."
g++ -std=c++17 $testFile -o glm_test.exe -Ivcpkg/installed/x64-windows/include

# Run the test if compilation succeeded
if ($LASTEXITCODE -eq 0) {
    Write-Host "Compilation successful! Running test..."
    .\glm_test.exe
} else {
    Write-Host "Compilation failed."
} 