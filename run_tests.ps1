# PowerShell script to run all component tests for Phase 1 of Silent Forge
Write-Host "==== Silent Forge: Phase 1 Component Tests ===="

# Find Visual Studio installation
$vsPath = "C:\Program Files\Microsoft Visual Studio\2022\Community"
if (Test-Path $vsPath) {
    Write-Host "Visual Studio found at $vsPath"
} else {
    Write-Host "Visual Studio not found at expected location. Please update the script with the correct path."
    exit 1
}

# Import Visual Studio environment
$vcvarsPath = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
if (Test-Path $vcvarsPath) {
    Write-Host "Found MSVC environment script at $vcvarsPath"
} else {
    Write-Host "MSVC environment script not found at $vcvarsPath"
    exit 1
}

# Setup mock implementations directory
$mocksDir = "mocks"
if (-not (Test-Path $mocksDir)) {
    New-Item -ItemType Directory -Path $mocksDir | Out-Null
}

# Create mocks for dependencies
Write-Host "Creating mock implementations..."

# 1. Mock OpenCV
$mockOpenCVFile = Join-Path $mocksDir "opencv2/core.hpp"
New-Item -ItemType Directory -Path (Join-Path $mocksDir "opencv2") -Force | Out-Null
@"
// Mock implementation of OpenCV core for testing
#pragma once
#ifndef OPENCV2_CORE_HPP
#define OPENCV2_CORE_HPP

#include <vector>

namespace cv {
    class Mat {
    public:
        Mat() = default;
        Mat(int rows, int cols, int type) : rows(rows), cols(cols) {}
        int rows = 0;
        int cols = 0;
        void create(int rows, int cols, int type) { this->rows = rows; this->cols = cols; }
        bool empty() const { return rows == 0 || cols == 0; }
    };
    
    template<typename T>
    class Point_ {
    public:
        T x, y;
        Point_() : x(0), y(0) {}
        Point_(T x, T y) : x(x), y(y) {}
    };
    
    using Point2f = Point_<float>;
    
    template<typename T>
    class Rect_ {
    public:
        T x, y, width, height;
        Rect_() : x(0), y(0), width(0), height(0) {}
        Rect_(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
    };
    
    using Rect = Rect_<int>;
}

#endif
"@ | Out-File -FilePath $mockOpenCVFile -Encoding ASCII

# 2. Mock ParticleSystem header
$mockParticleSystemFile = Join-Path $mocksDir "ParticleSystem.hpp"
@"
// Mock implementation of ParticleSystem for testing
#pragma once
#ifndef PARTICLE_SYSTEM_HPP
#define PARTICLE_SYSTEM_HPP

#include <memory>
#include <iostream>
#include <glm/glm.hpp>

namespace TurtleEngine {
    class ParticleSystem {
    public:
        ParticleSystem(size_t maxParticles = 10000);
        virtual ~ParticleSystem();
        
        virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                         const glm::vec4& color, float size, float life);
        
        virtual void initialize();
        virtual void update(float deltaTime);
        virtual void render();
        
        virtual void spawnBurst(const glm::vec3& position, size_t count,
                              float minVel, float maxVel,
                              const glm::vec4& color, float size, float life);
        
        virtual void spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                 const glm::vec4& color, float size, float life);
    };
    
    // Also provide a mock for the Graphics namespace version used in some components
    namespace Graphics {
        class ParticleSystem {
        public:
            ParticleSystem(size_t maxParticles = 10000);
            virtual ~ParticleSystem();
            
            virtual void emit(const glm::vec3& position, const glm::vec3& velocity,
                            const glm::vec4& color, float size, float life);
            
            virtual void initialize();
            virtual void update(float deltaTime);
            virtual void render();
            
            virtual void emitBurst(const glm::vec3& position, size_t count,
                                float minVel, float maxVel,
                                const glm::vec4& color, float size, float life);
        };
    }
}

#endif
"@ | Out-File -FilePath $mockParticleSystemFile -Encoding ASCII

# 3. Mock ParticleSystem implementation
$mockParticleSystemImpl = Join-Path $mocksDir "ParticleSystem.cpp"
@"
// Mock implementation of ParticleSystem methods
#include "ParticleSystem.hpp"

