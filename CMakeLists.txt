cmake_minimum_required(VERSION 3.10)
project(TurtleEngine VERSION 1.0.0)

# Required dependencies
find_package(glm CONFIG REQUIRED)
find_package(OpenCV REQUIRED)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set vcpkg toolchain file if it exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake")
    set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" 
        CACHE STRING "Vcpkg toolchain file")
endif()

# Check for dependencies
find_package(glm CONFIG QUIET)
find_package(OpenCV QUIET)

# If dependencies are not found, provide helpful message
if(NOT glm_FOUND OR NOT OpenCV_FOUND)
    message(STATUS "Some dependencies were not found. You may need to run setup_dependencies.ps1")
    message(STATUS "GLM Found: ${glm_FOUND}")
    message(STATUS "OpenCV Found: ${OpenCV_FOUND}")
    
    # Continue anyway for basic compilation
endif()

# Define source directories
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)
set(ENGINE_DIR ${SRC_DIR}/engine)
set(COMBAT_DIR ${ENGINE_DIR}/combat)
set(TEMPORAL_DIR ${ENGINE_DIR}/temporal)
set(INPUT_DIR ${ENGINE_DIR}/input)
set(TESTS_DIR ${SRC_DIR}/tests)
set(PHASE1_TESTS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/phase1_tests)
set(PCH_DIR ${SRC_DIR}/pch)
set(MOCKS_DIR ${CMAKE_CURRENT_SOURCE_DIR}/mocks)

# Define GLM_ENABLE_EXPERIMENTAL for all targets
add_definitions(-DGLM_ENABLE_EXPERIMENTAL)

# Include directories
include_directories(
    ${SRC_DIR}
    ${ENGINE_DIR}/include
    ${COMBAT_DIR}/include
    ${TEMPORAL_DIR}/include
    ${INPUT_DIR}/include
    ${PCH_DIR}
    ${MOCKS_DIR}
)

