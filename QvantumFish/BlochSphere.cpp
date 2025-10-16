#define _USE_MATH_DEFINES

#include "BlochSphere.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader
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

// Fragment shader with glow effect and opacity
const char* fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    // Cyan color with subtle pulsing effect
    vec3 glowColor = color;
    
    // Add a very subtle pulse to simulate CRT phosphor
    float pulse = sin(time * 2.0) * 0.05 + 0.95;
    glowColor *= pulse;
    
    // Apply opacity for depth effect
    FragColor = vec4(glowColor, opacity);
}
)";

// Compile shader
unsigned int BlochSphere::compileShader(unsigned int type, const char* source) {
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

// Generate sphere with only 4 longitudinal lines
std::vector<float> BlochSphere::generateMinimalSphere(float radius, int slices, int stacks) {
    std::vector<float> vertices;

    // Generate only 4 longitudinal lines (vertical)
    int selectedLongitudes[] = { 0, 8, 16, 24 }; // Evenly spaced around the sphere
    for (int k = 0; k < 4; ++k) {
        int i = selectedLongitudes[k];
        float lng = 2 * M_PI * i / slices;
        for (int j = 0; j <= stacks; ++j) {
            float lat = M_PI * (-0.5 + (float)j / stacks);
            float x = radius * cos(lat) * cos(lng);
            float y = radius * cos(lat) * sin(lng);
            float z = radius * sin(lat);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        }
    }

    // Generate latitude lines (horizontal) for sphere definition
    for (int j = 0; j <= stacks; ++j) {
        float lat = M_PI * (-0.5 + (float)j / stacks);
        float z = radius * sin(lat);
        float r = radius * cos(lat);
        for (int i = 0; i <= slices; ++i) {
            float lng = 2 * M_PI * i / slices;
            float x = r * cos(lng);
            float y = r * sin(lng);
            vertices.push_back(x); vertices.push_back(y); vertices.push_back(z);
        }
    }

    return vertices;
}

// Generate only the middle disc that slices through the sphere
std::vector<float> BlochSphere::generateMiddleDisc(float radius, int segments) {
    std::vector<float> vertices;

    // Generate only the middle disc at z = 0
    float z = 0.0f;
    float sliceRadius = sqrt(radius * radius - z * z); // Radius of the slice circle

    // Create triangle fan for filled disc
    // Center point
    vertices.push_back(0.0f);
    vertices.push_back(0.0f);
    vertices.push_back(z);

    // Outer points
    for (int i = 0; i <= segments; i++) {
        float angle = 2.0f * M_PI * i / segments;
        float x = sliceRadius * cos(angle);
        float y = sliceRadius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(z);
    }

    return vertices;
}

void BlochSphere::compileShaders() {
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

void BlochSphere::createSphereGeometry() {
    std::vector<float> sphereVerts = generateMinimalSphere(radius, slices, stacks);
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVerts.size() * sizeof(float), sphereVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void BlochSphere::createDiscGeometry() {
    std::vector<float> discVerts = generateMiddleDisc(radius, 64);
    glGenVertexArrays(1, &discVAO);
    glGenBuffers(1, &discVBO);
    glBindVertexArray(discVAO);
    glBindBuffer(GL_ARRAY_BUFFER, discVBO);
    glBufferData(GL_ARRAY_BUFFER, discVerts.size() * sizeof(float), discVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

BlochSphere::BlochSphere(float sphereRadius, int sphereSlices, int sphereStacks)
    : radius(sphereRadius), slices(sphereSlices), stacks(sphereStacks), color(0.2f, 0.8f, 1.0f) {

    // Calculate vertex counts
    verticesPerLongitude = stacks + 1;
    verticesPerLatitude = slices + 1;
    totalLongitudeVertices = 4 * verticesPerLongitude;
    totalLatitudeVertices = (stacks + 1) * verticesPerLatitude;
    verticesPerDisc = 66; // 64 segments + center + duplicate first vertex to close

    compileShaders();
    createSphereGeometry();
    createDiscGeometry();
}

BlochSphere::~BlochSphere() {
    cleanup();
}

void BlochSphere::cleanup() {
    glDeleteVertexArrays(1, &sphereVAO);
    glDeleteBuffers(1, &sphereVBO);
    glDeleteVertexArrays(1, &discVAO);
    glDeleteBuffers(1, &discVBO);
    glDeleteProgram(shaderProgram);
}

void BlochSphere::rebuild(float newRadius, int newSlices, int newStacks) {
    // Clean up existing resources
    cleanup();

    // Update parameters if provided
    if (newRadius > 0) radius = newRadius;
    if (newSlices > 0) slices = newSlices;
    if (newStacks > 0) stacks = newStacks;

    // Recalculate vertex counts
    verticesPerLongitude = stacks + 1;
    verticesPerLatitude = slices + 1;
    totalLongitudeVertices = 4 * verticesPerLongitude;
    totalLatitudeVertices = (stacks + 1) * verticesPerLatitude;

    // Recreate geometry
    compileShaders();
    createSphereGeometry();
    createDiscGeometry();
}

void BlochSphere::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // Pass time for pulsing effect
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    // Apply rotation
    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    finalModel = glm::rotate(finalModel, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    // Draw sphere lines with cyan color
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
    glBindVertexArray(sphereVAO);

    // Draw only 4 longitudinal lines (vertical) with full opacity
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 1.0f);
    for (int i = 0; i < 4; ++i) {
        glDrawArrays(GL_LINE_STRIP, i * verticesPerLongitude, verticesPerLongitude);
    }

    // Draw latitude lines (horizontal) with reduced opacity for depth
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.3f); // Semi-transparent
    for (int j = 0; j <= stacks; ++j) {
        glDrawArrays(GL_LINE_STRIP, totalLongitudeVertices + j * verticesPerLatitude, verticesPerLatitude);
    }

    // Draw the single middle disc as very transparent filled triangles
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), 0.15f, 0.6f, 0.8f); // Slightly different cyan
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.20f); // Very transparent (20% opacity)

    // Temporarily disable wireframe mode for disc
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glBindVertexArray(discVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, verticesPerDisc);
    // Re-enable wireframe mode for sphere
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
}

void BlochSphere::setColor(const glm::vec3& newColor) {
    color = newColor;
}

glm::vec3 BlochSphere::getColor() const {
    return color;
}