namespace TurtleEngine {
    // Base ParticleSystem implementation
    ParticleSystem::ParticleSystem(size_t maxParticles) {
        std::cout << "[MockParticleSystem] Created with maxParticles: " << maxParticles << std::endl;
    }
    
    ParticleSystem::~ParticleSystem() {
        std::cout << "[MockParticleSystem] Destroyed" << std::endl;
    }
    
    void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity,
                             const glm::vec4& color, float size, float life) {
        std::cout << "[MockParticleSystem] Emitted particle at position: (" 
                 << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
    }
    
    void ParticleSystem::initialize() {
        std::cout << "[MockParticleSystem] Initialized" << std::endl;
    }
    
    void ParticleSystem::update(float deltaTime) {
        // No output to avoid spam
    }
    
    void ParticleSystem::render() {
        // No output to avoid spam
    }
    
    void ParticleSystem::spawnBurst(const glm::vec3& position, size_t count,
                                  float minVel, float maxVel,
                                  const glm::vec4& color, float size, float life) {
        std::cout << "[MockParticleSystem] Spawned burst of " << count << " particles" << std::endl;
    }
    
    void ParticleSystem::spawnParticle(const glm::vec3& position, const glm::vec3& velocity, 
                                     const glm::vec4& color, float size, float life) {
        std::cout << "[MockParticleSystem] Spawned single particle" << std::endl;
    }
    
    // Graphics namespace implementation
    namespace Graphics {
        ParticleSystem::ParticleSystem(size_t maxParticles) {
            std::cout << "[MockGraphicsParticleSystem] Created with maxParticles: " << maxParticles << std::endl;
        }
        
        ParticleSystem::~ParticleSystem() {
            std::cout << "[MockGraphicsParticleSystem] Destroyed" << std::endl;
        }
        
        void ParticleSystem::emit(const glm::vec3& position, const glm::vec3& velocity,
                                const glm::vec4& color, float size, float life) {
            std::cout << "[MockGraphicsParticleSystem] Emitted particle at position: (" 
                     << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        }
        
        void ParticleSystem::initialize() {
            std::cout << "[MockGraphicsParticleSystem] Initialized" << std::endl;
        }
        
        void ParticleSystem::update(float deltaTime) {
            // No output to avoid spam
        }
        
        void ParticleSystem::render() {
            // No output to avoid spam
        }
        
        void ParticleSystem::emitBurst(const glm::vec3& position, size_t count,
                                    float minVel, float maxVel,
                                    const glm::vec4& color, float size, float life) {
            std::cout << "[MockGraphicsParticleSystem] Emitted burst of " << count << " particles" << std::endl;
        }
    }
}
"@ | Out-File -FilePath $mockParticleSystemImpl -Encoding ASCII

# 4. Define GLM experimental flag for dual quaternion
$glmDefinesFile = Join-Path $mocksDir "glm_defines.hpp"
@"
// GLM experimental definitions
#pragma once
#ifndef GLM_DEFINES_HPP
#define GLM_DEFINES_HPP

#ifndef GLM_ENABLE_EXPERIMENTAL
#define GLM_ENABLE_EXPERIMENTAL
#endif

#endif // GLM_DEFINES_HPP
"@ | Out-File -FilePath $glmDefinesFile -Encoding ASCII

