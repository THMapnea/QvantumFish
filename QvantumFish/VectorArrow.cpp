#define _USE_MATH_DEFINES

#include "VectorArrow.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Shader sources devono essere definiti SOLO nel .cpp
static const char* vector_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

static const char* vector_fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    vec3 glowColor = color;
    float pulse = sin(time * 2.0) * 0.05 + 0.95;
    glowColor *= pulse;
    FragColor = vec4(glowColor, opacity);
}
)";

unsigned int VectorArrow::compileShader(unsigned int type, const char* source) {
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

std::vector<float> VectorArrow::generateLineVertices(const glm::vec3& start, const glm::vec3& end, int segments) {
    std::vector<float> vertices;
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;
        glm::vec3 point = start + t * (end - start);
        vertices.push_back(point.x);
        vertices.push_back(point.y);
        vertices.push_back(point.z);
    }
    return vertices;
}

std::vector<float> VectorArrow::generateConeVertices(float height, float baseRadius, int slices) {
    std::vector<float> vertices;

    // Tip of the cone (at origin)
    glm::vec3 tip(0.0f, 0.0f, 0.0f);

    // Base vertices and lines from tip to base
    for (int i = 0; i <= slices; ++i) {
        float theta = static_cast<float>(i) / slices * 2.0f * M_PI;
        float x = baseRadius * cos(theta);
        float y = baseRadius * sin(theta);
        float z = -height;

        // Add base vertex
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);

        // Add line from tip to base vertex
        vertices.push_back(tip.x);
        vertices.push_back(tip.y);
        vertices.push_back(tip.z);

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    // Add base circle
    for (int i = 0; i <= slices; ++i) {
        float theta = static_cast<float>(i) / slices * 2.0f * M_PI;
        float x = baseRadius * cos(theta);
        float y = baseRadius * sin(theta);
        float z = -height;

        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    return vertices;
}

void VectorArrow::compileShaders() {
    unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, vector_vertex_shader_source);
    unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, vector_fragment_shader_source);

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

void VectorArrow::createLineGeometry() {
    glm::vec3 start(0.0f, 0.0f, 0.0f);
    glm::vec3 end = position;

    std::vector<float> lineVerts = generateLineVertices(start, end, lineSegments);

    glGenVertexArrays(1, &lineVAO);
    glGenBuffers(1, &lineVBO);
    glBindVertexArray(lineVAO);
    glBindBuffer(GL_ARRAY_BUFFER, lineVBO);
    glBufferData(GL_ARRAY_BUFFER, lineVerts.size() * sizeof(float), lineVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void VectorArrow::createConeGeometry() {
    std::vector<float> coneVerts = generateConeVertices(coneHeight, coneBaseRadius, coneSlices);

    glGenVertexArrays(1, &coneVAO);
    glGenBuffers(1, &coneVBO);
    glBindVertexArray(coneVAO);
    glBindBuffer(GL_ARRAY_BUFFER, coneVBO);
    glBufferData(GL_ARRAY_BUFFER, coneVerts.size() * sizeof(float), coneVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

VectorArrow::VectorArrow(const glm::vec3& vecPosition, float vectorRadius,
    float arrowHeight, float arrowBaseRadius, int lineSegmentsCount,
    int coneSlicesCount)
    : position(vecPosition), radius(vectorRadius), coneHeight(arrowHeight),
    coneBaseRadius(arrowBaseRadius), lineSegments(lineSegmentsCount),
    coneSlices(coneSlicesCount), color(1.0f, 0.2f, 0.2f) {

    compileShaders();
    createLineGeometry();
    createConeGeometry();
}

VectorArrow::~VectorArrow() {
    cleanup();
}

void VectorArrow::cleanup() {
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &coneVAO);
    glDeleteBuffers(1, &coneVBO);
    glDeleteProgram(shaderProgram);
}

void VectorArrow::rebuild(const glm::vec3& newPosition) {
    cleanup();
    if (newPosition != glm::vec3(0.0f)) {
        position = newPosition;
    }
    compileShaders();
    createLineGeometry();
    createConeGeometry();
}

void VectorArrow::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    finalModel = glm::rotate(finalModel, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.8f);

    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINE_STRIP, 0, lineSegments + 1);

    // Draw the cone at the end of the vector
    glm::mat4 coneModel = finalModel;
    coneModel = glm::translate(coneModel, position);

    glm::vec3 direction = glm::normalize(position);
    glm::vec3 up(0.0f, 0.0f, 1.0f);

    if (glm::length(glm::cross(direction, up)) < 0.001f) {
        up = glm::vec3(0.0f, 1.0f, 0.0f);
    }

    glm::mat4 rotation = glm::inverse(glm::lookAt(glm::vec3(0.0f), direction, up));
    coneModel = coneModel * rotation;

    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(coneModel));
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 1.0f);

    glBindVertexArray(coneVAO);
    for (int i = 0; i <= coneSlices; ++i) {
        glDrawArrays(GL_LINES, i * 2, 2);
    }

    int baseStartIndex = (coneSlices + 1) * 2;
    glDrawArrays(GL_LINE_STRIP, baseStartIndex, coneSlices + 1);
}

void VectorArrow::setColor(const glm::vec3& newColor) {
    color = newColor;
}

void VectorArrow::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
    cleanup();
    createLineGeometry();
    createConeGeometry();
}

glm::vec3 VectorArrow::getColor() const {
    return color;
}

glm::vec3 VectorArrow::getPosition() const {
    return position;
}