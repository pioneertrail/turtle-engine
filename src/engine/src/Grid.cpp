#include "Grid.hpp"
#include <glm/gtc/matrix_transform.hpp>

namespace TurtleEngine {

Grid::Grid(int width, int height, float cellSize)
    : m_width(width), m_height(height), m_cellSize(cellSize) {
    m_colors.resize(width * height, glm::vec3(0.2f, 0.2f, 0.2f));
    initializeGrid();
}

Grid::~Grid() {
    glDeleteVertexArrays(1, &m_VAO);
    glDeleteBuffers(1, &m_VBO);
    glDeleteBuffers(1, &m_EBO);
}

void Grid::initializeGrid() {
    // Load shaders
    m_shader.loadFromFiles("shaders/basic.vert", "shaders/basic.frag");
    createBuffers();
}

void Grid::createBuffers() {
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
    // Create vertices for each cell
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float xPos = x * m_cellSize - (m_width * m_cellSize) / 2.0f;
            float zPos = y * m_cellSize - (m_height * m_cellSize) / 2.0f;
            
            // Add vertices for the cell (quad)
            vertices.push_back(xPos);          // Position
            vertices.push_back(0.0f);
            vertices.push_back(zPos);
            vertices.push_back(m_colors[y * m_width + x].r);  // Color
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            vertices.push_back(xPos + m_cellSize);
            vertices.push_back(0.0f);
            vertices.push_back(zPos);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            vertices.push_back(xPos + m_cellSize);
            vertices.push_back(0.0f);
            vertices.push_back(zPos + m_cellSize);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            vertices.push_back(xPos);
            vertices.push_back(0.0f);
            vertices.push_back(zPos + m_cellSize);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            // Add indices for the cell
            unsigned int baseIndex = (y * m_width + x) * 4;
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 1);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex);
            indices.push_back(baseIndex + 2);
            indices.push_back(baseIndex + 3);
        }
    }

    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // Color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void Grid::render(const glm::mat4& projection, const glm::mat4& view) {
    // logToFile("[Grid Render] Entered."); // Removed entry log

    m_shader.use();
    
    // Check shader program validity after use()
    GLint currentProgram = 0;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    // logToFile(std::string("[Grid Render] Shader ID: ") + std::to_string(currentProgram)); // Removed shader ID log

    if (currentProgram == 0) {
        logToFile("[Grid Render] Error: Shader program is not valid or not bound.");
        return; // Don't attempt to render without a valid shader
    }

    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for model
    m_shader.setMat4("projection", projection);
    m_shader.setMat4("view", view);
    m_shader.setMat4("model", model);

    glBindVertexArray(m_VAO);
    // logToFile(std::string("[Grid Render] Drawing Elements: mode=GL_TRIANGLES, count=") + // Removed draw log
    //           std::to_string(m_indices.size()) + ", type=GL_UNSIGNED_INT, indices=0");
    glDrawElements(GL_TRIANGLES, m_width * m_height * 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
    glUseProgram(0); // Unbind shader
}

void Grid::setCellColor(int x, int y, const glm::vec3& color) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        m_colors[y * m_width + x] = color;
        updateColors();
    }
}

void Grid::updateColors() {
    // Update the VBO with new colors
    std::vector<float> vertices;
    for (int y = 0; y < m_height; ++y) {
        for (int x = 0; x < m_width; ++x) {
            float xPos = x * m_cellSize - (m_width * m_cellSize) / 2.0f;
            float zPos = y * m_cellSize - (m_height * m_cellSize) / 2.0f;
            
            // Add vertices for the cell (quad)
            vertices.push_back(xPos);
            vertices.push_back(0.0f);
            vertices.push_back(zPos);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            vertices.push_back(xPos + m_cellSize);
            vertices.push_back(0.0f);
            vertices.push_back(zPos);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            vertices.push_back(xPos + m_cellSize);
            vertices.push_back(0.0f);
            vertices.push_back(zPos + m_cellSize);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
            
            vertices.push_back(xPos);
            vertices.push_back(0.0f);
            vertices.push_back(zPos + m_cellSize);
            vertices.push_back(m_colors[y * m_width + x].r);
            vertices.push_back(m_colors[y * m_width + x].g);
            vertices.push_back(m_colors[y * m_width + x].b);
        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(float), vertices.data());
}

} 