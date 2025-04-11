cmake_minimum_required(VERSION 3.10)
project(SilentForgeDemo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake" CACHE STRING "Vcpkg toolchain file")

# Required dependencies
find_package(glm CONFIG REQUIRED)
find_package(OpenCV REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/combat/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/temporal/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src/engine/input/include)

# Collect source files
file(GLOB CORE_SOURCES 
    "src/main.cpp"
    "src/engine/src/*.cpp"
)

file(GLOB COMBAT_SOURCES 
    "src/engine/combat/src/*.cpp"
)

file(GLOB TEMPORAL_SOURCES 
    "src/engine/temporal/src/*.cpp"
)

file(GLOB INPUT_SOURCES 
    "src/engine/input/src/*.cpp"
)

# Create the executable
add_executable(SilentForgeDemo 
    ${CORE_SOURCES}
    ${COMBAT_SOURCES}
    ${TEMPORAL_SOURCES}
    ${INPUT_SOURCES}
)

# Link libraries
target_link_libraries(SilentForgeDemo PRIVATE glm::glm ${OpenCV_LIBS})

# Add test executable if testing is enabled
option(BUILD_TESTS "Build test executables" ON)
if(BUILD_TESTS)
    # Create test executable for PlasmaWeapon
    add_executable(PlasmaWeaponTest "phase1_tests/PlasmaWeaponTest.cpp")
    target_link_libraries(PlasmaWeaponTest PRIVATE glm::glm ${OpenCV_LIBS})
    
    # Create test executable for TemporalAnomalySystem
    add_executable(TemporalAnomalyTest "phase1_tests/TemporalAnomalyTest.cpp")
    target_link_libraries(TemporalAnomalyTest PRIVATE glm::glm ${OpenCV_LIBS})
    
    # Create test executable for AIConstruct
    add_executable(AIConstructTest "phase1_tests/AIConstructTest.cpp")
    target_link_libraries(AIConstructTest PRIVATE glm::glm ${OpenCV_LIBS})
    
    # Create test executable for HealthSystem
    add_executable(HealthSystemTest "phase1_tests/HealthSystemTest.cpp")
    target_link_libraries(HealthSystemTest PRIVATE glm::glm ${OpenCV_LIBS})
    
    # Create test executable for GestureRecognizer
    add_executable(GestureRecognizerTest "phase1_tests/GestureRecognizerTest.cpp")
    target_link_libraries(GestureRecognizerTest PRIVATE glm::glm ${OpenCV_LIBS})
endif()

# Set output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# Copy resources if needed
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/shaders DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})

# Installation rules
install(TARGETS SilentForgeDemo DESTINATION bin)
if(BUILD_TESTS)
    install(TARGETS PlasmaWeaponTest TemporalAnomalyTest AIConstructTest HealthSystemTest GestureRecognizerTest DESTINATION bin/tests)
endif() 