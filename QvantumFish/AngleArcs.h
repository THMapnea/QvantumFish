#ifndef ANGLE_ARCS_H
#define ANGLE_ARCS_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class AngleArcs {
private:
    unsigned int thetaArcVAO, thetaArcVBO;
    unsigned int phiArcVAO, phiArcVBO;
    unsigned int shaderProgram;

    glm::vec3 vectorPosition;
    glm::vec3 color;
    float arcRadius;
    int arcSegments;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateThetaArc();
    std::vector<float> generatePhiArc();
    void compileShaders();
    void createThetaArcGeometry();
    void createPhiArcGeometry();

public:
    AngleArcs(const glm::vec3& position = glm::vec3(0.0f, 0.0f, 1.0f),
        const glm::vec3& arcColor = glm::vec3(0.2f, 0.8f, 0.2f),
        float radius = 0.3f,
        int segments = 32);
    ~AngleArcs();

    void cleanup() const;
    void updatePosition(const glm::vec3& newPosition);
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f);
    void setColor(const glm::vec3& newColor);
    glm::vec3 getColor() const;
};

#endif // ANGLE_ARCS_H