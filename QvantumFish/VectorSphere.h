#ifndef VECTOR_SPHERE_H
#define VECTOR_SPHERE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class VectorSphere {
private:
    unsigned int lineVAO, lineVBO;
    unsigned int sphereVAO, sphereVBO;
    unsigned int shaderProgram;

    float radius;
    glm::vec3 color;
    glm::vec3 position;

    int lineSegments;
    int sphereSlices, sphereStacks;
    float sphereRadius;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateLineVertices(const glm::vec3& start, const glm::vec3& end, int segments);
    std::vector<float> generateSphereVertices(float radius, int slices, int stacks);
    void compileShaders();
    void createLineGeometry();
    void createSphereGeometry();

public:
    VectorSphere(const glm::vec3& vecPosition = glm::vec3(0.0f, 0.0f, 1.0f),
        float vectorRadius = 1.0f,
        float endSphereRadius = 0.1f,
        int lineSegmentsCount = 8,
        int sphereSlicesCount = 16,
        int sphereStacksCount = 16);
    ~VectorSphere();

    void cleanup();
    void rebuild(const glm::vec3& newPosition = glm::vec3(0.0f));
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f);
    void setColor(const glm::vec3& newColor);
    void setPosition(const glm::vec3& newPosition);
    glm::vec3 getColor() const;
    glm::vec3 getPosition() const;
};

#endif // VECTOR_SPHERE_H