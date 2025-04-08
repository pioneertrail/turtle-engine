#pragma once

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#define _USE_MATH_DEFINES
#include <cmath>

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
    float radius;  // Radius of light influence
    ShadowMap shadowMap;
    
    Light(const glm::vec3& pos, const glm::vec3& col, float intens = 1.0f, float rad = 10.0f)
        : position(pos)
        , color(col)
        , intensity(intens)
        , radius(rad) {}
};

class Renderer {
public:
    static const int MAX_LIGHTS = 8;  // Maximum number of lights we'll support
    static const int SHADOW_WIDTH = 1024;
    static const int SHADOW_HEIGHT = 1024;

    Renderer();
    ~Renderer();

    void init();
    void cleanup();
    void clear();
    void setClearColor(float r, float g, float b, float a);
    void setViewMatrix(const glm::mat4& view);
    void setProjectionMatrix(const glm::mat4& projection);
    void useShader();
    
    // Light management
    void addLight(const Light& light);
    void removeLight(int index);
    void updateLight(int index, const Light& light);
    void clearLights();
    
    void drawTriangle(const glm::vec3& pos, float size);
    void drawRectangle(const glm::vec3& pos, float width, float height);
    void drawCircle(const glm::vec3& pos, float radius);
    
    bool loadShader(const std::string& vertexPath, const std::string& fragmentPath);

private:
    GLuint defaultShader;
    GLuint shadowShader;  // Shader for rendering depth maps
    GLuint triangleVAO, triangleVBO;
    GLuint rectangleVAO, rectangleVBO;
    GLuint circleVAO, circleVBO;
    
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec4 clearColor;
    glm::vec3 lightPos;   // Legacy, will be removed
    glm::vec3 lightColor; // Legacy, will be removed
    glm::vec3 viewPos;
    
    // Light storage
    std::vector<Light> lights;
    
    // Shadow mapping methods
    void initShadowMap(Light& light);
    void renderShadowMaps();
    void renderSceneToShadowMap(const Light& light);
    glm::mat4 calculateLightSpaceMatrix(const Light& light);
    
    void setLightingUniforms();
    void setUniform(const char* name, const glm::mat4& value);
    void setUniform(const char* name, const glm::vec4& value);
    void setUniform(const char* name, const glm::vec3& value);
    void setUniform(const char* name, float value);
    void setUniform(const char* name, int value);
    
    void checkShaderCompilation(GLuint shader, const char* type);
    GLuint createShader(GLenum type, const char* source);
    std::vector<float> generateCircleVertices(int segments);
};