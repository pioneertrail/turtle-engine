# PowerShell script to set up dependencies for Silent Forge Phase 1 Demo
Write-Host "==== Silent Forge: Dependencies Setup ===="

# Function to check if vcpkg is installed
function Check-VcpkgInstallation {
    $vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
    
    if (Test-Path $vcpkgPath) {
        Write-Host "vcpkg found at: $vcpkgPath"
        return $vcpkgPath
    } else {
        Write-Host "vcpkg not found in the project directory."
        return $null
    }
}

# Function to install vcpkg if needed
function Install-Vcpkg {
    $vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
    
    Write-Host "Installing vcpkg..."
    git clone https://github.com/microsoft/vcpkg.git
    
    # Run the bootstrap script
    Set-Location vcpkg
    .\bootstrap-vcpkg.bat
    Set-Location ..
    
    Write-Host "vcpkg installed successfully at: $vcpkgPath"
    return $vcpkgPath
}

# Function to install dependencies using vcpkg
function Install-Dependencies {
    param (
        [string]$VcpkgPath
    )
    
    Write-Host "Installing required dependencies..."
    
    # Install GLM
    Write-Host "Installing GLM..."
    & "$VcpkgPath\vcpkg" install glm:x64-windows
    
    # Install OpenCV
    Write-Host "Installing OpenCV..."
    & "$VcpkgPath\vcpkg" install opencv:x64-windows
    
    # Integrate with Visual Studio
    Write-Host "Integrating vcpkg with Visual Studio..."
    & "$VcpkgPath\vcpkg" integrate install
    
    Write-Host "Dependencies installed successfully."
}

# Function to update CMakeLists.txt
function Update-CMakeLists {
    $cmakeListsPath = Join-Path $PSScriptRoot "CMakeLists.txt"
    
    if (Test-Path $cmakeListsPath) {
        Write-Host "Updating CMakeLists.txt to include dependencies..."
        
        $cmakeContent = Get-Content $cmakeListsPath
        
        # Check if the toolchain file is already set
        $toolchainLine = 'set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")'
        if ($cmakeContent -notcontains $toolchainLine) {
            # Add the toolchain file at the beginning
            $newContent = @($toolchainLine)
            $newContent += $cmakeContent
            Set-Content -Path $cmakeListsPath -Value $newContent
        }
        
        # Check if the find_package commands are already there
        $findGlm = 'find_package(glm CONFIG REQUIRED)'
        $findOpenCV = 'find_package(OpenCV REQUIRED)'
        
        $updatedContent = @()
        $glmAdded = $false
        $openCVAdded = $false
        
        foreach ($line in $cmakeContent) {
            $updatedContent += $line
            
            # Add find_package commands after the project() line
            if ($line -match "^project\(") {
                if ($cmakeContent -notcontains $findGlm) {
                    $updatedContent += ""
                    $updatedContent += "# Required dependencies"
                    $updatedContent += $findGlm
                    $glmAdded = $true
                }
                
                if ($cmakeContent -notcontains $findOpenCV) {
                    if (-not $glmAdded) {
                        $updatedContent += ""
                        $updatedContent += "# Required dependencies"
                    }
                    $updatedContent += $findOpenCV
                    $openCVAdded = $true
                }
                
                if ($glmAdded -or $openCVAdded) {
                    $updatedContent += ""
                }
            }
        }
        
        if ($glmAdded -or $openCVAdded) {
            Set-Content -Path $cmakeListsPath -Value $updatedContent
            Write-Host "CMakeLists.txt updated successfully."
        } else {
            Write-Host "CMakeLists.txt already contains dependency includes."
        }
    } else {
        Write-Host "CMakeLists.txt not found in the project directory."
    }
}

# Main script execution
Write-Host "Checking for vcpkg installation..."
$vcpkgPath = Check-VcpkgInstallation

if (-not $vcpkgPath) {
    $installVcpkg = Read-Host "vcpkg not found. Would you like to install it? (Y/N)"
    if ($installVcpkg -eq "Y" -or $installVcpkg -eq "y") {
        $vcpkgPath = Install-Vcpkg
    } else {
        Write-Host "vcpkg is required to install the dependencies. Exiting..."
        exit
    }
}

