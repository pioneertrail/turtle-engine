# Build script for TemporalCollisionTest using MSVC
Write-Host "Building TemporalCollisionTest with MSVC..."

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
#include <glm/gtx/string_cast.hpp> // Include string_cast for debug output

// Simple test to verify GLM functionality
int main() {
    std::cout << "=== GLM Include Test ===" << std::endl;
    
    // Create a vector
    glm::vec3 testVector(1.0f, 2.0f, 3.0f);
    
    // Print the vector using glm::to_string
    std::cout << "Test vector: " << glm::to_string(testVector) << std::endl;
    
    // Test random functionality
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    
    glm::vec3 randomVector(
        dist(gen),
        dist(gen),
        dist(gen)
    );
    
    std::cout << "Random vector: " << glm::to_string(randomVector) << std::endl;
    
    // Test distance calculation
    float distance = glm::distance(testVector, randomVector);
    std::cout << "Distance between vectors: " << distance << std::endl;
    
    // Test smoothstep
    float factor = glm::smoothstep(0.0f, 1.0f, 0.5f);
    std::cout << "Smoothstep result: " << factor << std::endl;
    
    return 0;
}
"@

# Write to a temporary file
$testFile = "glm_test.cpp"
$testFile | Out-File -Encoding utf8 $testFile
$testRunner | Out-File -Encoding utf8 -Append $testFile

# Check if we can find Visual Studio's environment
$vsPath = ""
$vsVersions = @(
    "Enterprise",
    "Professional", 
    "Community"
)

foreach ($version in $vsVersions) {
    $potentialPath = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\2019\$version\VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $potentialPath) {
        $vsPath = $potentialPath
        break
    }
    
    # Check for VS 2022 as well
    $potentialPath = "${env:ProgramFiles}\Microsoft Visual Studio\2022\$version\VC\Auxiliary\Build\vcvars64.bat"
    if (Test-Path $potentialPath) {
        $vsPath = $potentialPath
        break
    }
}

if ([string]::IsNullOrEmpty($vsPath)) {
    Write-Host "Could not find Visual Studio installation. Please make sure Visual Studio is installed."
    exit 1
}

# Compile using MSVC by creating a temporary batch file
$batchFile = "compile_test.bat"
@"
@echo off
call "$vsPath"
cl /std:c++17 /EHsc /DGLM_ENABLE_EXPERIMENTAL /I"vcpkg/installed/x64-windows/include" /Feglm_test.exe $testFile
"@ | Out-File -Encoding ascii $batchFile

# Run the batch file
Write-Host "Compiling test with MSVC..."
cmd /c $batchFile

# Run the test if compilation succeeded
if (Test-Path "glm_test.exe") {
    Write-Host "Compilation successful! Running test..."
    .\glm_test.exe
} else {
    Write-Host "Compilation failed."
}

# Clean up
Remove-Item $batchFile -ErrorAction SilentlyContinue 