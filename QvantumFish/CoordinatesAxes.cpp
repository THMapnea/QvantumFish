#define _USE_MATH_DEFINES

#include "CoordinatesAxes.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader
static const char* axes_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader
static const char* axes_fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    vec3 glowColor = color;
    // Subtle pulse effect
    float pulse = sin(time * 1.5) * 0.03 + 0.97;
    glowColor *= pulse;
    FragColor = vec4(glowColor, opacity);
}
)";

unsigned int CoordinateAxes::compileShader(unsigned int type, const char* source) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    int success;
    char log[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, log);
        std::cout << "ERROR::SHADER::COMPILATION_FAILED\n" << log << std::endl;
    }
    return shader;
}

std::vector<float> CoordinateAxes::generateAxisVertices() {
    std::vector<float> vertices;

    // X-axis - from (-length, 0, 0) to (length, 0, 0)
    vertices.push_back(-axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);

    // Y-axis - from (0, -length, 0) to (0, length, 0)
    vertices.push_back(0.0f); vertices.push_back(-axisLength); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);

    // Z-axis - from (0, 0, -length) to (0, 0, length)
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-axisLength);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);

    return vertices;
}

std::vector<float> CoordinateAxes::generateLabelVertices() {
    std::vector<float> vertices;
    float labelOffset = axisLength * 1.1f;
    float arrowSize = axisLength * 0.08f;

    // X-axis arrow
    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength - arrowSize); vertices.push_back(arrowSize); vertices.push_back(0.0f);
    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength - arrowSize); vertices.push_back(-arrowSize); vertices.push_back(0.0f);

    // Y-axis arrow
    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);
    vertices.push_back(arrowSize); vertices.push_back(axisLength - arrowSize); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);
    vertices.push_back(-arrowSize); vertices.push_back(axisLength - arrowSize); vertices.push_back(0.0f);

    // Z-axis arrow
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);
    vertices.push_back(arrowSize); vertices.push_back(0.0f); vertices.push_back(axisLength - arrowSize);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);
    vertices.push_back(-arrowSize); vertices.push_back(0.0f); vertices.push_back(axisLength - arrowSize);

    return vertices;
}

std::vector<float> CoordinateAxes::generateTextVertices() {
    std::vector<float> vertices;

    // Create geometric representations for quantum state labels
    float labelOffset = axisLength * 1.25f;
    float markerSize = axisLength * 0.03f;

    // Different geometric patterns for different labels
    // |+> and |-> on X-axis: Plus signs
    // |+i> and |-i> on Y-axis: Circles  
    // |0> and |1> on Z-axis: Squares

    // X-axis: |+> and |-> (Plus signs)
    // |+> at (labelOffset, 0, 0)
    vertices.push_back(labelOffset - markerSize); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(labelOffset + markerSize); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(labelOffset); vertices.push_back(-markerSize); vertices.push_back(0.0f);
    vertices.push_back(labelOffset); vertices.push_back(markerSize); vertices.push_back(0.0f);

    // |-> at (-labelOffset, 0, 0) - Horizontal line only
    vertices.push_back(-labelOffset - markerSize); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(-labelOffset + markerSize); vertices.push_back(0.0f); vertices.push_back(0.0f);

    // Y-axis: |+i> and |-i> (Circles - approximated with octagon)
    int circleSegments = 8;
    float circleRadius = markerSize * 1.2f;

    // |+i> at (0, labelOffset, 0)
    for (int i = 0; i < circleSegments; i++) {
        float angle1 = 2.0f * M_PI * i / circleSegments;
        float angle2 = 2.0f * M_PI * (i + 1) / circleSegments;

        vertices.push_back(cos(angle1) * circleRadius);
        vertices.push_back(labelOffset + sin(angle1) * circleRadius);
        vertices.push_back(0.0f);
        vertices.push_back(cos(angle2) * circleRadius);
        vertices.push_back(labelOffset + sin(angle2) * circleRadius);
        vertices.push_back(0.0f);
    }

    // |-i> at (0, -labelOffset, 0) - Circle with dot in center
    for (int i = 0; i < circleSegments; i++) {
        float angle1 = 2.0f * M_PI * i / circleSegments;
        float angle2 = 2.0f * M_PI * (i + 1) / circleSegments;

        vertices.push_back(cos(angle1) * circleRadius);
        vertices.push_back(-labelOffset + sin(angle1) * circleRadius);
        vertices.push_back(0.0f);
        vertices.push_back(cos(angle2) * circleRadius);
        vertices.push_back(-labelOffset + sin(angle2) * circleRadius);
        vertices.push_back(0.0f);
    }
    // Center dot for |-i>
    float dotSize = markerSize * 0.3f;
    vertices.push_back(-dotSize); vertices.push_back(-labelOffset - dotSize); vertices.push_back(0.0f);
    vertices.push_back(dotSize); vertices.push_back(-labelOffset + dotSize); vertices.push_back(0.0f);
    vertices.push_back(-dotSize); vertices.push_back(-labelOffset + dotSize); vertices.push_back(0.0f);
    vertices.push_back(dotSize); vertices.push_back(-labelOffset - dotSize); vertices.push_back(0.0f);

    // Z-axis: |0> and |1> (Squares)
    // |0> at (0, 0, labelOffset) - Square
    float squareSize = markerSize;
    vertices.push_back(-squareSize); vertices.push_back(-squareSize); vertices.push_back(labelOffset);
    vertices.push_back(squareSize); vertices.push_back(-squareSize); vertices.push_back(labelOffset);
    vertices.push_back(squareSize); vertices.push_back(-squareSize); vertices.push_back(labelOffset);
    vertices.push_back(squareSize); vertices.push_back(squareSize); vertices.push_back(labelOffset);
    vertices.push_back(squareSize); vertices.push_back(squareSize); vertices.push_back(labelOffset);
    vertices.push_back(-squareSize); vertices.push_back(squareSize); vertices.push_back(labelOffset);
    vertices.push_back(-squareSize); vertices.push_back(squareSize); vertices.push_back(labelOffset);
    vertices.push_back(-squareSize); vertices.push_back(-squareSize); vertices.push_back(labelOffset);

    // |1> at (0, 0, -labelOffset) - Vertical line
    vertices.push_back(0.0f); vertices.push_back(-squareSize); vertices.push_back(-labelOffset);
    vertices.push_back(0.0f); vertices.push_back(squareSize); vertices.push_back(-labelOffset);

    return vertices;
}

