#define _USE_MATH_DEFINES

#include "CartesianAxes.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader (same as BlochSphere)
const char* vertex_shader_source = R"(
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
const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    // Add a very subtle pulse to simulate CRT phosphor
    float pulse = sin(time * 2.0) * 0.05 + 0.95;
    vec3 glowColor = color * pulse;
    
    // Apply opacity
    FragColor = vec4(glowColor, opacity);
}
)";

// Compile shader
unsigned int CartesianAxes::compileShader(unsigned int type, const char* source) {
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

// Generate axes geometry (X, Y, Z axes with arrows)
std::vector<float> CartesianAxes::generateAxesGeometry() {
    std::vector<float> vertices;

    // X-axis (red) - main line
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);

    // Y-axis (green) - main line
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);

    // Z-axis (blue) - main line
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);

    // X-axis arrow
    float arrowSize = axisLength * 0.1f;
    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength - arrowSize); vertices.push_back(arrowSize * 0.5f); vertices.push_back(0.0f);

    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength - arrowSize); vertices.push_back(-arrowSize * 0.5f); vertices.push_back(0.0f);

    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength - arrowSize); vertices.push_back(0.0f); vertices.push_back(arrowSize * 0.5f);

    vertices.push_back(axisLength); vertices.push_back(0.0f); vertices.push_back(0.0f);
    vertices.push_back(axisLength - arrowSize); vertices.push_back(0.0f); vertices.push_back(-arrowSize * 0.5f);

    // Y-axis arrow
    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);
    vertices.push_back(arrowSize * 0.5f); vertices.push_back(axisLength - arrowSize); vertices.push_back(0.0f);

    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);
    vertices.push_back(-arrowSize * 0.5f); vertices.push_back(axisLength - arrowSize); vertices.push_back(0.0f);

    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(axisLength - arrowSize); vertices.push_back(arrowSize * 0.5f);

    vertices.push_back(0.0f); vertices.push_back(axisLength); vertices.push_back(0.0f);
    vertices.push_back(0.0f); vertices.push_back(axisLength - arrowSize); vertices.push_back(-arrowSize * 0.5f);

    // Z-axis arrow
    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);
    vertices.push_back(arrowSize * 0.5f); vertices.push_back(0.0f); vertices.push_back(axisLength - arrowSize);

    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);
    vertices.push_back(-arrowSize * 0.5f); vertices.push_back(0.0f); vertices.push_back(axisLength - arrowSize);

    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);
    vertices.push_back(0.0f); vertices.push_back(arrowSize * 0.5f); vertices.push_back(axisLength - arrowSize);

    vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(axisLength);
    vertices.push_back(0.0f); vertices.push_back(-arrowSize * 0.5f); vertices.push_back(axisLength - arrowSize);

    return vertices;
}

void CartesianAxes::compileShaders() {
    unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, vertex_shader_source);
    unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, fragment_shader_source);

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

void CartesianAxes::createAxesGeometry() {
    std::vector<float> axesVerts = generateAxesGeometry();
    glGenVertexArrays(1, &axesVAO);
    glGenBuffers(1, &axesVBO);
    glBindVertexArray(axesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, axesVBO);
    glBufferData(GL_ARRAY_BUFFER, axesVerts.size() * sizeof(float), axesVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

CartesianAxes::CartesianAxes(float length)
    : axisLength(length),
    xColor(1.0f, 0.2f, 0.2f),  // Red
    yColor(0.2f, 1.0f, 0.2f),  // Green  
    zColor(0.2f, 0.4f, 1.0f) { // Blue
    compileShaders();
    createAxesGeometry();
}

CartesianAxes::~CartesianAxes() {
    cleanup();
}

void CartesianAxes::cleanup() {
    glDeleteVertexArrays(1, &axesVAO);
    glDeleteBuffers(1, &axesVBO);
    glDeleteProgram(shaderProgram);
}

void CartesianAxes::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // Pass time for pulsing effect
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    // Apply rotation only around Z-axis
    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(static_cast<float>(yaw)), glm::vec3(0.0f, 0.0f, 1.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    // Set full opacity for axes
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 1.0f);

    glBindVertexArray(axesVAO);

    // Draw X-axis (red) - main line + arrow
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), xColor.r, xColor.g, xColor.b);
    glDrawArrays(GL_LINES, 0, 2);

    // Draw Y-axis (green) - main line + arrow  
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), yColor.r, yColor.g, yColor.b);
    glDrawArrays(GL_LINES, 2, 2);

    // Draw Z-axis (blue) - main line + arrow
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), zColor.r, zColor.g, zColor.b);
    glDrawArrays(GL_LINES, 4, 2);

    // Draw arrow heads
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), xColor.r, xColor.g, xColor.b);
    glDrawArrays(GL_LINES, 6, 8); // X-axis arrows

    glUniform3f(glGetUniformLocation(shaderProgram, "color"), yColor.r, yColor.g, yColor.b);
    glDrawArrays(GL_LINES, 14, 8); // Y-axis arrows

    glUniform3f(glGetUniformLocation(shaderProgram, "color"), zColor.r, zColor.g, zColor.b);
    glDrawArrays(GL_LINES, 22, 8); // Z-axis arrows
}

void CartesianAxes::setColors(const glm::vec3& xCol, const glm::vec3& yCol, const glm::vec3& zCol) {
    xColor = xCol;
    yColor = yCol;
    zColor = zCol;
}