# Define the components to test
$components = @(
    @{
        Name = "HealthSystem";
        SourceFiles = @("src\tests\standalone\HealthSystemTest.cpp", "src\engine\combat\src\HealthSystem.cpp", "mocks\ParticleSystem.cpp");
        IncludePaths = @(".", ".\vcpkg\installed\x64-windows\include", ".\src", ".\src\engine", ".\src\engine\combat\include", ".\mocks");
        ExtraFlags = @("/D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING");
    },
    @{
        Name = "GestureRecognizer";
        SourceFiles = @("src\tests\standalone\GestureRecognizerStandaloneTest.cpp", "src\engine\input\src\GestureRecognizer.cpp");
        IncludePaths = @(".", ".\vcpkg\installed\x64-windows\include", ".\src", ".\src\engine", ".\src\engine\input\include", ".\mocks");
        ExtraFlags = @("/D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING");
    },
    @{
        Name = "PlasmaWeapon";
        SourceFiles = @("src\tests\standalone\PlasmaWeaponStandaloneTest.cpp", "src\engine\combat\src\PlasmaWeapon.cpp", "mocks\ParticleSystem.cpp");
        IncludePaths = @(".", ".\vcpkg\installed\x64-windows\include", ".\src", ".\src\engine", ".\src\engine\combat\include", ".\mocks");
        ExtraFlags = @("/D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING");
    },
    @{
        Name = "AIConstruct";
        SourceFiles = @("src\tests\standalone\AIConstructStandaloneTest.cpp", "src\engine\combat\src\AIConstruct.cpp", "mocks\ParticleSystem.cpp");
        IncludePaths = @(".", ".\vcpkg\installed\x64-windows\include", ".\src", ".\src\engine", ".\src\engine\combat\include", ".\mocks");
        ExtraFlags = @("/D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING", "/FI""mocks/ParticleSystem.hpp""");
    },
    @{
        Name = "TemporalAnomalySystem";
        SourceFiles = @("src\tests\standalone\TemporalAnomalySystemStandaloneTest.cpp", "src\engine\temporal\src\TemporalAnomalySystem.cpp");
        IncludePaths = @(".", ".\vcpkg\installed\x64-windows\include", ".\src", ".\src\engine", ".\src\engine\temporal\include", ".\mocks");
        ExtraFlags = @("/D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING", "/DGLM_ENABLE_EXPERIMENTAL", "/FI""mocks/glm_defines.hpp""");
    }
)

# Run tests and collect results
$results = @()

foreach ($component in $components) {
    Write-Host "`n----- Testing $($component.Name) -----"
    
    # Build include paths
    $includeDirs = $component.IncludePaths | ForEach-Object { "/I""$_""" }
    $includeDirsStr = $includeDirs -join " "
    
    # Build source files
    $sourceFilesStr = $component.SourceFiles -join " "
    
    # Build extra flags
    $extraFlagsStr = $component.ExtraFlags -join " "
    
    # Create a temporary batch file to compile and run the test
    $tempBatch = "temp_compile_$($component.Name.ToLower()).bat"
    $exeName = "$($component.Name)Test.exe"
    $resultFile = "test_results_$($component.Name.ToLower()).txt"
    
    @"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling $($component.Name) test...
cl /nologo /EHsc /std:c++17 $includeDirsStr $sourceFilesStr /Fe:$exeName $extraFlagsStr
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  $exeName
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

    Write-Host "Running compilation and test..."
    cmd /c $tempBatch > $resultFile 2>&1
    
    # Check the result
    $content = Get-Content $resultFile -Raw
    $success = $content -match "Compilation successful" -and $content -notmatch "Compilation failed"
    $passed = $content -match "PASSED" -and $content -notmatch "FAILED"
    
    $status = "FAILED"
    if ($success -and $passed) {
        $status = "PASSED"
    } elseif ($success) {
        $status = "COMPILED, TESTS FAILED"
    } else {
        $status = "COMPILATION FAILED"
    }
    
    $results += [PSCustomObject]@{
        Component = $component.Name
        Status = $status
    }
    
    Write-Host "$($component.Name) test completed with status: $status"
    
    # Clean up
    Remove-Item -Path $tempBatch -Force
}

# Display summary
Write-Host "`n==== Test Summary ====" -ForegroundColor Cyan
$results | ForEach-Object {
    $color = "Red"
    if ($_.Status -eq "PASSED") { $color = "Green" }
    elseif ($_.Status -eq "COMPILED, TESTS FAILED") { $color = "Yellow" }
    
    Write-Host "$($_.Component): " -NoNewline
    Write-Host "$($_.Status)" -ForegroundColor $color
}

# Count success/failures
$passed = ($results | Where-Object { $_.Status -eq "PASSED" }).Count
$total = $results.Count

Write-Host "`nPassed $passed of $total component tests" -ForegroundColor $(if ($passed -eq $total) { "Green" } else { "Yellow" })

# Save summary to file
$summaryPath = "phase1_test_summary.txt"
"==== Silent Forge: Phase 1 Component Test Summary ====`n" | Out-File $summaryPath
$results | ForEach-Object {
    "$($_.Component): $($_.Status)" | Out-File $summaryPath -Append
}
"`nPassed $passed of $total component tests." | Out-File $summaryPath -Append

Write-Host "`nTest summary saved to: $summaryPath" 