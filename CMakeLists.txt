cmake_minimum_required(VERSION 3.10)
project(TurtleEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type and optimization settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Performance optimization flags for gesture recognition and plasma effects
if(MSVC)
    add_compile_options(/O2 /fp:fast /arch:AVX2)
else()
    add_compile_options(-O3 -ffast-math -march=native)
endif()

# OpenCV optimization options
option(WITH_OPENMP "Enable OpenMP support" ON)
option(ENABLE_FAST_MATH "Enable fast math optimizations" ON)
option(WITH_CUDA "Enable CUDA support if available" ON)
option(BUILD_TESTS "Build test suite" OFF)

# Set OpenCV_DIR to help find the local installation with fallback
set(OpenCV_DIR "C:/Users/HydraPony/dev/opencv/build")
find_package(OpenCV REQUIRED PATHS ${OpenCV_DIR} NO_DEFAULT_PATH)
if(NOT OpenCV_FOUND)
    message(STATUS "Local OpenCV not found, searching system paths")
    find_package(OpenCV REQUIRED)
endif()

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(glm CONFIG REQUIRED)

# Core engine library for CSL, rendering, and game logic
add_library(TurtleEngineCore
    src/engine/src/Engine.cpp
    src/engine/src/Shader.cpp
    src/engine/src/Grid.cpp
    src/engine/src/csl/GestureRecognizer.cpp
    src/engine/src/csl/CSLSystem.cpp
)

target_include_directories(TurtleEngineCore PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/engine/include
    ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(TurtleEngineCore PUBLIC
    OpenGL::GL
    GLEW::GLEW
    glfw
    glm::glm-header-only
    ${OpenCV_LIBS}
)

# Create main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE TurtleEngineCore)

# Copy shader files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR})

if(BUILD_TESTS)
    enable_testing()
    
    # Add CSL Animation System test with optimizations
    add_executable(CSLTest src/tests/CSLTest.cpp)
    target_link_libraries(CSLTest PRIVATE TurtleEngineCore)
    target_include_directories(CSLTest PRIVATE 
        ${CMAKE_SOURCE_DIR}/src/engine/include
        ${CMAKE_SOURCE_DIR}/src/tests
    )
    
    # Add Gesture Recognition test with performance monitoring
    add_executable(GestureTest 
        src/tests/GestureTestMain.cpp 
        src/tests/GestureTest.cpp
    )
    target_link_libraries(GestureTest PRIVATE TurtleEngineCore)
    target_include_directories(GestureTest PRIVATE 
        ${CMAKE_SOURCE_DIR}/src/engine/include 
        ${CMAKE_SOURCE_DIR}/src/tests
    )
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) 