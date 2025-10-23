#ifndef VECTOR_ARROW_H
#define VECTOR_ARROW_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class VectorArrow {
private:
    unsigned int lineVAO, lineVBO;
    unsigned int coneVAO, coneVBO;
    unsigned int shaderProgram;

    float radius;
    glm::vec3 color;
    glm::vec3 position;

    int lineSegments;
    int coneSlices;
    float coneHeight;
    float coneBaseRadius;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateLineVertices(const glm::vec3& start, const glm::vec3& end, int segments);
    std::vector<float> generateConeVertices(float height, float baseRadius, int slices);
    void compileShaders();
    void createLineGeometry();
    void createConeGeometry();

public:
    VectorArrow(const glm::vec3& vecPosition = glm::vec3(0.0f, 0.0f, 1.0f),
        float vectorRadius = 1.0f,
        float arrowHeight = 0.2f,
        float arrowBaseRadius = 0.08f,
        int lineSegmentsCount = 8,
        int coneSlicesCount = 16);
    ~VectorArrow();

    void cleanup();
    void rebuild(const glm::vec3& newPosition = glm::vec3(0.0f));
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f) const;
    void setColor(const glm::vec3& newColor);
    void setPosition(const glm::vec3& newPosition);
    glm::vec3 getColor() const;
    glm::vec3 getPosition() const;
};

#endif // VECTOR_ARROW_H