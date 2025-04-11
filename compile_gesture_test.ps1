# PowerShell script to compile GestureRecognizerStandaloneTest
Write-Host "Setting up MSVC environment..."

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

# Create a file with a mock OpenCV implementation
$mockOpenCVFile = "mock_opencv.hpp"
@"
// Mock implementation of OpenCV for testing
#pragma once
#ifndef OPENCV2_CORE_HPP
#define OPENCV2_CORE_HPP

namespace cv {
    class Mat {
    public:
        Mat() = default;
        Mat(int rows, int cols, int type) {}
        int rows = 0;
        int cols = 0;
        void create(int rows, int cols, int type) {}
        bool empty() const { return true; }
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

# Create a temporary batch file to set up the environment and compile
$tempBatch = "temp_compile_gesture.bat"
@"
@echo off
call "$vcvarsPath"
cd /d "%CD%"
echo Compiling GestureRecognizerStandaloneTest...
set "INCLUDE_DIRS=/I. /I.\vcpkg\installed\x64-windows\include /I.\src /I.\src\engine /I.\src\engine\input\include"
set "SOURCE_FILES=src\tests\standalone\GestureRecognizerStandaloneTest.cpp src\engine\input\src\GestureRecognizer.cpp"
cl /nologo /EHsc /std:c++17 %INCLUDE_DIRS% %SOURCE_FILES% /Fe:GestureTestStandalone.exe /D_SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
if %ERRORLEVEL% EQU 0 (
  echo Compilation successful. Running test...
  GestureTestStandalone.exe
) else (
  echo Compilation failed.
)
"@ | Out-File -FilePath $tempBatch -Encoding ASCII

# Run the temporary batch file
Write-Host "Running compilation..."
cmd /c $tempBatch > test_results_gesture.txt 2>&1

# Clean up
Remove-Item -Path $tempBatch -Force 
# Leave mock_opencv.hpp for later use 