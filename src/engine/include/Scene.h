#ifndef SCENE_H
#define SCENE_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

struct Camera {
    glm::vec3 position;
    glm::vec3 target;
    glm::vec3 up;
};

struct DirectionalLight {
    glm::vec3 direction;
    glm::vec3 color;
    float intensity;
};

struct PointLight {
    glm::vec3 position;
    glm::vec3 color;
    float intensity;
    float constant;
    float linear;
    float quadratic;
};

class Scene {
public:
    Scene();
    
    void update(float deltaTime);
    
    // Camera functions
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;
    
    // Light functions
    const DirectionalLight& getDirectionalLight() const;
    const PointLight& getPointLight() const;
    void setDirectionalLight(const DirectionalLight& light);
    void setPointLight(const PointLight& light);
    
    // Camera control functions
    void setCameraPosition(const glm::vec3& position);
    void setCameraTarget(const glm::vec3& target);
    void setCameraUp(const glm::vec3& up);
    
private:
    Camera camera;
    DirectionalLight directionalLight;
    PointLight pointLight;
};

#endif 