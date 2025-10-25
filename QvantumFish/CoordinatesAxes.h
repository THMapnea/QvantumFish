#ifndef COORDINATES_AXES_H
#define COORDINATES_AXES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class CoordinateAxes {
private:
    unsigned int axesVAO, axesVBO;
    unsigned int labelsVAO, labelsVBO;
    unsigned int textVAO, textVBO;
    unsigned int shaderProgram;

    float axisLength;
    float axisThickness;
    glm::vec3 xColor, yColor, zColor;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateAxisVertices();
    std::vector<float> generateLabelVertices();
    std::vector<float> generateTextVertices();
    void compileShaders();
    void createAxesGeometry();
    void createLabelsGeometry();
    void createTextGeometry();

public:
    CoordinateAxes(float length = 1.2f,
        float thickness = 0.02f,
        glm::vec3 xAxisColor = glm::vec3(0.6f, 0.6f, 0.8f),
        glm::vec3 yAxisColor = glm::vec3(0.6f, 0.6f, 0.8f),
        glm::vec3 zAxisColor = glm::vec3(0.6f, 0.6f, 0.8f));
    ~CoordinateAxes();

    void cleanup();
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f);
    void setAxisLength(float length);
    void setAxisColors(const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor);
    float getAxisLength() const;
};

#endif