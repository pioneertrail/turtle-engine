#include "InputManager.hpp"

namespace TurtleEngine {

InputManager::InputManager(GLFWwindow* window) : window(window) {
    // Initialize keyboard state
    currentKeyState.fill(false);
    previousKeyState.fill(false);
    
    // Initialize mouse state
    currentMouseState.fill(false);
    previousMouseState.fill(false);
    
    // Initialize mouse position
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    previousMousePosition = mousePosition;
    mouseDelta = glm::vec2(0.0f);
}

InputManager::~InputManager() {
}

void InputManager::update() {
    // Update keyboard state
    previousKeyState = currentKeyState;
    for (int i = 0; i <= GLFW_KEY_LAST; ++i) {
        currentKeyState[i] = glfwGetKey(window, i) == GLFW_PRESS;
    }
    
    // Update mouse button state
    previousMouseState = currentMouseState;
    for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; ++i) {
        currentMouseState[i] = glfwGetMouseButton(window, i) == GLFW_PRESS;
    }
    
    // Update mouse position
    previousMousePosition = mousePosition;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);
    mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    mouseDelta = mousePosition - previousMousePosition;
}

bool InputManager::isKeyPressed(int key) const {
    return currentKeyState[key];
}

bool InputManager::isKeyJustPressed(int key) const {
    return currentKeyState[key] && !previousKeyState[key];
}

bool InputManager::isKeyJustReleased(int key) const {
    return !currentKeyState[key] && previousKeyState[key];
}

bool InputManager::isMouseButtonPressed(int button) const {
    return currentMouseState[button];
}

bool InputManager::isMouseButtonJustPressed(int button) const {
    return currentMouseState[button] && !previousMouseState[button];
}

bool InputManager::isMouseButtonJustReleased(int button) const {
    return !currentMouseState[button] && previousMouseState[button];
}

void InputManager::onKeyPress(int key) {
    currentKeyState[key] = true;
}

void InputManager::onKeyRelease(int key) {
    currentKeyState[key] = false;
}

void InputManager::onMouseButtonPress(int button) {
    currentMouseState[button] = true;
}

void InputManager::onMouseButtonRelease(int button) {
    currentMouseState[button] = false;
}

void InputManager::onMouseMove(double xpos, double ypos) {
    previousMousePosition = mousePosition;
    mousePosition = glm::vec2(static_cast<float>(xpos), static_cast<float>(ypos));
    mouseDelta = mousePosition - previousMousePosition;
}

void InputManager::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    // Handle key events
}

void InputManager::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    // Handle mouse button events
}

void InputManager::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    // Handle mouse movement events
}

} // namespace TurtleEngine 