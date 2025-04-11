cmake_minimum_required(VERSION 3.10)
project(SilentForgeDemo)

set(CMAKE_CXX_STANDARD 17)

# Explicitly set vcpkg toolchain file
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Force CMake to use the vcpkg GLM
set(glm_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/installed/x64-windows/share/glm" CACHE PATH "Path to GLM config" FORCE)

# Find dependencies
find_package(glm CONFIG REQUIRED)

# Display found package details for debugging
message(STATUS "GLM include dirs: ${GLM_INCLUDE_DIRS}")

# Include directories
include_directories(
    ${GLM_INCLUDE_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Create the main demo executable
add_executable(SilentForgeDemo src/console_demo.cpp)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Installation rules
install(TARGETS SilentForgeDemo DESTINATION bin) 