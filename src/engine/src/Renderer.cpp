#include "Renderer.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace TurtleEngine {

Renderer::Renderer() :
    defaultShader(0),
    shadowShader(0),
    currentShader(0),
    triangleVAO(0),
    triangleVBO(0),
    rectangleVAO(0),
    rectangleVBO(0),
    rectangleEBO(0),
    circleVAO(0),
    circleVBO(0),
    viewMatrix(1.0f),
    projectionMatrix(1.0f),
    clearColor(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Renderer::~Renderer() {
    cleanup();
}

void Renderer::init() {
    // Initialize GLEW
    glewExperimental = GL_TRUE;
    GLenum err = glewInit();
    if (err != GLEW_OK) {
        std::cerr << "GLEW Error: " << glewGetErrorString(err) << std::endl;
        throw std::runtime_error("Failed to initialize GLEW");
    }
    
    // Print OpenGL version for verification
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << std::endl;
    
    // Clear any OpenGL error that might have been generated during GLEW initialization
    while (glGetError() != GL_NO_ERROR) {}
    
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);
    
    // Enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // Initialize shaders
    initShaders();
    
    // Initialize shapes
    initShapes();
    
    // Set default view and projection matrices
    viewMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    
    // Set default uniforms
    setUniform("view", viewMatrix);
    setUniform("projection", projectionMatrix);
}

void Renderer::cleanup() {
    // Delete shaders
    if (defaultShader != 0) {
        glDeleteProgram(defaultShader);
        defaultShader = 0;
    }
    if (shadowShader != 0) {
        glDeleteProgram(shadowShader);
        shadowShader = 0;
    }
    
    // Delete VAOs and VBOs
    if (triangleVAO != 0) {
        glDeleteVertexArrays(1, &triangleVAO);
        triangleVAO = 0;
    }
    if (triangleVBO != 0) {
        glDeleteBuffers(1, &triangleVBO);
        triangleVBO = 0;
    }
    
    if (rectangleVAO != 0) {
        glDeleteVertexArrays(1, &rectangleVAO);
        rectangleVAO = 0;
    }
    if (rectangleVBO != 0) {
        glDeleteBuffers(1, &rectangleVBO);
        rectangleVBO = 0;
    }
    if (rectangleEBO != 0) {
        glDeleteBuffers(1, &rectangleEBO);
        rectangleEBO = 0;
    }
    
    if (circleVAO != 0) {
        glDeleteVertexArrays(1, &circleVAO);
        circleVAO = 0;
    }
    if (circleVBO != 0) {
        glDeleteBuffers(1, &circleVBO);
        circleVBO = 0;
    }
    
    // Reset current shader
    currentShader = 0;
}

void Renderer::clear() {
    glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void Renderer::setClearColor(const glm::vec4& color) {
    clearColor = color;
}

void Renderer::loadShader(const std::string& vertexPath, const std::string& fragmentPath) {
    // Read vertex shader
    std::ifstream vertexFile(vertexPath);
    if (!vertexFile.is_open()) {
        throw std::runtime_error("Failed to open vertex shader file: " + vertexPath);
    }
    std::stringstream vertexStream;
    vertexStream << vertexFile.rdbuf();
    std::string vertexCode = vertexStream.str();
    vertexFile.close();
    
    // Read fragment shader
    std::ifstream fragmentFile(fragmentPath);
    if (!fragmentFile.is_open()) {
        throw std::runtime_error("Failed to open fragment shader file: " + fragmentPath);
    }
    std::stringstream fragmentStream;
    fragmentStream << fragmentFile.rdbuf();
    std::string fragmentCode = fragmentStream.str();
    fragmentFile.close();
    
    // Create shaders
    GLuint vertex = createShader(vertexCode, GL_VERTEX_SHADER);
    GLuint fragment = createShader(fragmentCode, GL_FRAGMENT_SHADER);
    
    // Create program
    GLuint program = glCreateProgram();
    glAttachShader(program, vertex);
    glAttachShader(program, fragment);
    glLinkProgram(program);
    
    checkShaderCompilation(program, "PROGRAM");
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // Set as default shader if none exists
    if (defaultShader == 0) {
        defaultShader = program;
        currentShader = program;
    }
}

void Renderer::initShaders() {
    // Default vertex shader
    std::string defaultVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    // Default fragment shader
    std::string defaultFragmentShader = R"(
        #version 330 core
        out vec4 FragColor;
        
        uniform vec4 color;
        
        void main() {
            FragColor = color;
        }
    )";
    
    // Create default shader
    GLuint vertex = createShader(defaultVertexShader, GL_VERTEX_SHADER);
    GLuint fragment = createShader(defaultFragmentShader, GL_FRAGMENT_SHADER);
    
    defaultShader = glCreateProgram();
    glAttachShader(defaultShader, vertex);
    glAttachShader(defaultShader, fragment);
    glLinkProgram(defaultShader);
    
    checkShaderCompilation(defaultShader, "PROGRAM");
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // Shadow mapping vertex shader
    std::string shadowVertexShader = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 lightSpaceMatrix;
        uniform mat4 model;
        
        void main() {
            gl_Position = lightSpaceMatrix * model * vec4(aPos, 1.0);
        }
    )";
    
    // Shadow mapping fragment shader
    std::string shadowFragmentShader = R"(
        #version 330 core
        void main() {
            // Fragment shader is empty as we only need depth information
        }
    )";
    
    // Create shadow shader
    vertex = createShader(shadowVertexShader, GL_VERTEX_SHADER);
    fragment = createShader(shadowFragmentShader, GL_FRAGMENT_SHADER);
    
    shadowShader = glCreateProgram();
    glAttachShader(shadowShader, vertex);
    glAttachShader(shadowShader, fragment);
    glLinkProgram(shadowShader);
    
    checkShaderCompilation(shadowShader, "PROGRAM");
    
    glDeleteShader(vertex);
    glDeleteShader(fragment);
    
    // Set current shader to default
    currentShader = defaultShader;
    glUseProgram(currentShader);
}

