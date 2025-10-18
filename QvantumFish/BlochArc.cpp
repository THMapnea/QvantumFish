#define _USE_MATH_DEFINES

#include "BlochArc.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader (same as others for consistency)
const char* arc_vertex_shader_source = R"(
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
const char* arc_fragment_shader_source = R"(
#version 330 core
out vec4 FragColor;

uniform vec3 color;
uniform float time;
uniform float opacity;

void main(){
    // Yellow-orange color for the arc with pulsing effect
    vec3 glowColor = color;
    
    // Add a subtle pulse
    float pulse = sin(time * 3.0) * 0.1 + 0.9;
    glowColor *= pulse;
    
    // Apply opacity
    FragColor = vec4(glowColor, opacity);
}
)";

// Compile shader
unsigned int BlochArc::compileShader(unsigned int type, const char* source) {
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

// Generate arc vertices based on azimuthal and polar angles
std::vector<float> BlochArc::generateArcVertices(float radius, float azimuthal, float polar, int segments) {
    std::vector<float> vertices;

    // The arc starts from the north pole (|0> state) and goes to the point defined by the angles
    glm::vec3 startPoint(0.0f, 0.0f, radius); // North pole

    // Calculate the end point on Bloch sphere using spherical coordinates
    // x = sin(theta)cos(phi), y = sin(theta)sin(phi), z = cos(theta)
    glm::vec3 endPoint(
        radius * sin(polar) * cos(azimuthal),
        radius * sin(polar) * sin(azimuthal),
        radius * cos(polar)
    );

    // Generate points along the great circle arc
    for (int i = 0; i <= segments; ++i) {
        float t = static_cast<float>(i) / segments;

        // Spherical linear interpolation (slerp) along the great circle
        float omega = acos(glm::dot(glm::normalize(startPoint), glm::normalize(endPoint)));

        if (omega < 1e-6) {
            // Points are very close, use linear interpolation
            glm::vec3 point = startPoint + t * (endPoint - startPoint);
            vertices.push_back(point.x);
            vertices.push_back(point.y);
            vertices.push_back(point.z);
        }
        else {
            // Proper spherical interpolation
            float sinOmega = sin(omega);
            glm::vec3 point = (sin((1.0f - t) * omega) / sinOmega) * startPoint +
                (sin(t * omega) / sinOmega) * endPoint;
            vertices.push_back(point.x);
            vertices.push_back(point.y);
            vertices.push_back(point.z);
        }
    }

    return vertices;
}

void BlochArc::compileShaders() {
    unsigned int vertex_shader = compileShader(GL_VERTEX_SHADER, arc_vertex_shader_source);
    unsigned int fragment_shader = compileShader(GL_FRAGMENT_SHADER, arc_fragment_shader_source);

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

void BlochArc::createArcGeometry() {
    arcVertices = generateArcVertices(radius, azimuthalAngle, polarAngle, segments);

    glGenVertexArrays(1, &arcVAO);
    glGenBuffers(1, &arcVBO);
    glBindVertexArray(arcVAO);
    glBindBuffer(GL_ARRAY_BUFFER, arcVBO);
    glBufferData(GL_ARRAY_BUFFER, arcVertices.size() * sizeof(float), arcVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

BlochArc::BlochArc(float sphereRadius, float azimuthal, float polar, int arcSegments)
    : radius(sphereRadius), azimuthalAngle(azimuthal), polarAngle(polar),
    segments(arcSegments), color(1.0f, 0.8f, 0.2f) { // Yellow-orange color

    compileShaders();
    createArcGeometry();
}

BlochArc::~BlochArc() {
    cleanup();
}

void BlochArc::cleanup() {
    glDeleteVertexArrays(1, &arcVAO);
    glDeleteBuffers(1, &arcVBO);
    glDeleteProgram(shaderProgram);
}

void BlochArc::rebuild(float newRadius, float newAzimuthal, float newPolar) {
    // Clean up existing resources
    cleanup();

    // Update parameters if provided
    if (newRadius > 0) radius = newRadius;
    if (newAzimuthal >= 0) azimuthalAngle = newAzimuthal;
    if (newPolar >= 0) polarAngle = newPolar;

    // Recreate geometry
    compileShaders();
    createArcGeometry();
}

void BlochArc::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));

    // Pass time for pulsing effect
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    // Apply rotation (consistent with Bloch sphere)
    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    finalModel = glm::rotate(finalModel, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    // Draw the arc with yellow-orange color and good visibility
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.9f); // Highly visible

    glBindVertexArray(arcVAO);

    // Use thicker line for the arc to make it stand out
    glLineWidth(3.0f);
    glDrawArrays(GL_LINE_STRIP, 0, segments + 1);
    glLineWidth(1.0f); // Reset to default
}

void BlochArc::setColor(const glm::vec3& newColor) {
    color = newColor;
}

void BlochArc::setAngles(float newAzimuthal, float newPolar) {
    azimuthalAngle = newAzimuthal;
    polarAngle = newPolar;
    rebuild(-1.0f, newAzimuthal, newPolar); // Rebuild with new angles
}

glm::vec3 BlochArc::getColor() const {
    return color;
}

float BlochArc::getAzimuthalAngle() const {
    return azimuthalAngle;
}

float BlochArc::getPolarAngle() const {
    return polarAngle;
}