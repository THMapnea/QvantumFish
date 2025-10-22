#ifndef COORDINATES_AXES_H
#define COORDINATES_AXES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

class CoordinateAxes {
private:
    unsigned int axesVAO, axesVBO;
    unsigned int labelsVAO, labelsVBO;
    unsigned int shaderProgram;

    float axisLength;
    float axisThickness;
    glm::vec3 xColor, yColor, zColor;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateAxisVertices();
    std::vector<float> generateLabelVertices();
    void compileShaders();
    void createAxesGeometry();
    void createLabelsGeometry();

public:
    CoordinateAxes(float length = 1.2f,
        float thickness = 0.02f,
        glm::vec3 xAxisColor = glm::vec3(1.0f, 0.2f, 0.2f),
        glm::vec3 yAxisColor = glm::vec3(0.2f, 1.0f, 0.2f),
        glm::vec3 zAxisColor = glm::vec3(0.2f, 0.2f, 1.0f));
    ~CoordinateAxes();

    void cleanup();
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f);
    void setAxisLength(float length);
    void setAxisColors(const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor);
    float getAxisLength() const;
};

#endif // COORDINATE_AXES_H