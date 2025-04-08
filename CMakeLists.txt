cmake_minimum_required(VERSION 3.10)
project(TurtleEngine)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Set GLEW paths
set(GLEW_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/deps/glew/include)
set(GLEW_LIBRARY ${CMAKE_SOURCE_DIR}/deps/glew/lib/glew32.lib)
set(GLEW_DLL ${CMAKE_SOURCE_DIR}/deps/glew/bin/glew32.dll)

# Create GLEW imported target
add_library(GLEW::GLEW STATIC IMPORTED)
set_target_properties(GLEW::GLEW PROPERTIES
    IMPORTED_LOCATION "${GLEW_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLEW_INCLUDE_DIR}"
)

# Build GLFW from source
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(deps/glfw)

# Set GLM paths
set(GLM_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/deps/glm)

# Create GLM imported target
add_library(glm::glm INTERFACE IMPORTED)
set_target_properties(glm::glm PROPERTIES
    INTERFACE_INCLUDE_DIRECTORIES "${GLM_INCLUDE_DIR}"
)

# Find required packages
find_package(OpenGL REQUIRED)

# Configure GTest
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(deps/googletest)

# Engine library
add_library(TurtleEngine STATIC
    src/engine/src/Renderer.cpp
    src/engine/src/Window.cpp
    src/engine/src/InputManager.cpp
    src/engine/src/Engine.cpp
)

target_include_directories(TurtleEngine
    PUBLIC
        ${CMAKE_SOURCE_DIR}/src/engine/include
    PRIVATE
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
)

target_link_libraries(TurtleEngine
    PUBLIC
        OpenGL::GL
        GLEW::GLEW
        glfw
        glm::glm
)

# Main executable
add_executable(TurtleEngineApp src/main.cpp)
target_link_libraries(TurtleEngineApp PRIVATE TurtleEngine)

# Tests
enable_testing()

add_executable(RenderTests src/tests/src/RenderTests.cpp)
target_include_directories(RenderTests
    PRIVATE
        ${CMAKE_SOURCE_DIR}/src/tests/include
        ${CMAKE_SOURCE_DIR}/src/engine/include
        ${OPENGL_INCLUDE_DIR}
        ${GLEW_INCLUDE_DIR}
        ${GLM_INCLUDE_DIR}
)

target_link_libraries(RenderTests
    PRIVATE
        TurtleEngine
        gtest_main
)

# Copy shader files to test binary directory
add_custom_command(TARGET RenderTests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    "${CMAKE_SOURCE_DIR}/shaders"
    "$<TARGET_FILE_DIR:RenderTests>/shaders"
    COMMENT "Copying shaders to test binary directory"
)

# Copy GLEW DLL to test output directory on Windows
if(WIN32)
    add_custom_command(TARGET RenderTests POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${GLEW_DLL}"
        "$<TARGET_FILE_DIR:RenderTests>/glew32.dll"
        COMMENT "Copying GLEW DLL for Windows runtime"
    )
endif()

# Add test to CTest with working directory set to test binary location
add_test(NAME RenderTests COMMAND RenderTests WORKING_DIRECTORY $<TARGET_FILE_DIR:RenderTests>) 