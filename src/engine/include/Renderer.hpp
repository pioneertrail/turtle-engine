#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <map>

namespace TurtleEngine {

struct ShadowMap {
    GLuint depthMapFBO;
    GLuint depthMap;
    glm::mat4 lightSpaceMatrix;
    
    ShadowMap() : depthMapFBO(0), depthMap(0), lightSpaceMatrix(1.0f) {}
};

struct Light {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float radius;
    ShadowMap shadowMap;
    
    Light(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f, float rad = 10.0f)
        : position(pos), color(col), intensity(intens), radius(rad) {}
};

class Renderer {
public:
    static const int MAX_LIGHTS = 8;
    static const int SHADOW_WIDTH = 1024;
    static const int SHADOW_HEIGHT = 1024;

    Renderer();
    ~Renderer();

    void init();
    void cleanup();
    void clear();
    void setClearColor(const glm::vec4& color);
    
    // Basic rendering functions
    void drawTriangle(const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec4& color);
    void drawRectangle(const glm::vec2& position, float rotation, const glm::vec2& scale, const glm::vec4& color);
    void drawCircle(const glm::vec2& position, float radius, const glm::vec4& color);
    
    // Camera control
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);
    
    // Light management
    void addLight(const Light& light);
    void removeLight(int index);
    void updateLight(int index, const Light& light);
    void clearLights();
    
    // Shader management
    void loadShader(const std::string& vertexPath, const std::string& fragmentPath);
    GLuint createShader(const std::string& source, GLenum type);
    void checkShaderCompilation(GLuint shader, const std::string& type);
    void useShader(GLuint shaderProgram);
    
    // Uniform setting
    void setUniform(const std::string& name, const glm::mat4& value);
    void setUniform(const std::string& name, const glm::vec3& value);
    void setUniform(const std::string& name, const glm::vec4& value);
    void setUniform(const std::string& name, float value);
    void setUniform(const std::string& name, int value);
    
    // Test accessors
    GLuint getDefaultShader() const { return defaultShader; }
    GLuint getShadowShader() const { return shadowShader; }
    GLuint getTriangleVAO() const { return triangleVAO; }
    GLuint getRectangleVAO() const { return rectangleVAO; }
    GLuint getCircleVAO() const { return circleVAO; }
    const std::vector<Light>& getLights() const { return lights; }
    const glm::mat4& getViewMatrix() const { return viewMatrix; }
    const glm::mat4& getProjectionMatrix() const { return projectionMatrix; }

private:
    void initShaders();
    void initShapes();
    
    // Shader programs
    GLuint defaultShader;
    GLuint shadowShader;
    GLuint currentShader;  // Currently active shader program
    
    // Vertex arrays and buffers
    GLuint triangleVAO;
    GLuint triangleVBO;
    GLuint rectangleVAO;
    GLuint rectangleVBO;
    GLuint rectangleEBO;
    GLuint circleVAO;
    GLuint circleVBO;
    
    // Matrices
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    
    // State
    glm::vec4 clearColor;
    
    // Lighting
    std::vector<Light> lights;
    
    // Helper functions
    std::vector<float> generateCircleVertices(int segments);
    
    // Shadow mapping
    void initShadowMap(Light& light);
    void renderShadowMaps();
    void renderSceneToShadowMap(const Light& light);
    glm::mat4 calculateLightSpaceMatrix(const Light& light);
    void setLightingUniforms();
};

} // namespace TurtleEngine 