$installDeps = Read-Host "Would you like to install GLM and OpenCV dependencies? (Y/N)"
if ($installDeps -eq "Y" -or $installDeps -eq "y") {
    Install-Dependencies -VcpkgPath $vcpkgPath
} else {
    Write-Host "Skipping dependency installation."
}

$updateCMake = Read-Host "Would you like to update CMakeLists.txt to include the dependencies? (Y/N)"
if ($updateCMake -eq "Y" -or $updateCMake -eq "y") {
    Update-CMakeLists
} else {
    Write-Host "Skipping CMakeLists.txt update."
}

Write-Host "`n==== Dependency Setup Complete ===="
Write-Host "Next steps:"
Write-Host "1. Run 'cmake -B build -S . -G \"Visual Studio 17 2022\" -A x64' to generate project files"
Write-Host "2. Run 'cmake --build build --config Release' to build the project"
Write-Host "3. Once built, run the Phase 1 Demo with 'build\Release\SilentForge.exe'"

# Setup dependencies for TurtleEngine
Write-Host "Setting up dependencies for TurtleEngine..." -ForegroundColor Green

# Check if vcpkg is initialized
$vcpkgPath = Join-Path $PSScriptRoot "vcpkg"
if (-not (Test-Path (Join-Path $vcpkgPath "vcpkg.exe"))) {
    Write-Host "Initializing vcpkg..." -ForegroundColor Yellow
    
    # Clone vcpkg if not present
    if (-not (Test-Path $vcpkgPath)) {
        git clone https://github.com/microsoft/vcpkg.git
    }
    
    # Bootstrap vcpkg
    Push-Location $vcpkgPath
    .\bootstrap-vcpkg.bat
    Pop-Location
}

# Install required dependencies
Write-Host "Installing dependencies with vcpkg..." -ForegroundColor Yellow
Push-Location $vcpkgPath
./vcpkg install glm:x64-windows
./vcpkg install opencv:x64-windows
Pop-Location

# Create mock OpenCV header if needed
$mockOpenCVDir = Join-Path $PSScriptRoot "mocks"
$mockOpenCVHeader = Join-Path $mockOpenCVDir "opencv2/opencv.hpp"

if (-not (Test-Path $mockOpenCVHeader)) {
    Write-Host "Creating mock OpenCV header..." -ForegroundColor Yellow
    
    # Create directory
    New-Item -ItemType Directory -Path (Join-Path $mockOpenCVDir "opencv2") -Force
    
    # Create mock header
    @"
// Mock OpenCV header for building without actual OpenCV
#pragma once

#include <vector>
#include <string>
#include <memory>

namespace cv {
    class Mat {
    public:
        Mat() {}
        Mat(int rows, int cols, int type) {}
        Mat(const Mat& other) {}
        
        int rows = 0;
        int cols = 0;
    };
    
    enum ImreadModes {
        IMREAD_COLOR = 1,
        IMREAD_GRAYSCALE = 0
    };
    
    Mat imread(const std::string& filename, int mode = IMREAD_COLOR) {
        return Mat();
    }
    
    void imshow(const std::string& winname, const Mat& mat) {}
    
    int waitKey(int delay = 0) { return 0; }
    
    template<typename T>
    class Point_ {
    public:
        Point_() : x(0), y(0) {}
        Point_(T x, T y) : x(x), y(y) {}
        
        T x, y;
    };
    
    typedef Point_<int> Point;
    
    template<typename T>
    class Rect_ {
    public:
        Rect_() : x(0), y(0), width(0), height(0) {}
        Rect_(T x, T y, T width, T height) : x(x), y(y), width(width), height(height) {}
        
        T x, y, width, height;
    };
    
    typedef Rect_<int> Rect;
}
"@ | Out-File -FilePath $mockOpenCVHeader -Encoding utf8
}

# Create or update precompiled header for GLM
$pchDir = Join-Path $PSScriptRoot "src/pch"
$pchHeader = Join-Path $pchDir "pch.h"

