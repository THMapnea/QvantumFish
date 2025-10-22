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

    // X-axis (Red) - from (-length, 0, 0) to (length, 0, 0)
    vertices.push_back(-axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);

    // Y-axis (Green) - from (0, -length, 0) to (0, length, 0)
    vertices.push_back(0.0f); vertices.push_back(-axisLength); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);

    // Z-axis (Blue) - from (0, 0, -length) to (0, 0, length)
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(-axisLength);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);

    return vertices;
}

std::vector<float> CoordinateAxes::generateLabelVertices() {
    std::vector<float> vertices;
    float labelOffset = axisLength * 1.1f;
    float arrowSize = axisLength * 0.08f;

    // X-axis arrow (small triangle at the end of X-axis)
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

CoordinateAxes::CoordinateAxes(float length, float thickness,
    glm::vec3 xAxisColor, glm::vec3 yAxisColor, glm::vec3 zAxisColor)
    : axisLength(length), axisThickness(thickness),
    xColor(xAxisColor), yColor(yAxisColor), zColor(zAxisColor) {

    compileShaders();
    createAxesGeometry();
    createLabelsGeometry();
}

CoordinateAxes::~CoordinateAxes() {
    cleanup();
}

void CoordinateAxes::cleanup() {
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteVertexArrays(1, &labelsVAO);
    glDeleteBuffers(1, &labelsVBO);
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

    // Use medium line width for axes (thinner than vector)
    glLineWidth(2.0f);

    // Lower opacity for axes to make them subtle background elements
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.7f);

    // Draw all axes with the same color
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), xColor.r, xColor.g, xColor.b);
    glBindVertexArray(axesVAO);

    // Draw X, Y, Z axes
    glDrawArrays(GL_LINES, 0, 2);  // X-axis
    glDrawArrays(GL_LINES, 2, 2);  // Y-axis  
    glDrawArrays(GL_LINES, 4, 2);  // Z-axis

    // Draw arrow labels with thinner lines
    glLineWidth(1.5f);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.8f);

    glBindVertexArray(labelsVAO);

    // Draw all arrowheads with same color
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), xColor.r, xColor.g, xColor.b);
    glDrawArrays(GL_LINES, 0, 12); // Draw all arrowheads at once

    // Reset line width to default
    glLineWidth(1.0f);
}
void CoordinateAxes::setAxisLength(float length) {
    axisLength = length;
    cleanup();
    createAxesGeometry();
    createLabelsGeometry();
}

void CoordinateAxes::setAxisColors(const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor) {
    this->xColor = xColor;
    this->yColor = yColor;
    this->zColor = zColor;
}

float CoordinateAxes::getAxisLength() const {
    return axisLength;
}