void CoordinateAxes::compileShaders() {
    unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, axes_vertex_shader_source);
    unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, axes_fragment_shader_source);

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertex_shader);
    glAttachShader(shaderProgram, fragment_shader);
    glLinkProgram(shaderProgram);

    int success;
    char log[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, log);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << log << std::endl;
    }
    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
}

void CoordinateAxes::createAxesGeometry() {
    std::vector<float> axesVerts = generateAxisVertices();

    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, axesVerts.size() * sizeof(float), axesVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void CoordinateAxes::createLabelsGeometry() {
    std::vector<float> labelVerts = generateLabelVertices();

    glGenVertexArrays(1, &labelsVAO);
    glGenBuffers(1, &labelsVBO);
    glBindVertexArray(labelsVAO);
    glBindBuffer(GL_ARRAY_BUFFER, labelsVBO);
    glBufferData(GL_ARRAY_BUFFER, labelVerts.size() * sizeof(float), labelVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void CoordinateAxes::createTextGeometry() {
    std::vector<float> textVerts = generateTextVertices();

    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, textVerts.size() * sizeof(float), textVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

CoordinateAxes::CoordinateAxes(float length, float thickness,
    glm::vec3 xAxisColor, glm::vec3 yAxisColor, glm::vec3 zAxisColor)
    : axisLength(length), axisThickness(thickness),
    xColor(xAxisColor), yColor(yAxisColor), zColor(zAxisColor) {

    compileShaders();
    createAxesGeometry();
    createLabelsGeometry();
    createTextGeometry();
}

CoordinateAxes::~CoordinateAxes() {
    cleanup();
}

void CoordinateAxes::cleanup() {
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &labelsVAO);
    glDeleteBuffers(1, &labelsVBO);
    glDeleteVertexArrays(1, &textVAO);
    glDeleteBuffers(1, &textVBO);
    glDeleteProgram(shaderProgram);
}

void CoordinateAxes::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    // Apply rotation
    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    finalModel = glm::rotate(finalModel, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    // Use medium line width for axes
    glLineWidth(2.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.7f);

    // Draw axes
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), xColor.r, xColor.g, xColor.b);
    glBindVertexArray(axesVAO);
    glDrawArrays(GL_LINES, 0, 2);  // X-axis
    glDrawArrays(GL_LINES, 2, 2);  // Y-axis  
    glDrawArrays(GL_LINES, 4, 2);  // Z-axis

    // Draw arrow labels
    glLineWidth(1.5f);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.8f);
    glBindVertexArray(labelsVAO);
    glDrawArrays(GL_LINES, 0, 12); // Draw all arrowheads

    // Draw quantum state labels with brighter color
    glLineWidth(2.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.9f);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.9f, 0.9f, 1.0f); // Bright cyan-white

    glBindVertexArray(textVAO);

    // Draw all label geometries
    // The text vertices contain different geometric patterns for each quantum state
    glDrawArrays(GL_LINES, 0, 100); // Draw all label geometries

    // Reset line width
    glLineWidth(1.0f);
}

void CoordinateAxes::setAxisLength(float length) {
    axisLength = length;
    cleanup();
    createAxesGeometry();
    createLabelsGeometry();
    createTextGeometry();
}

void CoordinateAxes::setAxisColors(const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor) {
    this->xColor = xColor;
    this->yColor = yColor;
    this->zColor = zColor;
}

float CoordinateAxes::getAxisLength() const {
    return axisLength;
}