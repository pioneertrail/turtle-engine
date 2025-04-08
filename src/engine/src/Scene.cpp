#include "Scene.h"

Scene::Scene() {
    // Initialize default scene settings
    camera.position = glm::vec3(0.0f, 0.0f, 3.0f);
    camera.target = glm::vec3(0.0f, 0.0f, 0.0f);
    camera.up = glm::vec3(0.0f, 1.0f, 0.0f);
    
    // Initialize default directional light
    directionalLight.direction = glm::normalize(glm::vec3(-0.2f, -1.0f, -0.3f));
    directionalLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    directionalLight.intensity = 0.5f;
    
    // Initialize default point light
    pointLight.position = glm::vec3(2.0f, 2.0f, 2.0f);
    pointLight.color = glm::vec3(1.0f, 1.0f, 1.0f);
    pointLight.intensity = 1.0f;
    pointLight.constant = 1.0f;
    pointLight.linear = 0.09f;
    pointLight.quadratic = 0.032f;
}

void Scene::update(float deltaTime) {
    // Update scene logic here
    // This could include updating object positions, animations, etc.
}

glm::mat4 Scene::getViewMatrix() const {
    return glm::lookAt(camera.position, camera.target, camera.up);
}

glm::mat4 Scene::getProjectionMatrix() const {
    return glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);
}

const DirectionalLight& Scene::getDirectionalLight() const {
    return directionalLight;
}

const PointLight& Scene::getPointLight() const {
    return pointLight;
}

void Scene::setDirectionalLight(const DirectionalLight& light) {
    directionalLight = light;
}

void Scene::setPointLight(const PointLight& light) {
    pointLight = light;
}

void Scene::setCameraPosition(const glm::vec3& position) {
    camera.position = position;
}

void Scene::setCameraTarget(const glm::vec3& target) {
    camera.target = target;
}

void Scene::setCameraUp(const glm::vec3& up) {
    camera.up = up;
} 