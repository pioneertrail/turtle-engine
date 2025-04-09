#pragma once

#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>

namespace TurtleEngine {

class Shader {
public:
    Shader();
    ~Shader();

    bool loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath);
    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;

private:
    unsigned int m_program;
    bool checkCompileErrors(unsigned int shader, const std::string& type);
};
} 