if (-not (Test-Path $pchHeader)) {
    Write-Host "Creating precompiled header for GLM experimental features..." -ForegroundColor Yellow
    
    # Create directory
    New-Item -ItemType Directory -Path $pchDir -Force
    
    # Create PCH header
    @"
// Precompiled header for TurtleEngine
#pragma once

// Enable GLM experimental features
#define GLM_ENABLE_EXPERIMENTAL

// Include GLM headers
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/dual_quaternion.hpp>
#include <glm/gtx/compatibility.hpp>

// Standard library headers
#include <memory>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>
"@ | Out-File -FilePath $pchHeader -Encoding utf8
}

# Create dummy include files for any missing headers
$missingHeaders = @(
    "src/engine/combat/include/Combo.hpp",
    "src/engine/combat/include/Hitbox.hpp"
)

foreach ($header in $missingHeaders) {
    $headerPath = Join-Path $PSScriptRoot $header
    if (-not (Test-Path $headerPath)) {
        Write-Host "Creating missing header: $header" -ForegroundColor Yellow
        
        # Create directory if needed
        New-Item -ItemType Directory -Path (Split-Path $headerPath) -Force
        
        # Create basic header file
        $headerName = (Split-Path $headerPath -Leaf).Replace(".hpp", "")
        $namespace = "TurtleEngine::Combat"
        
        @"
// Auto-generated header for $headerName
#pragma once

namespace $namespace {
    class $headerName {
    public:
        $headerName() = default;
        ~$headerName() = default;
        
        void initialize() {}
        void update(float deltaTime) {}
    };
}
"@ | Out-File -FilePath $headerPath -Encoding utf8
    }
}

# Fix namespace issues in source files
$fixNamespaceFile = Join-Path $PSScriptRoot "src/engine/combat/include/HealthSystem.hpp" 
if (Test-Path $fixNamespaceFile) {
    Write-Host "Fixing namespace issues in HealthSystem.hpp..." -ForegroundColor Yellow
    
    $content = Get-Content $fixNamespaceFile -Raw
    
    # Add using directive if not exists
    if (-not ($content -match "using Graphics::ParticleSystem")) {
        $content = $content -replace "namespace TurtleEngine {", "namespace TurtleEngine {`r`n// Bridge namespace for ParticleSystem`r`nnamespace Graphics { class ParticleSystem; }`r`nusing Graphics::ParticleSystem;"
        $content | Out-File -FilePath $fixNamespaceFile -Encoding utf8
    }
}

# Update CMakeLists.txt to include mock directory in include paths
$cmakeFile = Join-Path $PSScriptRoot "CMakeLists.txt"
$cmakeContent = Get-Content $cmakeFile -Raw

if (-not ($cmakeContent -match "mocks")) {
    Write-Host "Updating CMakeLists.txt to include mock directories..." -ForegroundColor Yellow
    
    $updatedContent = $cmakeContent -replace "include_directories\(\s+(.+?)\s+\)", "include_directories(`r`n    `$1,`r`n    `${CMAKE_CURRENT_SOURCE_DIR}/mocks`r`n    `${CMAKE_CURRENT_SOURCE_DIR}/src/pch`r`n)"
    $updatedContent | Out-File -FilePath $cmakeFile -Encoding utf8
}

# Create header file with GLM_ENABLE_EXPERIMENTAL definition
$glmHeaderFile = Join-Path $PSScriptRoot "src/engine/include/GlmConfig.hpp"
if (-not (Test-Path $glmHeaderFile)) {
    Write-Host "Creating GLM configuration header..." -ForegroundColor Yellow
    
    New-Item -ItemType Directory -Path (Split-Path $glmHeaderFile) -Force
    
    @"
// GLM configuration to enable experimental features
#pragma once

#define GLM_ENABLE_EXPERIMENTAL
"@ | Out-File -FilePath $glmHeaderFile -Encoding utf8
}

Write-Host "Setup complete! You can now build the project with CMake." -ForegroundColor Green
Write-Host "Run the following commands:" -ForegroundColor Cyan
Write-Host "  cmake -B build -DCMAKE_BUILD_TYPE=Debug" -ForegroundColor White
Write-Host "  cmake --build build --config Debug" -ForegroundColor White 