void Renderer::initShapes() {
    // Initialize triangle
    float triangleVertices[] = {
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
         0.0f,  0.5f, 0.0f   // Top
    };
    
    glGenVertexArrays(1, &triangleVAO);
    glGenBuffers(1, &triangleVBO);
    
    glBindVertexArray(triangleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Initialize rectangle
    float rectangleVertices[] = {
        -0.5f, -0.5f, 0.0f,  // Bottom left
         0.5f, -0.5f, 0.0f,  // Bottom right
         0.5f,  0.5f, 0.0f,  // Top right
        -0.5f,  0.5f, 0.0f   // Top left
    };
    
    unsigned int rectangleIndices[] = {
        0, 1, 2,  // First triangle
        2, 3, 0   // Second triangle
    };
    
    glGenVertexArrays(1, &rectangleVAO);
    glGenBuffers(1, &rectangleVBO);
    glGenBuffers(1, &rectangleEBO);
    
    glBindVertexArray(rectangleVAO);
    
    glBindBuffer(GL_ARRAY_BUFFER, rectangleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(rectangleVertices), rectangleVertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, rectangleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(rectangleIndices), rectangleIndices, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Initialize circle
    const int segments = 32;
    std::vector<float> circleVertices;
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    circleVertices.push_back(0.0f);
    
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * float(i) / float(segments);
        circleVertices.push_back(cos(angle) * 0.5f);
        circleVertices.push_back(sin(angle) * 0.5f);
        circleVertices.push_back(0.0f);
    }
    
    glGenVertexArrays(1, &circleVAO);
    glGenBuffers(1, &circleVBO);
    
    glBindVertexArray(circleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, circleVBO);
    glBufferData(GL_ARRAY_BUFFER, circleVertices.size() * sizeof(float), circleVertices.data(), GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Unbind everything
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Renderer::drawTriangle(const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec4& color) {
    if (currentShader == 0) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
    
    setUniform("model", model);
    setUniform("color", color);
    
    glBindVertexArray(triangleVAO);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void Renderer::drawRectangle(const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec4& color) {
    if (currentShader == 0) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::rotate(model, rotation, glm::vec3(0.0f, 0.0f, 1.0f));
    model = glm::scale(model, glm::vec3(scale, 1.0f));
    
    setUniform("model", model);
    setUniform("color", color);
    
    glBindVertexArray(rectangleVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void Renderer::drawCircle(const glm::vec2& position, float radius, const glm::vec4& color) {
    if (currentShader == 0) return;
    
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    model = glm::scale(model, glm::vec3(radius * 2.0f, radius * 2.0f, 1.0f));
    
    setUniform("model", model);
    setUniform("color", color);
    
    glBindVertexArray(circleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 34); // 32 segments + center + first vertex repeated
    glBindVertexArray(0);
}

void Renderer::setUniform(const std::string& name, const glm::mat4& value) {
    GLint location = glGetUniformLocation(currentShader, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(value));
}

void Renderer::setUniform(const std::string& name, const glm::vec3& value) {
    GLint location = glGetUniformLocation(currentShader, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform3fv(location, 1, glm::value_ptr(value));
}

void Renderer::setUniform(const std::string& name, const glm::vec4& value) {
    GLint location = glGetUniformLocation(currentShader, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform4fv(location, 1, glm::value_ptr(value));
}

void Renderer::setUniform(const std::string& name, float value) {
    GLint location = glGetUniformLocation(currentShader, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform1f(location, value);
}

void Renderer::setUniform(const std::string& name, int value) {
    GLint location = glGetUniformLocation(currentShader, name.c_str());
    if (location == -1) {
        std::cerr << "Warning: Uniform '" << name << "' not found in shader" << std::endl;
        return;
    }
    glUniform1i(location, value);
}

GLuint Renderer::createShader(const std::string& source, GLenum type) {
    GLuint shader = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);
    
    checkShaderCompilation(shader, type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");
    
    return shader;
}

void Renderer::checkShaderCompilation(GLuint shader, const std::string& type) {
    GLint success;
    GLchar infoLog[512];
    
    if (type == "PROGRAM") {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
            throw std::runtime_error("Shader program linking failed");
        }
    } else {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 512, NULL, infoLog);
            std::cerr << "ERROR::SHADER::" << type << "::COMPILATION_FAILED\n" << infoLog << std::endl;
            throw std::runtime_error("Shader compilation failed");
        }
    }
}

void Renderer::setViewMatrix(const glm::mat4& view) {
    viewMatrix = view;
    if (currentShader != 0) {
        setUniform("view", viewMatrix);
    }
}

void Renderer::setProjectionMatrix(const glm::mat4& projection) {
    projectionMatrix = projection;
    if (currentShader != 0) {
        setUniform("projection", projectionMatrix);
    }
}

void Renderer::addLight(const Light& light) {
    if (lights.size() < MAX_LIGHTS) {
        lights.push_back(light);
        initShadowMap(lights.back());
        setLightingUniforms();
    }
}

void Renderer::removeLight(int index) {
    if (index >= 0 && index < lights.size()) {
        lights.erase(lights.begin() + index);
        setLightingUniforms();
    }
}

void Renderer::updateLight(int index, const Light& light) {
    if (index >= 0 && index < lights.size()) {
        lights[index] = light;
        setLightingUniforms();
    }
}

void Renderer::clearLights() {
    lights.clear();
    setLightingUniforms();
}

void Renderer::useShader(GLuint shaderProgram) {
    if (shaderProgram != 0) {
        currentShader = shaderProgram;
        glUseProgram(currentShader);
        
        // Update matrices
        setUniform("view", viewMatrix);
        setUniform("projection", projectionMatrix);
        
        // Update lighting uniforms
        setLightingUniforms();
    }
}

std::vector<float> Renderer::generateCircleVertices(int segments) {
    std::vector<float> vertices;
    vertices.reserve((segments + 2) * 3);
    
    // Center vertex
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    
    // Generate circle vertices
    for (int i = 0; i <= segments; ++i) {
        float angle = 2.0f * M_PI * float(i) / float(segments);
        vertices.push_back(cos(angle) * 0.5f);
        vertices.push_back(sin(angle) * 0.5f);
        vertices.push_back(0.0f);
    }
    
    return vertices;
}

void Renderer::initShadowMap(Light& light) {
    // Generate framebuffer
    glGenFramebuffers(1, &light.shadowMap.depthMapFBO);
    
    // Generate depth texture
    glGenTextures(1, &light.shadowMap.depthMap);
    glBindTexture(GL_TEXTURE_2D, light.shadowMap.depthMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    
    // Attach depth texture to framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, light.shadowMap.depthMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, light.shadowMap.depthMap, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    
    // Check framebuffer status
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        throw std::runtime_error("Framebuffer is not complete!");
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    
    // Calculate initial light space matrix
    light.shadowMap.lightSpaceMatrix = calculateLightSpaceMatrix(light);
}

void Renderer::renderShadowMaps() {
    // Save current viewport
    GLint viewport[4];
    glGetIntegerv(GL_VIEWPORT, viewport);
    
    // Use shadow shader
    useShader(shadowShader);
    
    // Set viewport for shadow maps
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    
    for (const auto& light : lights) {
        renderSceneToShadowMap(light);
    }
    
    // Restore viewport
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    
    // Switch back to default shader
    useShader(defaultShader);
}

void Renderer::renderSceneToShadowMap(const Light& light) {
    glBindFramebuffer(GL_FRAMEBUFFER, light.shadowMap.depthMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);
    
    // Set light space matrix uniform
    setUniform("lightSpaceMatrix", light.shadowMap.lightSpaceMatrix);
    
    // Render scene geometry here
    // Note: This should be a simplified version of your regular rendering code
    // that only renders the depth information
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 Renderer::calculateLightSpaceMatrix(const Light& light) {
    float near_plane = 1.0f;
    float far_plane = light.radius;
    
    // Create orthographic projection for directional light
    glm::mat4 lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
    
    // Create view matrix from light's perspective
    glm::mat4 lightView = glm::lookAt(
        light.position,
        glm::vec3(0.0f), // Look at the center
        glm::vec3(0.0f, 1.0f, 0.0f)
    );
    
    return lightProjection * lightView;
}

void Renderer::setLightingUniforms() {
    if (currentShader == 0) return;
    
    // Set number of active lights
    setUniform("numLights", static_cast<int>(lights.size()));
    
    // Set light properties
    for (size_t i = 0; i < lights.size(); ++i) {
        std::string prefix = "lights[" + std::to_string(i) + "].";
        setUniform(prefix + "position", lights[i].position);
        setUniform(prefix + "color", lights[i].color);
        setUniform(prefix + "intensity", lights[i].intensity);
        setUniform(prefix + "radius", lights[i].radius);
        setUniform(prefix + "lightSpaceMatrix", lights[i].shadowMap.lightSpaceMatrix);
        
        // Bind shadow map texture
        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, lights[i].shadowMap.depthMap);
        setUniform(prefix + "shadowMap", static_cast<int>(i));
    }
}

} // namespace TurtleEngine