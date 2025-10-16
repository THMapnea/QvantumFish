#define _USE_MATH_DEFINES

#include "VectorSphere.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader (same as BlochSphere)
const char* vector_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader with glow effect
const char* vector_fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    // Color with subtle pulsing effect
    vec3 glowColor = color;
    
    // Add a very subtle pulse
    float pulse = sin(time * 2.0) * 0.05 + 0.95;
    glowColor *= pulse;
    
    // Apply opacity
    FragColor = vec4(glowColor, opacity);
}
)";

// Compile shader
unsigned int VectorSphere::compileShader(unsigned int type, const char* source) {
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

// Generate line vertices from center to position
std::vector<float> VectorSphere::generateLineVertices(const glm::vec3& start, const glm::vec3& end, int segments) {
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

// Generate sphere vertices for the end point
std::vector<float> VectorSphere::generateSphereVertices(float radius, int slices, int stacks) {
    std::vector<float> vertices;

    for (int i = 0; i <= slices; ++i) {
        for (int j = 0; j <= stacks; ++j) {
            float theta = static_cast<float>(i) / slices * 2.0f * M_PI;
            float phi = static_cast<float>(j) / stacks * M_PI;

            float x = radius * sin(phi) * cos(theta);
            float y = radius * sin(phi) * sin(theta);
            float z = radius * cos(phi);

            vertices.push_back(x);
            vertices.push_back(y);
            vertices.push_back(z);
        }
    }

    return vertices;
}

void VectorSphere::compileShaders() {
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

void VectorSphere::createLineGeometry() {
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

void VectorSphere::createSphereGeometry() {
    std::vector<float> sphereVerts = generateSphereVertices(sphereRadius, sphereSlices, sphereStacks);

    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

VectorSphere::VectorSphere(const glm::vec3& vecPosition, float vectorRadius,
    float endSphereRadius, int lineSegmentsCount,
    int sphereSlicesCount, int sphereStacksCount)
    : position(vecPosition), radius(vectorRadius), sphereRadius(endSphereRadius),
    lineSegments(lineSegmentsCount), sphereSlices(sphereSlicesCount),
    sphereStacks(sphereStacksCount), color(1.0f, 0.2f, 0.2f) {

    compileShaders();
    createLineGeometry();
    createSphereGeometry();
}

VectorSphere::~VectorSphere() {
    cleanup();
}

void VectorSphere::cleanup() {
    glDeleteVertexArrays(1, &lineVAO);
    glDeleteBuffers(1, &lineVBO);
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteProgram(shaderProgram);
}

void VectorSphere::rebuild(const glm::vec3& newPosition) {
    // Clean up existing resources
    cleanup();

    // Update position if provided
    if (newPosition != glm::vec3(0.0f)) {
        position = newPosition;
    }

    // Recreate geometry
    compileShaders();
    createLineGeometry();
    createSphereGeometry();
}

void VectorSphere::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // Pass time for pulsing effect
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    // Apply rotation (same as Bloch sphere for consistent viewing)
    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    finalModel = glm::rotate(finalModel, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    // Draw the vector line
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.8f);

    glBindVertexArray(lineVAO);
    glDrawArrays(GL_LINE_STRIP, 0, lineSegments + 1);

    // Draw the sphere at the end of the vector
    // Translate the sphere to the end position
    glm::mat4 sphereModel = glm::translate(finalModel, position);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(sphereModel));

    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 1.0f);
    glBindVertexArray(sphereVAO);

    // Draw longitude lines (vertical)
    for (int i = 0; i <= sphereSlices; ++i) {
        glDrawArrays(GL_LINE_STRIP, i * (sphereStacks + 1), sphereStacks + 1);
    }

    // Draw latitude lines (horizontal)
    for (int j = 0; j <= sphereStacks; ++j) {
        glDrawArrays(GL_LINE_STRIP, (sphereSlices + 1) * (sphereStacks + 1) + j * (sphereSlices + 1), sphereSlices + 1);
    }
}
void VectorSphere::setColor(const glm::vec3& newColor) {
    color = newColor;
}

void VectorSphere::setPosition(const glm::vec3& newPosition) {
    position = newPosition;
    // Rebuild the line geometry with new position
    cleanup();
    createLineGeometry();
    createSphereGeometry();
}

glm::vec3 VectorSphere::getColor() const {
    return color;
}

glm::vec3 VectorSphere::getPosition() const {
    return position;
}