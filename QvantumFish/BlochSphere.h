#ifndef BLOCH_SPHERE_H
#define BLOCH_SPHERE_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class BlochSphere {
private:
    unsigned int sphereVAO, sphereVBO;
    unsigned int discVAO, discVBO;
    unsigned int shaderProgram;

    int slices, stacks;
    int verticesPerLongitude;
    int verticesPerLatitude;
    int totalLongitudeVertices;
    int totalLatitudeVertices;
    int verticesPerDisc;

    float radius;
    glm::vec3 color;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateMinimalSphere(float radius, int slices, int stacks);
    std::vector<float> generateMiddleDisc(float radius, int segments);
    void compileShaders();
    void createSphereGeometry();
    void createDiscGeometry();

public:
    BlochSphere(float sphereRadius = 1.0f, int sphereSlices = 32, int sphereStacks = 32);
    ~BlochSphere();

    void cleanup();
    void rebuild(float newRadius = -1.0f, int newSlices = -1, int newStacks = -1);
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f);
    void setColor(const glm::vec3& newColor);
    glm::vec3 getColor() const;
};

#endif // BLOCH_SPHERE_H