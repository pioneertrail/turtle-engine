# Silent Forge: Dependencies Documentation

This document outlines all the external dependencies required for the Silent Forge project, along with setup instructions.

## Core Dependencies

### 1. OpenCV (Required for GestureRecognizer)
- **Version**: 4.5.0 or higher
- **Purpose**: Used for computer vision and gesture recognition
- **Components Used**:
  - Core module
  - ImgProc module
  - Video module
- **Setup**:
  - Installed via vcpkg: `vcpkg install opencv4:x64-windows`
  - CMake Find Package: `find_package(OpenCV REQUIRED)`

### 2. GLM (Required throughout codebase)
- **Version**: 0.9.9 or higher
- **Purpose**: Vector mathematics library used for 3D calculations
- **Components Used**:
  - Vector types (vec2, vec3, vec4)
  - Matrix operations
  - Quaternions (for rotations)
  - Experimental features (string_cast, etc.)
- **Setup**:
  - Installed via vcpkg: `vcpkg install glm:x64-windows`
  - CMake configuration: 
    ```cmake
    find_package(glm CONFIG REQUIRED)
    add_definitions(-DGLM_ENABLE_EXPERIMENTAL) # Required for string_cast and other experimental features
    ```
- **Compiler Requirements**:
  - Requires C++17 compatible compiler
  - MSVC 2019 or newer recommended (MinGW has compatibility issues with GLM)

### 3. CMake
- **Version**: 3.20 or higher
- **Purpose**: Build system
- **Setup**: 
  - Download from [cmake.org](https://cmake.org/download/)
  - Add to PATH during installation

## Compiler Compatibility

### Recommended: MSVC (Visual Studio)
- **Version**: Visual Studio 2019 or higher with C++17 support
- **Why MSVC**: Ensures compatibility with GLM and modern C++ features
- **Configuration**: 
  ```cmake
  # CMake configuration for MSVC
  if(MSVC)
    add_compile_options(/W4 /EHsc)
    # Enable parallel builds
    add_compile_options(/MP)
  endif()
  ```

### Not Recommended: MinGW
- **Issues**: MinGW 6.x has compatibility issues with GLM and C++17 features
- **If you must use MinGW**: 
  - Use MinGW 8.0+ with POSIX threading model (not Win32)
  - Install from [MSYS2](https://www.msys2.org/) for better C++17 support
  - Add these compiler flags:
    ```cmake
    if(MINGW)
      add_compile_options(-std=c++17 -Wa,-mbig-obj)
    endif()
    ```

## Development Environment

### Windows
- **Visual Studio**: 2019 or higher with C++20 support
- **vcpkg**: Package manager for C++ dependencies
  - Setup: 
    ```
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    bootstrap-vcpkg.bat
    vcpkg integrate install
    ```

### Setup Script
The `setup_dependencies.ps1` PowerShell script automates dependency installation:
```powershell
# Run from project root
./setup_dependencies.ps1
```

## Configuration Options

### CMake Configuration
The project uses CMake with the following options:
- `SF_BUILD_TESTS`: Enable/disable building tests (default: ON)
- `SF_USE_SYSTEM_LIBS`: Use system-installed libraries instead of vcpkg (default: OFF)
- `SF_ENABLE_DEBUGGING`: Enable debug visualizations (default: ON in Debug builds)

Example CMake configuration:
```bash
cmake -B build -S . -DSF_BUILD_TESTS=ON -DCMAKE_TOOLCHAIN_FILE=./vcpkg/scripts/buildsystems/vcpkg.cmake
```

## Troubleshooting

### GLM Compatibility Issues
- **C++ Standard Library Version Mismatch**: 
  - Ensure you're using a compatible compiler with full C++17 support
  - Add `-DGLM_ENABLE_EXPERIMENTAL` for experimental GLM features
  - Use vcpkg's GLM over system-installed version to avoid inconsistencies

- **Tuple/Vector Errors**:
  - These typically indicate compiler incompatibility with GLM
  - Solution: Switch to MSVC (Visual Studio) compiler or newer MinGW (8.0+)

- **Build Script Failures**:
  - When using scripts like `build-and-run-temporal-test.ps1`, ensure they use MSVC:
    ```powershell
    # Use cl instead of g++
    cl /std:c++17 /EHsc /I"vcpkg/installed/x64-windows/include" /Fe:test.exe test.cpp
    ```

### Common OpenCV Issues
- **DLL Not Found**: Ensure OpenCV DLLs are in the system PATH or copied to the executable directory
- **Version Mismatch**: Check that the correct version is being linked (verify with `opencv_version`)

### vcpkg Maintenance
- **Clean Installation**:
  ```powershell
  # Remove and reinstall a package
  .\vcpkg\vcpkg.exe remove glm:x64-windows --recurse
  .\vcpkg\vcpkg.exe install glm:x64-windows
  ```

- **Upgrade All Packages**:
  ```powershell
  .\vcpkg\vcpkg.exe upgrade --no-dry-run
  ```

- **Troubleshooting vcpkg**:
  - If vcpkg has build artifacts in root folder, clean and rebuild:
    ```powershell
    cd vcpkg
    git clean -xfd
    .\bootstrap-vcpkg.bat
    .\vcpkg integrate install
    ```

## Component-Specific Dependencies

### GestureRecognizer
- Requires OpenCV with imgproc module

### TemporalAnomalySystem & PlasmaWeapon
- Require GLM for vector calculations and transformations
- TemporalAnomalySystem uses `glm::to_string()` which requires GLM_ENABLE_EXPERIMENTAL

### Rendering Components
- May require additional libraries in Phase 2 (likely DirectX or OpenGL) 