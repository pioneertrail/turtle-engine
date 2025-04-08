#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <array>

namespace TurtleEngine {

class InputManager {
public:
    InputManager(GLFWwindow* window);
    ~InputManager();
    
    void update();
    
    // Keyboard input
    bool isKeyPressed(int key) const;
    bool isKeyJustPressed(int key) const;
    bool isKeyJustReleased(int key) const;
    
    // Mouse input
    bool isMouseButtonPressed(int button) const;
    bool isMouseButtonJustPressed(int button) const;
    bool isMouseButtonJustReleased(int button) const;
    
    glm::vec2 getMousePosition() const { return mousePosition; }
    glm::vec2 getPreviousMousePosition() const { return previousMousePosition; }
    glm::vec2 getMouseDelta() const { return mouseDelta; }
    
    // Event handlers
    void onKeyPress(int key);
    void onKeyRelease(int key);
    void onMouseButtonPress(int button);
    void onMouseButtonRelease(int button);
    void onMouseMove(double xpos, double ypos);
    
    // GLFW callbacks
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    
private:
    GLFWwindow* window;
    
    // Keyboard state
    std::array<bool, GLFW_KEY_LAST + 1> currentKeyState;
    std::array<bool, GLFW_KEY_LAST + 1> previousKeyState;
    
    // Mouse button state
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> currentMouseState;
    std::array<bool, GLFW_MOUSE_BUTTON_LAST + 1> previousMouseState;
    
    // Mouse position
    glm::vec2 mousePosition;
    glm::vec2 previousMousePosition;
    glm::vec2 mouseDelta;
};

} // namespace TurtleEngine 