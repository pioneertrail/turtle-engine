#include <iostream>
#include <cassert>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>
#include "Renderer.hpp"
#include <fstream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <string>
#include <windows.h>
#include <cmath>
#include "RenderTests.hpp"

using namespace TurtleEngine;

void RenderTest::SetUp() {
    // Initialize GLFW
    if (!glfwInit()) {
        FAIL() << "Failed to initialize GLFW";
        return;
    }
    
    // Set OpenGL context hints
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    
    // Create a hidden window for OpenGL context
    window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        FAIL() << "Failed to create GLFW window";
        return;
    }
    
    glfwMakeContextCurrent(window);
    
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        FAIL() << "Failed to initialize GLEW: " << glewGetErrorString(err);
        return;
    }
    
    // Print OpenGL version for verification
    const GLubyte* version = glGetString(GL_VERSION);
    if (version) {
        std::cout << "OpenGL Version: " << version << std::endl;
    } else {
        std::cerr << "Failed to get OpenGL version" << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        FAIL() << "Failed to get OpenGL version";
        return;
    }
    
    // Clear any OpenGL error that might have been generated during initialization
    while (glGetError() != GL_NO_ERROR) {}
    
    // Initialize renderer
    try {
        renderer.init();
    } catch (const std::exception& e) {
        std::cerr << "Renderer initialization failed: " << e.what() << std::endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        FAIL() << "Renderer initialization failed: " << e.what();
        return;
    }
}

void RenderTest::TearDown() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

TEST_F(RenderTest, ShaderCompilation) {
    // Test shader compilation
    std::string vertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";
    
    std::string fragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        void main() {
            FragColor = vec4(1.0, 0.0, 0.0, 1.0);
        }
    )";
    
    GLuint vertex = renderer.createShader(vertexShader, GL_VERTEX_SHADER);
    GLuint fragment = renderer.createShader(fragmentShader, GL_FRAGMENT_SHADER);
    
    EXPECT_NE(vertex, 0);
    EXPECT_NE(fragment, 0);
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

TEST_F(RenderTest, UniformSetting) {
    // Test uniform setting
    glm::mat4 model = glm::mat4(1.0f);
    glm::vec3 position(1.0f, 2.0f, 3.0f);
    glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);
    float value = 42.0f;
    int count = 10;
    
    // These should not throw
    EXPECT_NO_THROW(renderer.setUniform("model", model));
    EXPECT_NO_THROW(renderer.setUniform("position", position));
    EXPECT_NO_THROW(renderer.setUniform("color", color));
    EXPECT_NO_THROW(renderer.setUniform("value", value));
    EXPECT_NO_THROW(renderer.setUniform("count", count));
}

TEST_F(RenderTest, ShapeDrawing) {
    // Test shape drawing
    glm::vec2 position(0.0f, 0.0f);
    float rotation = 0.0f;
    glm::vec2 scale(1.0f, 1.0f);
    glm::vec4 color(1.0f, 0.0f, 0.0f, 1.0f);
    float radius = 0.5f;
    
    // These should not throw
    EXPECT_NO_THROW(renderer.drawTriangle(position, rotation, scale, color));
    EXPECT_NO_THROW(renderer.drawRectangle(position, rotation, scale, color));
    EXPECT_NO_THROW(renderer.drawCircle(position, radius, color));
}

TEST_F(RenderTest, ClearColor) {
    // Test clear color setting
    glm::vec4 color(0.5f, 0.5f, 0.5f, 1.0f);
    renderer.setClearColor(color);
    renderer.clear();
    
    // No errors should occur
    EXPECT_NO_THROW(renderer.clear());
}

int main(int argc, char** argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
} 