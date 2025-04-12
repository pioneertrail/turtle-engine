cmake_minimum_required(VERSION 3.16)
project(SilentForge VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type configuration
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif()

# Configuration options
option(SF_BUILD_TESTS "Build tests" ON)
option(SF_USE_SYSTEM_LIBS "Use system-installed libraries instead of vcpkg" OFF)
option(SF_ENABLE_PCH "Use precompiled headers" ON) # Note: PCH seems disabled later, check pch.hpp existence
option(SF_ENABLE_DEBUGGING "Enable debug visualizations" ON)
option(SF_USE_OPENCV "Build with OpenCV support" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configure version header (if Version.hpp.in exists)
# configure_file(src/Version.hpp.in ${CMAKE_BINARY_DIR}/generated/Version.hpp)
# include_directories(${CMAKE_BINARY_DIR}/generated)

# --- Dependency Management ---
if(NOT SF_USE_SYSTEM_LIBS)
    # Explicitly set vcpkg toolchain file if it exists
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
        set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
            CACHE STRING "Vcpkg toolchain file" FORCE)
        # Force CMake to use the vcpkg GLM
        set(glm_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/share/glm"
            CACHE PATH "Path to GLM config" FORCE)
    else()
        message(WARNING "Vcpkg not found at expected location. System libraries will be used.")
        set(SF_USE_SYSTEM_LIBS ON)
    endif()
endif()

# --- Find Dependencies ---
find_package(glm CONFIG REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
# Use find_package for glad if installed via vcpkg (requires glad providing CMake config)
# Or include glad source directly if not using vcpkg for it. Assuming vcpkg for now.
find_package(glad REQUIRED) # Might need adjustment based on how glad is installed/provided

# OpenCV support
if(SF_USE_OPENCV)
    # Assuming vcpkg path setup works
    find_package(OpenCV QUIET COMPONENTS core imgproc videoio) # Specify components
    if(OpenCV_FOUND)
        add_definitions(-DUSE_OPENCV)
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
        message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
    else()
        message(WARNING "OpenCV not found. GestureRecognizer/Camera features will be disabled.")
        set(SF_USE_OPENCV OFF) # Update flag if not found
    endif()
endif()

# Enable GLM experimental features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# --- Include Directories ---
include_directories(
    ${CMAKE_SOURCE_DIR}/src # Top-level src
    # GLM includes are usually handled by target_link_libraries
    # ${GLM_INCLUDE_DIRS}
)

# Add specific include paths for engine modules
include_directories(
    src/engine/include
    src/engine/combat/include
    src/engine/temporal/include
    src/engine/input/include
    # src/pch # If using PCH
)

if(SF_USE_OPENCV AND OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# --- Compiler Flags ---
# Add debug flags if debug enabled
if(SF_ENABLE_DEBUGGING)
    add_definitions(-DDEBUG)
endif()

if(MSVC)
    # Common MSVC settings
    target_compile_options(SilentForge INTERFACE /W4 /EHsc /MP)
    # Disable warnings about secure functions
    target_compile_definitions(SilentForge INTERFACE _CRT_SECURE_NO_WARNINGS)
    # Treat warnings as errors in Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(SilentForge INTERFACE /WX)
    endif()
else()
    # Common GCC/Clang settings
    target_compile_options(SilentForge INTERFACE -Wall -Wextra -Wpedantic)
endif()


# --- Engine Library ---
set(ENGINE_SOURCES
    src/engine/src/Engine.cpp
    src/engine/src/Window.cpp
    src/engine/src/Renderer.cpp
    src/engine/src/InputManager.cpp
    # Add other core engine files here if necessary
    # src/engine/src/Shader.cpp # Example
    # src/engine/src/Scene.cpp  # Example
)

set(TEMPORAL_SOURCES
    src/engine/temporal/src/TemporalAnomalySystem.cpp
    src/engine/temporal/src/TemporalCollisionHandler.cpp
)

# Combine sources (or add more modules as needed)
set(ALL_ENGINE_SOURCES
    ${ENGINE_SOURCES}
    ${TEMPORAL_SOURCES}
    # Add other module sources (combat, input, etc.) if they should be part of the main library
)

add_library(TurtleEngine STATIC ${ALL_ENGINE_SOURCES})

# Link engine library dependencies
target_link_libraries(TurtleEngine PUBLIC
    glm::glm          # Use imported target from find_package(glm CONFIG)
    OpenGL::GL        # Use imported target from find_package(OpenGL)
    glfw              # Use target from find_package(glfw3)
    glad              # Use target from find_package(glad)
)

# Set engine library properties
set_target_properties(TurtleEngine PROPERTIES FOLDER "Engine")

# --- Main Application ---
add_executable(TurtleEngineApp src/main.cpp)

# Link application dependencies
target_link_libraries(TurtleEngineApp PRIVATE
    TurtleEngine      # Link against our engine library
    # Dependencies are inherited from TurtleEngine (PUBLIC), but list explicitly if needed
    # glm::glm
    # OpenGL::GL
    # glfw
    # glad
)
if(SF_USE_OPENCV AND OpenCV_FOUND)
    target_link_libraries(TurtleEngineApp PRIVATE ${OpenCV_LIBS})
endif()

# Set application properties
set_target_properties(TurtleEngineApp PROPERTIES
    OUTPUT_NAME "TurtleEngineApp"
    FOLDER "Applications"
)


# --- Demos ---

# Simple Temporal Demo (Console)
add_executable(SimpleTemporalDemo
    src/demos/simple_temporal_demo.cpp
    # Note: This demo doesn't use the engine library
)
target_link_libraries(SimpleTemporalDemo PRIVATE glm::glm)
set_target_properties(SimpleTemporalDemo PROPERTIES
    OUTPUT_NAME "SimpleTemporalDemo"
    FOLDER "Demos"
)

# Improved Temporal Demo (Console, uses some engine parts)
add_executable(ImprovedTemporalDemo
    src/demos/improved_temporal_demo.cpp
    src/engine/temporal/src/TemporalAnomalySystem.cpp    # Link source directly
    src/engine/temporal/src/TemporalCollisionHandler.cpp # Link source directly
    # Note: This demo doesn't use the main TurtleEngine library target
)
target_link_libraries(ImprovedTemporalDemo PRIVATE
    glm::glm
    # Does not link full engine, only specific temporal sources
)
if(SF_USE_OPENCV AND OpenCV_FOUND) # If demo needs OpenCV for some reason
    target_link_libraries(ImprovedTemporalDemo PRIVATE ${OpenCV_LIBS})
endif()
set_target_properties(ImprovedTemporalDemo PROPERTIES
    OUTPUT_NAME "ImprovedTemporalDemo"
    FOLDER "Demos"
)


# --- Temporal Gesture Blaster Demo ---
add_executable(TemporalGestureBlaster src/temporal_gesture_blaster.cpp)

target_link_libraries(TemporalGestureBlaster PRIVATE
    TurtleEngine # Link against our engine library
)
if(SF_USE_OPENCV AND OpenCV_FOUND)
    target_link_libraries(TemporalGestureBlaster PRIVATE ${OpenCV_LIBS})
endif()

set_target_properties(TemporalGestureBlaster PROPERTIES
    OUTPUT_NAME "TemporalGestureBlasterDemo"
    FOLDER "Demos"
)


# --- Tests ---
# (Tests remain commented out as before)
# if(SF_BUILD_TESTS)
# ...
# endif()


# --- Installation Rules ---
install(TARGETS TurtleEngineApp DESTINATION bin)
install(TARGETS SimpleTemporalDemo DESTINATION bin)
install(TARGETS ImprovedTemporalDemo DESTINATION bin)
install(TARGETS TemporalGestureBlaster DESTINATION bin)
# Add installation rules for TurtleEngine library if needed (e.g., for packaging)
# install(TARGETS TurtleEngine ARCHIVE DESTINATION lib)
# install(DIRECTORY src/engine/include/ DESTINATION include/TurtleEngine) # Example header install


# --- Final Summary Message ---
message(STATUS "")
message(STATUS "Silent Forge configuration summary:")
message(STATUS "  Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build tests:            ${SF_BUILD_TESTS}")
message(STATUS "  Use system libraries:   ${SF_USE_SYSTEM_LIBS}")
# message(STATUS "  Precompiled headers:    ${SF_ENABLE_PCH}") # PCH likely not working yet
message(STATUS "  Debugging enabled:      ${SF_ENABLE_DEBUGGING}")
message(STATUS "  OpenCV support:         ${SF_USE_OPENCV}")
message(STATUS "")
message(STATUS "Targets created:")
message(STATUS "  Engine Library:         TurtleEngine")
message(STATUS "  Main Application:       TurtleEngineApp")
message(STATUS "  Demos:                  SimpleTemporalDemo, ImprovedTemporalDemo")
message(STATUS "  New Demo:               TemporalGestureBlasterDemo")
# message(STATUS "  Tests:                  (Disabled)")
message(STATUS "")
