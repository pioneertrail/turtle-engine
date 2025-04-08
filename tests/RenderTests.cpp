TEST_CASE("UniformSettings", "[renderer]") {
    // Initialize GLFW
    if (!glfwInit()) {
        FAIL("Failed to initialize GLFW");
    }

    // Configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE); // Make window invisible

    // Create window
    GLFWwindow* window = glfwCreateWindow(800, 600, "Test Window", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        FAIL("Failed to create GLFW window");
    }

    // Make OpenGL context current
    glfwMakeContextCurrent(window);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        glfwDestroyWindow(window);
        glfwTerminate();
        FAIL("Failed to initialize GLEW");
    }

    Renderer renderer;
    renderer.init();

    // Set up viewport and clear color
    glViewport(0, 0, 800, 600);
    renderer.setClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    
    // Make sure we're rendering to the default framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    renderer.clear();

    // Load shader and set it as default
    REQUIRE(renderer.loadShader("shaders/test.vert", "shaders/test.frag"));
    GLuint shader = renderer.getDefaultShader();
    glUseProgram(shader);

    // Set up uniforms
    glm::vec4 testColor(0.5f, 0.25f, 0.75f, 1.0f);
    GLint colorLoc = glGetUniformLocation(shader, "color");
    REQUIRE(colorLoc != -1);
    glUniform4fv(colorLoc, 1, glm::value_ptr(testColor));

    // Set up view and projection matrices
    glm::mat4 view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 5.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    glm::mat4 projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    GLint viewLoc = glGetUniformLocation(shader, "view");
    REQUIRE(viewLoc != -1);
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    GLint projLoc = glGetUniformLocation(shader, "projection");
    REQUIRE(projLoc != -1);
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // Draw a triangle
    renderer.drawTriangle(glm::vec3(0.0f, 0.0f, 0.0f), 1.0f);
    glFinish(); // Make sure rendering is complete

    // Debug: Print shader program and uniform values
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    std::cout << "Current shader program: " << currentProgram << std::endl;
    std::cout << "Expected shader program: " << shader << std::endl;

    glm::vec4 currentColor;
    glGetUniformfv(shader, colorLoc, glm::value_ptr(currentColor));
    std::cout << "Current color: " << currentColor.r << ", " << currentColor.g << ", " << currentColor.b << ", " << currentColor.a << std::endl;
    std::cout << "Expected color: " << testColor.r << ", " << testColor.g << ", " << testColor.b << ", " << testColor.a << std::endl;

    // Read back pixel color from the center of the triangle
    unsigned char pixel[4];
    glReadBuffer(GL_BACK);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(400, 300, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, pixel);

    // Debug: Print pixel values
    std::cout << "Pixel values: " << (int)pixel[0] << ", " << (int)pixel[1] << ", " << (int)pixel[2] << ", " << (int)pixel[3] << std::endl;

    // Convert pixel values to float [0,1] range and apply inverse gamma correction
    float r = std::pow(pixel[0] / 255.0f, 2.2f);
    float g = std::pow(pixel[1] / 255.0f, 2.2f);
    float b = std::pow(pixel[2] / 255.0f, 2.2f);
    float a = pixel[3] / 255.0f;

    // Allow some tolerance due to color space conversion and rounding
    const float tolerance = 0.02f;  // 2% tolerance
    REQUIRE(r == Approx(testColor.r).epsilon(tolerance));
    REQUIRE(g == Approx(testColor.g).epsilon(tolerance));
    REQUIRE(b == Approx(testColor.b).epsilon(tolerance));
    REQUIRE(a == Approx(testColor.a).epsilon(tolerance));

    // Clean up
    glfwDestroyWindow(window);
    glfwTerminate();
} 