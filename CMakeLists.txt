cmake_minimum_required(VERSION 3.10)
project(TurtleEngine VERSION 1.0.0 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Set global output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Build type and optimization settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# OpenCV optimization options
option(WITH_OPENMP "Enable OpenMP support" ON)
option(ENABLE_FAST_MATH "Enable fast math optimizations" ON)
option(WITH_CUDA "Enable CUDA support if available" ON)
option(BUILD_TESTS "Build test suite" ON)

# Find required packages
find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED CONFIG) # Use CONFIG mode for glfw3 with vcpkg
find_package(glm REQUIRED CONFIG)   # Use CONFIG mode for glm with vcpkg
find_package(OpenCV REQUIRED)
if(NOT OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV not found. Please check your installation.")
endif()

# Add OpenCV bin directory to PATH for runtime
# No longer needed to set manually if vcpkg handles DLL deployment or they are found in system PATH
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Core engine library for CSL, rendering, and game logic
add_library(TurtleEngineCore
    src/engine/src/Engine.cpp
    src/engine/src/Shader.cpp
    src/engine/src/Grid.cpp
    src/engine/src/csl/GestureRecognizer.cpp
    src/engine/src/csl/CSLSystem.cpp
    src/engine/src/combat/Hitbox.cpp
    src/engine/src/combat/Combo.cpp
    src/engine/src/ParticleSystem.cpp
)

# Performance optimization flags for gesture recognition and plasma effects
# Apply only to Release configuration - AFTER add_library
if(MSVC)
    target_compile_options(TurtleEngineCore PRIVATE $<$<CONFIG:Release>:/O1 /arch:AVX2>)
else()
    target_compile_options(TurtleEngineCore PRIVATE $<$<CONFIG:Release>:-O3 -ffast-math -march=native>)
endif()

target_include_directories(TurtleEngineCore PUBLIC
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/engine/include
    # OpenCV includes are handled by find_package with vcpkg
    # ${OpenCV_INCLUDE_DIRS} # Removed
)

target_link_libraries(TurtleEngineCore PUBLIC
    OpenGL::GL
    GLEW::GLEW # This target name is usually correct for GLEW
    glfw      # glfw3 find_package (CONFIG mode) typically provides the 'glfw' target
    glm::glm  # glm find_package (CONFIG mode) often provides 'glm::glm'
    ${OpenCV_LIBS} # vcpkg should populate this variable via find_package
)

# Create main executable
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE 
    TurtleEngineCore 
    # GLEW::GLEW # Should be inherited from TurtleEngineCore (PUBLIC)
    # OpenGL::GL # Should be inherited from TurtleEngineCore (PUBLIC)
    # glfw      # Should be inherited from TurtleEngineCore (PUBLIC)
)

# Copy shader files to build directory
file(COPY ${CMAKE_SOURCE_DIR}/shaders DESTINATION ${CMAKE_BINARY_DIR}/bin) # Copy shaders to bin where executables are

# Set output directories *before* targets are defined if needed globally
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Keep this commented/removed if only tests go to bin

if(BUILD_TESTS)
    enable_testing()
    # Set output directory specifically for tests *before* add_executable
    # set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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
    
    # Add basic camera test
    add_executable(CameraTest src/tests/CameraTest.cpp)
    target_link_libraries(CameraTest PRIVATE ${OpenCV_LIBS}) # Use ${OpenCV_LIBS}
    # target_include_directories(CameraTest PRIVATE ${OpenCV_INCLUDE_DIRS}) # No longer needed

    # Add Hitbox test
    add_executable(HitboxTest src/tests/combat/HitboxTest.cpp)
    target_link_libraries(HitboxTest PRIVATE TurtleEngineCore glm::glm)
    target_include_directories(HitboxTest PRIVATE
        ${CMAKE_SOURCE_DIR}/src/engine/include
        ${CMAKE_SOURCE_DIR}/src/tests
    )

    # Add Combo test
    add_executable(ComboTest src/tests/combat/ComboTest.cpp)
    # ComboTest depends on the core engine library which now includes Combo.cpp implementation
    target_link_libraries(ComboTest PRIVATE TurtleEngineCore)
    target_include_directories(ComboTest PRIVATE
        ${CMAKE_SOURCE_DIR}/src/engine/include
        ${CMAKE_SOURCE_DIR}/src/tests
    )

    # Add DLL copy commands for HitboxTest if needed (assuming it doesnt need OpenCV DLLs here)
    # if(OpenCV_BIN_DIR AND EXISTS "${OpenCV_BIN_DIR}")
    #     file(GLOB OpenCV_DLLS "${OpenCV_BIN_DIR}/*.dll")
    #     foreach(DLL ${OpenCV_DLLS})
    #         if (TARGET HitboxTest)
    #             add_custom_command(TARGET HitboxTest POST_BUILD
    #                 COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #                 ${DLL}
    #                 $<TARGET_FILE_DIR:HitboxTest>)
    #         endif()
    #     endforeach()
    # endif()
endif()

# After all targets are defined, add the DLL copy commands
# This might need adjustment depending on how vcpkg handles DLLs
if(OpenCV_BIN_DIR AND EXISTS "${OpenCV_BIN_DIR}") # Check if vcpkg provided the bin dir
    file(GLOB OpenCV_DLLS "${OpenCV_BIN_DIR}/*.dll")
    foreach(DLL ${OpenCV_DLLS})
        if (TARGET TurtleEngine)
            add_custom_command(TARGET TurtleEngine POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:TurtleEngine>)
        endif()
        if (TARGET CSLTest)
            add_custom_command(TARGET CSLTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:CSLTest>)
        endif()
        if (TARGET GestureTest)
            add_custom_command(TARGET GestureTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:GestureTest>)
        endif()
        if (TARGET CameraTest)
            add_custom_command(TARGET CameraTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:CameraTest>)
        endif()
        if (TARGET HitboxTest)
            add_custom_command(TARGET HitboxTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:HitboxTest>)
        endif()
        if (TARGET ComboTest)
            add_custom_command(TARGET ComboTest POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${DLL}
                $<TARGET_FILE_DIR:ComboTest>)
        endif()
    endforeach()
endif()

# Set output directories for main targets (redundant if set globally)
# set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin) # Let's ensure this is set correctly outside the test block too
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib) 