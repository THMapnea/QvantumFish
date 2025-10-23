#ifndef PROJECTION_LINES_H
#define PROJECTION_LINES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class ProjectionLines {
private:
    unsigned int verticalLineVAO, verticalLineVBO;
    unsigned int horizontalLineVAO, horizontalLineVBO;
    unsigned int shaderProgram;

    glm::vec3 vectorPosition;
    glm::vec3 color;
    float dashLength;
    int segments;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateVerticalDashedLine();
    std::vector<float> generateHorizontalDashedLine();
    void compileShaders();
    void createVerticalLineGeometry();
    void createHorizontalLineGeometry();

public:
    ProjectionLines(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 1.0f),
        const glm::vec3& lineColor = glm::vec3(0.8f, 0.8f, 0.2f),
        float dashSize = 0.05f,
        int lineSegments = 20);
    ~ProjectionLines();

    void cleanup() const;
    void updatePosition(const glm::vec3& newPosition);
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f) const;
    void setColor(const glm::vec3& newColor);
    glm::vec3 getColor() const;
};

#endif // PROJECTION_LINES_H