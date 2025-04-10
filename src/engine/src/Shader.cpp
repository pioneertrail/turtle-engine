#include "Shader.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

namespace TurtleEngine {

Shader::Shader() : m_program(0) {}

Shader::~Shader() {
    if (m_program != 0) {
        glDeleteProgram(m_program);
    }
}

bool Shader::loadFromFiles(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;

    vShaderFile.open(vertexPath);
    fShaderFile.open(fragmentPath);
    std::stringstream vShaderStream, fShaderStream;

    vShaderStream << vShaderFile.rdbuf();
    fShaderStream << fShaderFile.rdbuf();

    vertexCode = vShaderStream.str();
    fragmentCode = fShaderStream.str();

    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    unsigned int vertex, fragment;
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    if (!checkCompileErrors(vertex, "VERTEX")) {
        return false;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    if (!checkCompileErrors(fragment, "FRAGMENT")) {
        return false;
    }

    m_program = glCreateProgram();
    glAttachShader(m_program, vertex);
    glAttachShader(m_program, fragment);

    // Add debug log before linking
    std::cout << "DEBUG::SHADER: Attaching shaders [V:" << vertex << ", F:" << fragment << "] to Program [P:" << m_program << "] before linking." << std::endl;

    glLinkProgram(m_program);
    if (!checkCompileErrors(m_program, "PROGRAM")) {
        // Clean up shaders even on link failure before returning
        glDeleteShader(vertex);
        glDeleteShader(fragment);
        return false;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);
    return true;
}

void Shader::use() {
    glUseProgram(m_program);
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(m_program, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(m_program, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(m_program, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(m_program, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(m_program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
}

bool Shader::checkCompileErrors(unsigned int shaderOrProgram, const std::string& type) {
    int success = 0; // Initialize to 0
    char infoLog[1024];
    for(int i=0; i<1024; ++i) infoLog[i] = '\0'; // Clear log buffer

    // Add check for valid ID
    if (shaderOrProgram == 0) {
        std::cout << "ERROR::SHADER: Invalid ID (0) passed to checkCompileErrors for type: " << type << std::endl;
        return false;
    }

    if (type != "PROGRAM") {
        // Check Shader Compilation
        std::cout << "DEBUG::SHADER: Checking compile status for " << type << " (ID: " << shaderOrProgram << ")" << std::endl;
        glGetShaderiv(shaderOrProgram, GL_COMPILE_STATUS, &success);
        if (!success) {
            std::cout << "ERROR::SHADER: Compile failed for " << type << " (ID: " << shaderOrProgram << ")" << std::endl;
            glGetShaderInfoLog(shaderOrProgram, 1024, NULL, infoLog);
            std::cout << "  InfoLog: " << infoLog << std::endl;
            return false;
        } else {
             std::cout << "DEBUG::SHADER: Compile successful for " << type << " (ID: " << shaderOrProgram << ")" << std::endl;
        }
    } else {
        // Check Program Linking
        std::cout << "DEBUG::SHADER: Checking link status for " << type << " (ID: " << shaderOrProgram << ")" << std::endl;
        glGetProgramiv(shaderOrProgram, GL_LINK_STATUS, &success);
        if (!success) {
            std::cout << "ERROR::SHADER: Link failed for " << type << " (ID: " << shaderOrProgram << ")" << std::endl;
            glGetProgramInfoLog(shaderOrProgram, 1024, NULL, infoLog);
            std::cout << "  InfoLog: " << infoLog << std::endl;
            return false;
        } else {
             std::cout << "DEBUG::SHADER: Link successful for " << type << " (ID: " << shaderOrProgram << ")" << std::endl;
        }
    }
    return true;
}

} 