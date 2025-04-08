#ifndef TURTLE_ENGINE_RENDER_TESTS_HPP
#define TURTLE_ENGINE_RENDER_TESTS_HPP

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <gtest/gtest.h>

namespace TurtleEngine {
class Renderer;

class RenderTest : public ::testing::Test {
protected:
    GLFWwindow* window;
    Renderer renderer;

    void SetUp() override;
    void TearDown() override;
};

} // namespace TurtleEngine

#endif // TURTLE_ENGINE_RENDER_TESTS_HPP 