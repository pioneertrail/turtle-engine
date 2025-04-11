cmake_minimum_required(VERSION 3.12)
project(SilentForgeStandaloneTests)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Make dependencies optional
find_package(OpenCV COMPONENTS core)
set(OPENCV_FOUND ${OpenCV_FOUND})

# Require GLM now
find_package(glm REQUIRED)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}  # For mock headers (might be needed for other tests)
)

if(OPENCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
endif()

# Always include GLM now
include_directories(${GLM_INCLUDE_DIRS})

# HealthSystem Test
add_executable(HealthSystemTest 
    HealthSystemTest.cpp
    ../../engine/combat/src/HealthSystem.cpp
)
# target_compile_definitions(HealthSystemTest PRIVATE STANDALONE_TEST) # Removed define

if(OPENCV_FOUND)
    # Link OpenCV if found (might still be needed by other tests)
    target_link_libraries(HealthSystemTest ${OpenCV_LIBS})
endif()


if(OPENCV_FOUND)
    # Other tests that require OpenCV and GLM
    
    # Plasma Weapon Test
    add_executable(PlasmaWeaponTest PlasmaWeaponStandaloneTest.cpp)
    target_link_libraries(PlasmaWeaponTest ${OpenCV_LIBS})

    # AIConstruct Test
    add_executable(AIConstructTest AIConstructStandaloneTest.cpp)
    target_link_libraries(AIConstructTest ${OpenCV_LIBS})

    # HealthComponent Test
    add_executable(HealthComponentTest HealthComponentStandaloneTest.cpp)
    target_link_libraries(HealthComponentTest ${OpenCV_LIBS})

    # Gesture Recognizer Test
    add_executable(GestureRecognizerTest GestureRecognizerStandaloneTest.cpp)
    target_link_libraries(GestureRecognizerTest ${OpenCV_LIBS})

    # Temporal Anomaly Test
    add_executable(TemporalAnomalyTest TemporalAnomalySystemStandaloneTest.cpp)
    target_link_libraries(TemporalAnomalyTest ${OpenCV_LIBS})
endif() 