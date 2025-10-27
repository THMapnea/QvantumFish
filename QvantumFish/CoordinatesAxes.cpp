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

    float labelOffset = axisLength * 1.15f;
    float markerSize = axisLength * 0.025f;
    float lineSize = markerSize * 2.0f;
    float spacing = markerSize * 1.5f;
    int circleSegments = 16;
    float circleRadius = markerSize * 1.2f;

    // ⟨0| at NORTH POLE - positioned in XZ plane (Y=0) so it faces front
    float centerX_N = 0.0f;
    float centerY_N = 0.0f;
    float centerZ_N = labelOffset;

    // ⟨1| at SOUTH POLE - positioned in XZ plane (Y=0)
    float centerX_S = 0.0f;
    float centerY_S = 0.0f;
    float centerZ_S = -labelOffset;

    // |+> at POSITIVE X-AXIS
    float centerX_PX = labelOffset;
    float centerY_PX = 0.0f;
    float centerZ_PX = 0.0f;

    // |-> at NEGATIVE X-AXIS  
    float centerX_NX = -labelOffset;
    float centerY_NX = 0.0f;
    float centerZ_NX = 0.0f;

    // |+i> at POSITIVE Y-AXIS
    float centerX_PY = 0.0f;
    float centerY_PY = labelOffset;
    float centerZ_PY = 0.0f;

    // |-i> at NEGATIVE Y-AXIS
    float centerX_NY = 0.0f;
    float centerY_NY = -labelOffset;
    float centerZ_NY = 0.0f;

    // NORTH POLE: ⟨0|
    // 1. LEFT ANGLE BRACKET ⟨
    float bracketLeftX_N = centerX_N - spacing - lineSize;
    vertices.push_back(bracketLeftX_N + markerSize); vertices.push_back(centerY_N); vertices.push_back(centerZ_N + lineSize);
    vertices.push_back(bracketLeftX_N); vertices.push_back(centerY_N); vertices.push_back(centerZ_N);
    vertices.push_back(bracketLeftX_N); vertices.push_back(centerY_N); vertices.push_back(centerZ_N);
    vertices.push_back(bracketLeftX_N + markerSize); vertices.push_back(centerY_N); vertices.push_back(centerZ_N - lineSize);

    // 2. ZERO 0 - Circle (in XZ plane)
    for (int i = 0; i < circleSegments; i++) {
        float angle1 = 2.0f * M_PI * i / circleSegments;
        float angle2 = 2.0f * M_PI * (i + 1) / circleSegments;
        vertices.push_back(centerX_N + cos(angle1) * circleRadius);
        vertices.push_back(centerY_N);
        vertices.push_back(centerZ_N + sin(angle1) * circleRadius);
        vertices.push_back(centerX_N + cos(angle2) * circleRadius);
        vertices.push_back(centerY_N);
        vertices.push_back(centerZ_N + sin(angle2) * circleRadius);
    }

    // 3. RIGHT VERTICAL BAR |
    float barX_N = centerX_N + spacing + lineSize;
    vertices.push_back(barX_N); vertices.push_back(centerY_N); vertices.push_back(centerZ_N - lineSize);
    vertices.push_back(barX_N); vertices.push_back(centerY_N); vertices.push_back(centerZ_N + lineSize);

    // SOUTH POLE: ⟨1|
    // 1. LEFT ANGLE BRACKET ⟨
    float bracketLeftX_S = centerX_S - spacing - lineSize;
    vertices.push_back(bracketLeftX_S + markerSize); vertices.push_back(centerY_S); vertices.push_back(centerZ_S + lineSize);
    vertices.push_back(bracketLeftX_S); vertices.push_back(centerY_S); vertices.push_back(centerZ_S);
    vertices.push_back(bracketLeftX_S); vertices.push_back(centerY_S); vertices.push_back(centerZ_S);
    vertices.push_back(bracketLeftX_S + markerSize); vertices.push_back(centerY_S); vertices.push_back(centerZ_S - lineSize);

    // 2. ONE 1 - Vertical line
    vertices.push_back(centerX_S); vertices.push_back(centerY_S); vertices.push_back(centerZ_S - lineSize);
    vertices.push_back(centerX_S); vertices.push_back(centerY_S); vertices.push_back(centerZ_S + lineSize);

    // 3. RIGHT VERTICAL BAR |
    float barX_S = centerX_S + spacing + lineSize;
    vertices.push_back(barX_S); vertices.push_back(centerY_S); vertices.push_back(centerZ_S - lineSize);
    vertices.push_back(barX_S); vertices.push_back(centerY_S); vertices.push_back(centerZ_S + lineSize);

    // POSITIVE X-AXIS: |+>
    // 1. LEFT VERTICAL BAR |
    float barX_PX = centerX_PX - spacing - lineSize;
    vertices.push_back(barX_PX); vertices.push_back(centerY_PX - lineSize); vertices.push_back(centerZ_PX);
    vertices.push_back(barX_PX); vertices.push_back(centerY_PX + lineSize); vertices.push_back(centerZ_PX);

    // 2. PLUS + - Horizontal and vertical lines
    vertices.push_back(centerX_PX - markerSize); vertices.push_back(centerY_PX); vertices.push_back(centerZ_PX);
    vertices.push_back(centerX_PX + markerSize); vertices.push_back(centerY_PX); vertices.push_back(centerZ_PX);
    vertices.push_back(centerX_PX); vertices.push_back(centerY_PX - markerSize); vertices.push_back(centerZ_PX);
    vertices.push_back(centerX_PX); vertices.push_back(centerY_PX + markerSize); vertices.push_back(centerZ_PX);

    // 3. RIGHT ANGLE BRACKET >
    float bracketX_PX = centerX_PX + spacing + lineSize;
    vertices.push_back(bracketX_PX - markerSize); vertices.push_back(centerY_PX + lineSize); vertices.push_back(centerZ_PX);
    vertices.push_back(bracketX_PX); vertices.push_back(centerY_PX); vertices.push_back(centerZ_PX);
    vertices.push_back(bracketX_PX); vertices.push_back(centerY_PX); vertices.push_back(centerZ_PX);
    vertices.push_back(bracketX_PX - markerSize); vertices.push_back(centerY_PX - lineSize); vertices.push_back(centerZ_PX);

    // NEGATIVE X-AXIS: |->
    // 1. LEFT VERTICAL BAR |
    float barX_NX = centerX_NX - spacing - lineSize;
    vertices.push_back(barX_NX); vertices.push_back(centerY_NX - lineSize); vertices.push_back(centerZ_NX);
    vertices.push_back(barX_NX); vertices.push_back(centerY_NX + lineSize); vertices.push_back(centerZ_NX);

    // 2. MINUS - - Horizontal line
    vertices.push_back(centerX_NX - markerSize); vertices.push_back(centerY_NX); vertices.push_back(centerZ_NX);
    vertices.push_back(centerX_NX + markerSize); vertices.push_back(centerY_NX); vertices.push_back(centerZ_NX);

    // 3. RIGHT ANGLE BRACKET >
    float bracketX_NX = centerX_NX + spacing + lineSize;
    vertices.push_back(bracketX_NX - markerSize); vertices.push_back(centerY_NX + lineSize); vertices.push_back(centerZ_NX);
    vertices.push_back(bracketX_NX); vertices.push_back(centerY_NX); vertices.push_back(centerZ_NX);
    vertices.push_back(bracketX_NX); vertices.push_back(centerY_NX); vertices.push_back(centerZ_NX);
    vertices.push_back(bracketX_NX - markerSize); vertices.push_back(centerY_NX - lineSize); vertices.push_back(centerZ_NX);

    // POSITIVE Y-AXIS: |+i>
    // 1. LEFT VERTICAL BAR |
    float barX_PY = centerX_PY - spacing - lineSize;
    vertices.push_back(barX_PY); vertices.push_back(centerY_PY - lineSize); vertices.push_back(centerZ_PY);
    vertices.push_back(barX_PY); vertices.push_back(centerY_PY + lineSize); vertices.push_back(centerZ_PY);

    // 2. PLUS + - Horizontal and vertical lines
    vertices.push_back(centerX_PY - markerSize); vertices.push_back(centerY_PY); vertices.push_back(centerZ_PY);
    vertices.push_back(centerX_PY + markerSize); vertices.push_back(centerY_PY); vertices.push_back(centerZ_PY);
    vertices.push_back(centerX_PY); vertices.push_back(centerY_PY - markerSize); vertices.push_back(centerZ_PY);
    vertices.push_back(centerX_PY); vertices.push_back(centerY_PY + markerSize); vertices.push_back(centerZ_PY);

    // 3. i character - Dot and vertical line
    // Move i to the right of the plus sign
    float iOffset = markerSize * 2.5f;

    // Dot - make it a small horizontal line instead of a point
    float dotSize = markerSize * 0.3f;
    vertices.push_back(centerX_PY + iOffset - dotSize); vertices.push_back(centerY_PY + markerSize * 1.0f); vertices.push_back(centerZ_PY);
    vertices.push_back(centerX_PY + iOffset + dotSize); vertices.push_back(centerY_PY + markerSize * 1.0f); vertices.push_back(centerZ_PY);

    // Vertical line - position it lower so it doesn't touch the dot
    vertices.push_back(centerX_PY + iOffset); vertices.push_back(centerY_PY - markerSize * 0.8f); vertices.push_back(centerZ_PY);
    vertices.push_back(centerX_PY + iOffset); vertices.push_back(centerY_PY + markerSize * 0.5f); vertices.push_back(centerZ_PY);

    // 4. RIGHT ANGLE BRACKET >
    float bracketX_PY = centerX_PY + spacing + lineSize + iOffset * 0.5f;
    vertices.push_back(bracketX_PY - markerSize); vertices.push_back(centerY_PY + lineSize); vertices.push_back(centerZ_PY);
    vertices.push_back(bracketX_PY); vertices.push_back(centerY_PY); vertices.push_back(centerZ_PY);
    vertices.push_back(bracketX_PY); vertices.push_back(centerY_PY); vertices.push_back(centerZ_PY);
    vertices.push_back(bracketX_PY - markerSize); vertices.push_back(centerY_PY - lineSize); vertices.push_back(centerZ_PY);

    // NEGATIVE Y-AXIS: |-i>
    // 1. LEFT VERTICAL BAR |
    float barX_NY = centerX_NY - spacing - lineSize;
    vertices.push_back(barX_NY); vertices.push_back(centerY_NY - lineSize); vertices.push_back(centerZ_NY);
    vertices.push_back(barX_NY); vertices.push_back(centerY_NY + lineSize); vertices.push_back(centerZ_NY);

    // 2. MINUS - - Horizontal line
    vertices.push_back(centerX_NY - markerSize); vertices.push_back(centerY_NY); vertices.push_back(centerZ_NY);
    vertices.push_back(centerX_NY + markerSize); vertices.push_back(centerY_NY); vertices.push_back(centerZ_NY);

    // 3. i character - Dot and vertical line
    // Move i to the right of the minus sign
    float iOffset_NY = markerSize * 2.5f;

    // Dot - make it a small horizontal line instead of a point
    vertices.push_back(centerX_NY + iOffset_NY - dotSize); vertices.push_back(centerY_NY + markerSize * 1.0f); vertices.push_back(centerZ_NY);
    vertices.push_back(centerX_NY + iOffset_NY + dotSize); vertices.push_back(centerY_NY + markerSize * 1.0f); vertices.push_back(centerZ_NY);

    // Vertical line - position it lower so it doesn't touch the dot
    vertices.push_back(centerX_NY + iOffset_NY); vertices.push_back(centerY_NY - markerSize * 0.8f); vertices.push_back(centerZ_NY);
    vertices.push_back(centerX_NY + iOffset_NY); vertices.push_back(centerY_NY + markerSize * 0.5f); vertices.push_back(centerZ_NY);

    // 4. RIGHT ANGLE BRACKET >
    float bracketX_NY = centerX_NY + spacing + lineSize + iOffset_NY * 0.5f;
    vertices.push_back(bracketX_NY - markerSize); vertices.push_back(centerY_NY + lineSize); vertices.push_back(centerZ_NY);
    vertices.push_back(bracketX_NY); vertices.push_back(centerY_NY); vertices.push_back(centerZ_NY);
    vertices.push_back(bracketX_NY); vertices.push_back(centerY_NY); vertices.push_back(centerZ_NY);
    vertices.push_back(bracketX_NY - markerSize); vertices.push_back(centerY_NY - lineSize); vertices.push_back(centerZ_NY);

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

    // Draw all text labels with brighter color
    glLineWidth(2.0f);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.9f);
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.9f, 0.9f, 1.0f); // Bright cyan-white

    glBindVertexArray(textVAO);

    // Draw all text labels
    int totalVertices = 0;

    // |0> - 2 (|) + 32 (0 circle) + 4 (>) = 38 vertices
    glDrawArrays(GL_LINES, totalVertices, 38);
    totalVertices += 38;

    // |1> - 2 (|) + 2 (1) + 4 (>) = 8 vertices
    glDrawArrays(GL_LINES, totalVertices, 8);
    totalVertices += 8;

    // |+> - 2 (|) + 4 (+) + 4 (>) = 10 vertices  
    glDrawArrays(GL_LINES, totalVertices, 10);
    totalVertices += 10;

    // |-> - 2 (|) + 2 (-) + 4 (>) = 8 vertices
    glDrawArrays(GL_LINES, totalVertices, 8);
    totalVertices += 8;

    // |+i> - 2 (|) + 4 (+) + 6 (i) + 4 (>) = 16 vertices
    glDrawArrays(GL_LINES, totalVertices, 16);
    totalVertices += 16;

    // |-i> - 2 (|) + 2 (-) + 6 (i) + 4 (>) = 14 vertices
    glDrawArrays(GL_LINES, totalVertices, 14);

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