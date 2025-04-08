void Renderer::setUniform(const char* name, const glm::mat4& value) {
    glUseProgram(defaultShader);
    GLint location = glGetUniformLocation(defaultShader, name);
    if (location != -1) {
        glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
    }
}

void Renderer::setUniform(const char* name, const glm::vec3& value) {
    glUseProgram(defaultShader);
    GLint location = glGetUniformLocation(defaultShader, name);
    if (location != -1) {
        glUniform3fv(location, 1, glm::value_ptr(value));
    }
}

void Renderer::setUniform(const char* name, float value) {
    glUseProgram(defaultShader);
    GLint location = glGetUniformLocation(defaultShader, name);
    if (location != -1) {
        glUniform1f(location, value);
    }
}

void Renderer::init() {
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        throw std::runtime_error("Failed to initialize GLEW");
    }

    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);

    // Load shaders
    defaultShader = loadShaders("shaders/basic.vert", "shaders/basic.frag");
    shadowShader = loadShaders("shaders/shadow.vert", "shaders/shadow.frag");

    // Initialize view and projection matrices
    view = glm::lookAt(
        glm::vec3(0.0f, 0.0f, 3.0f),
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);

    // Create VAOs for basic shapes
    createTriangleVAO();
    createRectangleVAO();
    createCircleVAO();

    // Initialize with no lights by default
    lights.clear();
} 