# Collect source files for engine core
file(GLOB_RECURSE ENGINE_SOURCES
    ${ENGINE_DIR}/src/*.cpp
    ${ENGINE_DIR}/src/*.c
)

# Collect source files for combat system
file(GLOB_RECURSE COMBAT_SOURCES
    ${COMBAT_DIR}/src/*.cpp
    ${COMBAT_DIR}/src/*.c
)

# Collect source files for temporal system
file(GLOB_RECURSE TEMPORAL_SOURCES
    ${TEMPORAL_DIR}/src/*.cpp
    ${TEMPORAL_DIR}/src/*.c
)

# Collect source files for input system
file(GLOB_RECURSE INPUT_SOURCES
    ${INPUT_DIR}/src/*.cpp
    ${INPUT_DIR}/src/*.c
)

# Add main executable if main.cpp exists
if(EXISTS "${SRC_DIR}/main.cpp")
    add_executable(TurtleEngine ${SRC_DIR}/main.cpp
        ${ENGINE_SOURCES}
        ${COMBAT_SOURCES}
        ${TEMPORAL_SOURCES}
        ${INPUT_SOURCES}
    )
    
    # Link libraries if found
    if(glm_FOUND)
        target_link_libraries(TurtleEngine PRIVATE glm::glm)
    endif()
    
    if(OpenCV_FOUND)
        target_link_libraries(TurtleEngine PRIVATE ${OpenCV_LIBS})
    endif()
endif()

# Build test executables
option(BUILD_TESTS "Build test executables" ON)
if(BUILD_TESTS)
    message(STATUS "Building test executables")
    # Check for test files in phase1_tests directory
    if(EXISTS "${PHASE1_TESTS_DIR}/PlasmaWeaponTest.cpp")
        add_executable(PlasmaWeaponTest ${PHASE1_TESTS_DIR}/PlasmaWeaponTest.cpp 
            ${ENGINE_SOURCES} ${COMBAT_SOURCES})
        target_include_directories(PlasmaWeaponTest PRIVATE ${PHASE1_TESTS_DIR})
        if(glm_FOUND AND OpenCV_FOUND)
            target_link_libraries(PlasmaWeaponTest PRIVATE glm::glm ${OpenCV_LIBS})
        endif()
    endif()
    
    if(EXISTS "${PHASE1_TESTS_DIR}/AIConstructTest.cpp")
        add_executable(AIConstructTest ${PHASE1_TESTS_DIR}/AIConstructTest.cpp 
            ${ENGINE_SOURCES} ${COMBAT_SOURCES})
        target_include_directories(AIConstructTest PRIVATE ${PHASE1_TESTS_DIR})
        if(glm_FOUND AND OpenCV_FOUND)
            target_link_libraries(AIConstructTest PRIVATE glm::glm ${OpenCV_LIBS})
        endif()
    endif()
    
    if(EXISTS "${PHASE1_TESTS_DIR}/TemporalAnomalyTest.cpp")
        add_executable(TemporalAnomalyTest ${PHASE1_TESTS_DIR}/TemporalAnomalyTest.cpp 
            ${ENGINE_SOURCES} ${COMBAT_SOURCES} ${TEMPORAL_SOURCES})
        target_include_directories(TemporalAnomalyTest PRIVATE ${PHASE1_TESTS_DIR})
        if(glm_FOUND AND OpenCV_FOUND)
            target_link_libraries(TemporalAnomalyTest PRIVATE glm::glm ${OpenCV_LIBS})
        endif()
    endif()
    
    if(EXISTS "${PHASE1_TESTS_DIR}/GestureRecognizerTest.cpp")
        add_executable(GestureRecognizerTest ${PHASE1_TESTS_DIR}/GestureRecognizerTest.cpp 
            ${ENGINE_SOURCES} ${INPUT_SOURCES})
        target_include_directories(GestureRecognizerTest PRIVATE ${PHASE1_TESTS_DIR})
        if(glm_FOUND AND OpenCV_FOUND)
            target_link_libraries(GestureRecognizerTest PRIVATE glm::glm ${OpenCV_LIBS})
        endif()
    endif()
    
    if(EXISTS "${PHASE1_TESTS_DIR}/HealthSystemTest.cpp")
        add_executable(HealthSystemTest ${PHASE1_TESTS_DIR}/HealthSystemTest.cpp 
            ${ENGINE_SOURCES} ${COMBAT_SOURCES})
        target_include_directories(HealthSystemTest PRIVATE ${PHASE1_TESTS_DIR})
        if(glm_FOUND AND OpenCV_FOUND)
            target_link_libraries(HealthSystemTest PRIVATE glm::glm ${OpenCV_LIBS})
        endif()
    endif()
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Copy resources if they exist
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/assets DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
endif()

# Create run script for Windows
if(WIN32)
    file(WRITE ${CMAKE_BINARY_DIR}/run_tests.bat
        "@echo off\n"
        "cd bin\n"
        "echo Running PlasmaWeaponTest...\n"
        "PlasmaWeaponTest.exe\n"
        "echo Running AIConstructTest...\n"
        "AIConstructTest.exe\n"
        "echo Running TemporalAnomalyTest...\n"
        "TemporalAnomalyTest.exe\n"
        "echo Running GestureRecognizerTest...\n"
        "GestureRecognizerTest.exe\n"
        "echo Running HealthSystemTest...\n"
        "HealthSystemTest.exe\n"
        "pause\n"
    )
endif()

# Print configuration summary
message(STATUS "CMake configuration for TurtleEngine:")
message(STATUS "  Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "  Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "  Build directory: ${CMAKE_BINARY_DIR}")
message(STATUS "  Tests enabled: ${BUILD_TESTS}")
message(STATUS "  Using mock files: ${MOCKS_DIR}")
message(STATUS "  Using precompiled header: ${PCH_DIR}")
if(glm_FOUND)
    message(STATUS "  GLM version: ${GLM_VERSION}")
endif()
if(OpenCV_FOUND)
    message(STATUS "  OpenCV version: ${OpenCV_VERSION}")
endif() 
