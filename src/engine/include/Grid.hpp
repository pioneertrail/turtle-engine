#pragma once

#include <vector>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include "Shader.hpp"

namespace TurtleEngine {

class Grid {
public:
    Grid(int width, int height, float cellSize);
    ~Grid();

    void render(const glm::mat4& view, const glm::mat4& projection);
    void setCellColor(int x, int y, const glm::vec3& color);

private:
    void initializeGrid();
    void createBuffers();
    void updateColors();

    int m_width;
    int m_height;
    float m_cellSize;
    std::vector<glm::vec3> m_colors;
    
    unsigned int m_VAO;
    unsigned int m_VBO;
    unsigned int m_EBO;
    Shader m_shader;
};
} 