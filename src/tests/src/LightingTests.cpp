#include <gtest/gtest.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "../engine/Renderer.hpp"
#include "../engine/Shader.h"

class LightingTest : public ::testing::Test {
protected:
    GLFWwindow* window;
    std::unique_ptr<Renderer> renderer;

    void SetUp() override {
        // Initialize GLFW and GLEW for testing
        if (!glfwInit()) {
            FAIL() << "Failed to initialize GLFW";
            return;
        }
        
        // Create a hidden window for OpenGL context
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
        window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
        if (!window) {
            FAIL() << "Failed to create GLFW window";
            glfwTerminate();
            return;
        }
        
        glfwMakeContextCurrent(window);
        
        // Initialize GLEW
        glewExperimental = GL_TRUE;
        if (glewInit() != GLEW_OK) {
            FAIL() << "Failed to initialize GLEW";
            return;
        }
        
        renderer = std::make_unique<Renderer>();
        renderer->init();
    }
    
    void TearDown() override {
        renderer->cleanup();
        renderer.reset();
        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

// Test light management
TEST_F(LightingTest, LightManagement) {
    // Create test lights
    Light light1(
        glm::vec3(1.0f, -1.0f, -1.0f), // position
        glm::vec3(1.0f),                // color
        1.0f,                           // intensity
        10.0f                           // radius
    );
    
    Light light2(
        glm::vec3(0.0f, 5.0f, 0.0f),   // position
        glm::vec3(1.0f, 0.0f, 0.0f),   // color
        1.0f,                           // intensity
        5.0f                            // radius
    );
    
    // Add lights
    renderer->addLight(light1);
    renderer->addLight(light2);
    
    // Update a light
    light2.color = glm::vec3(0.0f, 1.0f, 0.0f);
    renderer->updateLight(1, light2);
    
    // Remove a light
    renderer->removeLight(0);
    
    // Clear all lights
    renderer->clearLights();
}

// Test shader loading
TEST_F(LightingTest, ShaderLoading) {
    bool success = renderer->loadShader("shaders/default.vert", "shaders/default.frag");
    EXPECT_TRUE(success);
}

// Test matrix transformations
TEST_F(LightingTest, MatrixTransformations) {
    // Test view matrix
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),   // Camera position
        glm::vec3(0.0f),                // Look at origin
        glm::vec3(0.0f, 1.0f, 0.0f)     // Up vector
    );
    renderer->setViewMatrix(view);
    
    // Test projection matrix
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),            // FOV
        800.0f / 600.0f,                // Aspect ratio
        0.1f,                           // Near plane
        100.0f                          // Far plane
    );
    renderer->setProjectionMatrix(projection);
}

// Test clear color
TEST_F(LightingTest, ClearColor) {
    renderer->setClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    renderer->clear();
    
    // Get the clear color
    GLfloat color[4];
    glGetFloatv(GL_COLOR_CLEAR_VALUE, color);
    
    EXPECT_NEAR(color[0], 0.2f, 0.001f);
    EXPECT_NEAR(color[1], 0.3f, 0.001f);
    EXPECT_NEAR(color[2], 0.3f, 0.001f);
    EXPECT_NEAR(color[3], 1.0f, 0.001f);
}

// Test basic rendering
TEST_F(LightingTest, BasicRendering) {
    // Set up a basic scene
    renderer->setClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    renderer->setViewMatrix(view);
    
    glm::mat4 projection = glm::perspective(
        glm::radians(45.0f),
        800.0f / 600.0f,
        0.1f,
        100.0f
    );
    renderer->setProjectionMatrix(projection);
    
    // Add a light
    Light light(
        glm::vec3(1.0f, 1.0f, 1.0f),
        glm::vec3(1.0f),
        1.0f,
        10.0f
    );
    renderer->addLight(light);
    
    // Clear and draw
    renderer->clear();
    renderer->drawTriangle(glm::vec3(0.0f), 1.0f);
    renderer->drawRectangle(glm::vec3(1.0f, 0.0f, 0.0f), 1.0f, 1.0f);
    renderer->drawCircle(glm::vec3(-1.0f, 0.0f, 0.0f), 0.5f);
} 