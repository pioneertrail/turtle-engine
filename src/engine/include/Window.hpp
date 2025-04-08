#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <string>

namespace TurtleEngine {

/**
 * @brief Window management class that handles the GLFW window and input.
 * 
 * The Window class encapsulates GLFW window creation, destruction, and management.
 * It provides methods for handling window events, buffer swapping, and input processing.
 */
class Window {
public:
    /**
     * @brief Constructs a new Window instance.
     * 
     * Creates a GLFW window with the specified dimensions and title.
     * Sets up the OpenGL context and basic window hints.
     * 
     * @param width The window width in pixels
     * @param height The window height in pixels
     * @param title The window title
     */
    Window(int width, int height, const std::string& title);

    /**
     * @brief Destroys the Window instance.
     * 
     * Destroys the GLFW window and cleans up associated resources.
     */
    ~Window();
    
    /**
     * @brief Checks if the window was created successfully.
     * 
     * @return true if the window is valid and ready to use, false otherwise
     */
    bool isValid() const;

    /**
     * @brief Checks if the window should close.
     * 
     * @return true if the window has been marked for closing, false otherwise
     */
    bool shouldClose() const;

    /**
     * @brief Processes pending window events.
     * 
     * Processes window events like input, window movement, resizing, etc.
     */
    void pollEvents();

    /**
     * @brief Swaps the front and back buffers.
     * 
     * Updates the window's contents by swapping the color buffers.
     */
    void swapBuffers();

    /**
     * @brief Gets the native GLFW window handle.
     * 
     * @return Pointer to the GLFWwindow instance
     */
    GLFWwindow* getHandle() const { return window; }
    
    // Input handling
    /**
     * @brief Checks if a keyboard key is currently pressed.
     * 
     * @param key The GLFW key code to check
     * @return true if the key is pressed, false otherwise
     */
    bool isKeyPressed(int key) const;

    /**
     * @brief Checks if a mouse button is currently pressed.
     * 
     * @param button The GLFW mouse button code to check
     * @return true if the button is pressed, false otherwise
     */
    bool isMouseButtonPressed(int button) const;

    /**
     * @brief Gets the current mouse cursor position.
     * 
     * @return Vector containing the x and y coordinates in screen space
     */
    glm::vec2 getMousePosition() const;
    
private:
    GLFWwindow* window;  ///< Pointer to the GLFW window instance
};

} // namespace TurtleEngine 