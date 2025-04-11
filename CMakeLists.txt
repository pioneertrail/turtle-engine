cmake_minimum_required(VERSION 3.16)  # Increased for better PCH support
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
option(SF_ENABLE_PCH "Use precompiled headers" ON)
option(SF_ENABLE_DEBUGGING "Enable debug visualizations" ON)
option(SF_USE_OPENCV "Build with OpenCV support" ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Configure version header
configure_file(src/Version.hpp.in ${CMAKE_BINARY_DIR}/generated/Version.hpp)
include_directories(${CMAKE_BINARY_DIR}/generated)

# Dependency management
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

# Find dependencies
find_package(glm CONFIG REQUIRED)

# OpenCV support
if(SF_USE_OPENCV)
    set(OpenCV_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/share/opencv" CACHE PATH "Path to OpenCV config")
    find_package(OpenCV QUIET)
    if(OpenCV_FOUND)
        add_definitions(-DUSE_OPENCV)
        message(STATUS "OpenCV found: ${OpenCV_VERSION}")
        message(STATUS "OpenCV include dirs: ${OpenCV_INCLUDE_DIRS}")
        message(STATUS "OpenCV libs: ${OpenCV_LIBS}")
    else()
        message(WARNING "OpenCV not found. GestureRecognizer will use simplified implementation.")
        set(SF_USE_OPENCV OFF)
    endif()
endif()

# Enable GLM experimental features
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Display found package details for debugging
message(STATUS "GLM include dirs: ${GLM_INCLUDE_DIRS}")
message(STATUS "")
message(STATUS "Silent Forge configuration summary:")
message(STATUS "  Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build tests:            ${SF_BUILD_TESTS}")
message(STATUS "  Use system libraries:   ${SF_USE_SYSTEM_LIBS}")
message(STATUS "  Precompiled headers:    ${SF_ENABLE_PCH}")
message(STATUS "  Debugging enabled:      ${SF_ENABLE_DEBUGGING}")
message(STATUS "  OpenCV support:         ${SF_USE_OPENCV}")
message(STATUS "")

# Include directories
include_directories(
    ${GLM_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/engine/include
    ${CMAKE_SOURCE_DIR}/src/engine/combat/include
    ${CMAKE_SOURCE_DIR}/src/engine/temporal/include
    ${CMAKE_SOURCE_DIR}/src/engine/input/include
    ${CMAKE_SOURCE_DIR}/src/pch
)

if(SF_USE_OPENCV AND OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Add debug flags if debug enabled
if(SF_ENABLE_DEBUGGING)
    add_definitions(-DDEBUG)
endif()

# Standalone temporal demos
add_executable(SimpleTemporalDemo 
    src/demos/simple_temporal_demo.cpp
)

target_link_libraries(SimpleTemporalDemo PRIVATE glm::glm)
set_target_properties(SimpleTemporalDemo PROPERTIES
    OUTPUT_NAME "SimpleTemporalDemo"
    FOLDER "Demos"
)

# Improved Temporal Demo
add_executable(ImprovedTemporalDemo 
    src/demos/improved_temporal_demo.cpp
)

target_link_libraries(ImprovedTemporalDemo PRIVATE glm::glm)
if(SF_USE_OPENCV AND OpenCV_FOUND)
    target_link_libraries(ImprovedTemporalDemo PRIVATE ${OpenCV_LIBS})
endif()

set_target_properties(ImprovedTemporalDemo PROPERTIES
    OUTPUT_NAME "ImprovedTemporalDemo"
    FOLDER "Demos"
)

if(MSVC)
    # Enable parallel builds for MSVC
    target_compile_options(SimpleTemporalDemo PRIVATE /MP)
    target_compile_options(ImprovedTemporalDemo PRIVATE /MP)
    # Disable warnings about secure functions
    target_compile_definitions(SimpleTemporalDemo PRIVATE _CRT_SECURE_NO_WARNINGS)
    target_compile_definitions(ImprovedTemporalDemo PRIVATE _CRT_SECURE_NO_WARNINGS)
    # Treat warnings as errors in Release builds
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        target_compile_options(SimpleTemporalDemo PRIVATE /WX)
        target_compile_options(ImprovedTemporalDemo PRIVATE /WX)
    endif()
endif()

# Installation rules
install(TARGETS SimpleTemporalDemo ImprovedTemporalDemo DESTINATION bin)

# Tests
if(SF_BUILD_TESTS)
    enable_testing()
    
    # PlasmaWeapon test
    add_executable(PlasmaWeaponTest 
        "src/tests/combat/PlasmaWeaponTest.cpp" 
        ${ENGINE_SOURCES} 
        ${COMBAT_SOURCES}
        ${PCH_SOURCES}
    )
    if(SF_ENABLE_PCH)
        target_precompile_headers(PlasmaWeaponTest PRIVATE src/pch/pch.hpp)
    endif()
    target_link_libraries(PlasmaWeaponTest PRIVATE glm::glm)
    add_test(NAME PlasmaWeaponTest COMMAND PlasmaWeaponTest)
    
    # AIConstruct test
    add_executable(AIConstructTest 
        "src/tests/combat/AIConstructTest.cpp" 
        ${ENGINE_SOURCES} 
        ${COMBAT_SOURCES}
        ${PCH_SOURCES}
    )
    if(SF_ENABLE_PCH)
        target_precompile_headers(AIConstructTest PRIVATE src/pch/pch.hpp)
    endif()
    target_link_libraries(AIConstructTest PRIVATE glm::glm)
    add_test(NAME AIConstructTest COMMAND AIConstructTest)
    
    # TemporalAnomalySystem test
    add_executable(TemporalAnomalyTest 
        "src/tests/TemporalAnomalyTest.cpp" 
        ${ENGINE_SOURCES} 
        ${TEMPORAL_SOURCES}
        ${PCH_SOURCES}
    )
    if(SF_ENABLE_PCH)
        target_precompile_headers(TemporalAnomalyTest PRIVATE src/pch/pch.hpp)
    endif()
    target_link_libraries(TemporalAnomalyTest PRIVATE glm::glm)
    add_test(NAME TemporalAnomalyTest COMMAND TemporalAnomalyTest)
    
    # TemporalCollisionHandler test (new)
    add_executable(TemporalCollisionTest 
        "src/tests/TemporalCollisionTest.cpp" 
        ${ENGINE_SOURCES} 
        ${TEMPORAL_SOURCES}
        ${PCH_SOURCES}
    )
    if(SF_ENABLE_PCH)
        target_precompile_headers(TemporalCollisionTest PRIVATE src/pch/pch.hpp)
    endif()
    target_link_libraries(TemporalCollisionTest PRIVATE glm::glm)
    add_test(NAME TemporalCollisionTest COMMAND TemporalCollisionTest)
    
    # HealthSystem test (new)
    add_executable(HealthSystemTest 
        "src/tests/combat/HealthSystemTest.cpp" 
        ${ENGINE_SOURCES} 
        ${COMBAT_SOURCES}
        ${PCH_SOURCES}
    )
    if(SF_ENABLE_PCH)
        target_precompile_headers(HealthSystemTest PRIVATE src/pch/pch.hpp)
    endif()
    target_link_libraries(HealthSystemTest PRIVATE glm::glm)
    add_test(NAME HealthSystemTest COMMAND HealthSystemTest)
    
    # GestureRecognizer test (new)
    add_executable(GestureRecognizerTest 
        "src/tests/input/GestureRecognizerTest.cpp" 
        ${ENGINE_SOURCES} 
        ${INPUT_SOURCES}
        ${PCH_SOURCES}
    )
    if(SF_ENABLE_PCH)
        target_precompile_headers(GestureRecognizerTest PRIVATE src/pch/pch.hpp)
    endif()
    target_link_libraries(GestureRecognizerTest PRIVATE glm::glm)
    if(SF_USE_OPENCV AND OpenCV_FOUND)
        target_link_libraries(GestureRecognizerTest PRIVATE ${OpenCV_LIBS})
    endif()
    add_test(NAME GestureRecognizerTest COMMAND GestureRecognizerTest)
endif()

# Installation rules
install(TARGETS SilentForgeDemo DESTINATION bin)
if(SF_BUILD_TESTS)
    install(TARGETS 
        PlasmaWeaponTest 
        AIConstructTest 
        TemporalAnomalyTest 
        TemporalCollisionTest
        HealthSystemTest 
        GestureRecognizerTest 
    DESTINATION bin/tests)
endif()

# Version information
configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Version.hpp.in
    ${CMAKE_CURRENT_BINARY_DIR}/include/Version.hpp
)

# Summary
message(STATUS "")
message(STATUS "Silent Forge configuration summary:")
message(STATUS "  Build type:             ${CMAKE_BUILD_TYPE}")
message(STATUS "  Build tests:            ${SF_BUILD_TESTS}")
message(STATUS "  Use system libraries:   ${SF_USE_SYSTEM_LIBS}")
message(STATUS "  Precompiled headers:    ${SF_ENABLE_PCH}")
message(STATUS "  Debugging enabled:      ${SF_ENABLE_DEBUGGING}")
message(STATUS "  OpenCV support:         ${SF_USE_OPENCV}")
message(STATUS "")

# Enhanced Temporal Demo Executable
add_executable(ImprovedTemporalDemo
    "src/demos/improved_temporal_demo.cpp"
    ${ENGINE_SOURCES}
    ${TEMPORAL_SOURCES}
)
if(SF_ENABLE_PCH)
    target_precompile_headers(ImprovedTemporalDemo PRIVATE src/pch/pch.hpp)
endif()
target_link_libraries(ImprovedTemporalDemo PRIVATE glm::glm)
set_target_properties(ImprovedTemporalDemo PROPERTIES
    OUTPUT_NAME "ImprovedTemporalDemo"
    FOLDER "Demos"
)
install(TARGETS ImprovedTemporalDemo DESTINATION bin) 
