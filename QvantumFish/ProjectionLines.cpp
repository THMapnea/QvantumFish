#define _USE_MATH_DEFINES

#include "ProjectionLines.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader
static const char* projection_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader for dashed lines
static const char* projection_fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    vec3 glowColor = color;
    // Subtle pulse effect
    float pulse = sin(time * 2.0) * 0.05 + 0.95;
    glowColor *= pulse;
    FragColor = vec4(glowColor, opacity);
}
)";

unsigned int ProjectionLines::compileShader(unsigned int type, const char* source) {
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

std::vector<float> ProjectionLines::generateVerticalDashedLine() {
    std::vector<float> vertices;

    // Vertical line from vector tip down to disk (z=0 plane)
    glm::vec3 start = vectorPosition;
    glm::vec3 end = glm::vec3(vectorPosition.x, vectorPosition.y, 0.0f);

    float totalLength = glm::length(end - start);
    int totalDashes = static_cast<int>(totalLength / dashLength);

    // Create individual dash segments (each dash is a separate line)
    for (int i = 0; i < totalDashes; i += 2) { // Skip every other dash to create gaps
        float t1 = static_cast<float>(i) / totalDashes;
        float t2 = static_cast<float>(i + 1) / totalDashes;

        if (t2 > 1.0f) t2 = 1.0f;

        glm::vec3 point1 = start + t1 * (end - start);
        glm::vec3 point2 = start + t2 * (end - start);

        // Only add vertices if this is a visible dash (not a gap)
        vertices.push_back(point1.x);
        vertices.push_back(point1.y);
        vertices.push_back(point1.z);

        vertices.push_back(point2.x);
        vertices.push_back(point2.y);
        vertices.push_back(point2.z);
    }

    return vertices;
}

std::vector<float> ProjectionLines::generateHorizontalDashedLine() {
    std::vector<float> vertices;

    // Horizontal line from origin to the projection point on disk
    glm::vec3 start = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 end = glm::vec3(vectorPosition.x, vectorPosition.y, 0.0f);

    float totalLength = glm::length(end - start);
    int totalDashes = static_cast<int>(totalLength / dashLength);

    // Create individual dash segments (each dash is a separate line)
    for (int i = 0; i < totalDashes; i += 2) { // Skip every other dash to create gaps
        float t1 = static_cast<float>(i) / totalDashes;
        float t2 = static_cast<float>(i + 1) / totalDashes;

        if (t2 > 1.0f) t2 = 1.0f;

        glm::vec3 point1 = start + t1 * (end - start);
        glm::vec3 point2 = start + t2 * (end - start);

        // Only add vertices if this is a visible dash (not a gap)
        vertices.push_back(point1.x);
        vertices.push_back(point1.y);
        vertices.push_back(point1.z);

        vertices.push_back(point2.x);
        vertices.push_back(point2.y);
        vertices.push_back(point2.z);
    }

    return vertices;
}

void ProjectionLines::compileShaders() {
    unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, projection_vertex_shader_source);
    unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, projection_fragment_shader_source);

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

void ProjectionLines::createVerticalLineGeometry() {
    std::vector<float> vertLineVerts = generateVerticalDashedLine();

    glGenVertexArrays(1, &verticalLineVAO);
    glGenBuffers(1, &verticalLineVBO);
    glBindVertexArray(verticalLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, verticalLineVBO);
    glBufferData(GL_ARRAY_BUFFER, vertLineVerts.size() * sizeof(float), vertLineVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void ProjectionLines::createHorizontalLineGeometry() {
    std::vector<float> horzLineVerts = generateHorizontalDashedLine();

    glGenVertexArrays(1, &horizontalLineVAO);
    glGenBuffers(1, &horizontalLineVBO);
    glBindVertexArray(horizontalLineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, horizontalLineVBO);
    glBufferData(GL_ARRAY_BUFFER, horzLineVerts.size() * sizeof(float), horzLineVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

ProjectionLines::ProjectionLines(const glm::vec3& position, const glm::vec3& lineColor,
    float dashSize, int lineSegments)
    : vectorPosition(position), color(lineColor), dashLength(dashSize), segments(lineSegments) {

    compileShaders();
    createVerticalLineGeometry();
    createHorizontalLineGeometry();
}

ProjectionLines::~ProjectionLines() {
    cleanup();
}

void ProjectionLines::cleanup() {
    glDeleteVertexArrays(1, &verticalLineVAO);
    glDeleteBuffers(1, &verticalLineVBO);
    glDeleteVertexArrays(1, &horizontalLineVAO);
    glDeleteBuffers(1, &horizontalLineVBO);
    glDeleteProgram(shaderProgram);
}

void ProjectionLines::updatePosition(const glm::vec3& newPosition) {
    vectorPosition = newPosition;

    // Recreate geometry with new position
    cleanup();
    createVerticalLineGeometry();
    createHorizontalLineGeometry();
}

void ProjectionLines::render(float time, const glm::mat4& view, const glm::mat4& projection,
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

    // Set color and opacity for projection lines
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.6f);

    // Use thinner lines for dashed effect
    glLineWidth(1.6f);

    // Draw vertical dashed line (from vector tip to disk)
    // Each dash is drawn as a separate line segment
    glBindVertexArray(verticalLineVAO);
    int verticalDashCount = static_cast<int>(glm::length(glm::vec3(vectorPosition.x, vectorPosition.y, 0.0f) - vectorPosition) / dashLength) / 2;
    for (int i = 0; i < verticalDashCount; i++) {
        glDrawArrays(GL_LINES, i * 2, 2);
    }

    // Draw horizontal dashed line (from origin to projection point)
    glBindVertexArray(horizontalLineVAO);
    int horizontalDashCount = static_cast<int>(glm::length(glm::vec3(vectorPosition.x, vectorPosition.y, 0.0f)) / dashLength) / 2;
    for (int i = 0; i < horizontalDashCount; i++) {
        glDrawArrays(GL_LINES, i * 2, 2);
    }

    // Reset line width
    glLineWidth(1.0f);
}

void ProjectionLines::setColor(const glm::vec3& newColor) {
    color = newColor;
}

glm::vec3 ProjectionLines::getColor() const {
    return color;
}