#define _USE_MATH_DEFINES

#include "AngleArcs.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

// Vertex shader
static const char* arc_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

// Fragment shader for arcs
static const char* arc_fragment_shader_source = R"(
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

unsigned int AngleArcs::compileShader(unsigned int type, const char* source) {
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

std::vector<float> AngleArcs::generateThetaArc() {
    std::vector<float> vertices;

    // Calculate theta angle (angle between vector and Z-axis)
    float vectorLength = glm::length(vectorPosition);
    if (vectorLength < 0.001f) return vertices; // Avoid division by zero

    float theta = acos(vectorPosition.z / vectorLength);

    // Only draw arc if theta is meaningful (not 0 or PI)
    if (theta < 0.01f || theta > M_PI - 0.01f) {
        return vertices;
    }

    // Calculate the plane normal (perpendicular to both vector and Z-axis)
    glm::vec3 zAxis(0.0f, 0.0f, 1.0f);
    glm::vec3 planeNormal = glm::cross(zAxis, vectorPosition);

    // If vector is aligned with Z-axis, use a default plane
    if (glm::length(planeNormal) < 0.001f) {
        planeNormal = glm::vec3(0.0f, 1.0f, 0.0f); // Use Y-axis as normal
    }
    else {
        planeNormal = glm::normalize(planeNormal);
    }

    // Create basis vectors for the arc plane
    glm::vec3 u = glm::normalize(glm::cross(planeNormal, zAxis));
    glm::vec3 v = zAxis;

    // Generate arc points in the correct plane
    for (int i = 0; i <= arcSegments; ++i) {
        float angle = static_cast<float>(i) / arcSegments * theta;
        float x = arcRadius * sin(angle);
        float z = arcRadius * cos(angle);

        // Transform from local arc coordinates to world coordinates
        glm::vec3 point = u * x + v * z;
        vertices.push_back(point.x);
        vertices.push_back(point.y);
        vertices.push_back(point.z);
    }

    return vertices;
}

std::vector<float> AngleArcs::generatePhiArc() {
    std::vector<float> vertices;

    // Calculate phi angle (angle between projection and positive X-axis)
    // Project vector onto XY plane
    glm::vec3 projection = glm::vec3(vectorPosition.x, vectorPosition.y, 0.0f);
    float projLength = glm::length(projection);

    if (projLength < 0.001f) {
        return vertices; // No meaningful phi angle if projection is near zero
    }

    float phi = atan2(vectorPosition.y, vectorPosition.x);

    // Only draw arc if phi is meaningful
    if (abs(phi) < 0.01f) {
        return vertices;
    }

    // Arc in the XY plane (disk plane)
    float startAngle = 0.0f; // Start from positive X-axis
    float endAngle = phi;

    // Ensure we take the shorter arc
    if (abs(endAngle - startAngle) > M_PI) {
        if (endAngle > startAngle) {
            endAngle -= 2.0f * M_PI;
        }
        else {
            endAngle += 2.0f * M_PI;
        }
    }

    // Generate arc points
    for (int i = 0; i <= arcSegments; ++i) {
        float angle = startAngle + static_cast<float>(i) / arcSegments * (endAngle - startAngle);
        float x = arcRadius * cos(angle);
        float y = arcRadius * sin(angle);
        vertices.push_back(x);
        vertices.push_back(y);
        vertices.push_back(0.0f);
    }

    return vertices;
}

void AngleArcs::compileShaders() {
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

void AngleArcs::createThetaArcGeometry() {
    std::vector<float> thetaVerts = generateThetaArc();

    glGenVertexArrays(1, &thetaArcVAO);
    glGenBuffers(1, &thetaArcVBO);
    glBindVertexArray(thetaArcVAO);
    glBindBuffer(GL_ARRAY_BUFFER, thetaArcVBO);
    glBufferData(GL_ARRAY_BUFFER, thetaVerts.size() * sizeof(float), thetaVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

void AngleArcs::createPhiArcGeometry() {
    std::vector<float> phiVerts = generatePhiArc();

    glGenVertexArrays(1, &phiArcVAO);
    glGenBuffers(1, &phiArcVBO);
    glBindVertexArray(phiArcVAO);
    glBindBuffer(GL_ARRAY_BUFFER, phiArcVBO);
    glBufferData(GL_ARRAY_BUFFER, phiVerts.size() * sizeof(float), phiVerts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
}

AngleArcs::AngleArcs(const glm::vec3& position, const glm::vec3& arcColor,
    float radius, int segments)
    : vectorPosition(position), color(arcColor), arcRadius(radius), arcSegments(segments) {

    compileShaders();
    createThetaArcGeometry();
    createPhiArcGeometry();
}

AngleArcs::~AngleArcs() {
    cleanup();
}

void AngleArcs::cleanup() {
    glDeleteVertexArrays(1, &thetaArcVAO);
    glDeleteBuffers(1, &thetaArcVBO);
    glDeleteVertexArrays(1, &phiArcVAO);
    glDeleteBuffers(1, &phiArcVBO);
    glDeleteProgram(shaderProgram);
}

void AngleArcs::updatePosition(const glm::vec3& newPosition) {
    vectorPosition = newPosition;

    // Recreate geometry with new position
    cleanup();
    createThetaArcGeometry();
    createPhiArcGeometry();
}

void AngleArcs::render(float time, const glm::mat4& view, const glm::mat4& projection,
    const glm::mat4& model, float yaw, float pitch) {
    glUseProgram(shaderProgram);
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniform1f(glGetUniformLocation(shaderProgram, "time"), time);

    // Set color and opacity for arcs
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(shaderProgram, "opacity"), 0.8f);

    // Use medium line width for arcs
    glLineWidth(1.8f);

    // Apply rotation for all arcs
    glm::mat4 finalModel = model;
    finalModel = glm::rotate(finalModel, glm::radians(yaw), glm::vec3(0.0f, 0.0f, 1.0f));
    finalModel = glm::rotate(finalModel, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(finalModel));

    // Render theta arc (angle with Z-axis)
    glBindVertexArray(thetaArcVAO);
    int thetaVertexCount = generateThetaArc().size() / 3;
    if (thetaVertexCount > 1) {
        glDrawArrays(GL_LINE_STRIP, 0, thetaVertexCount);
    }

    // Render phi arc (angle with X-axis in XY plane)
    glBindVertexArray(phiArcVAO);
    int phiVertexCount = generatePhiArc().size() / 3;
    if (phiVertexCount > 1) {
        glDrawArrays(GL_LINE_STRIP, 0, phiVertexCount);
    }

    // Reset line width
    glLineWidth(1.0f);
}

void AngleArcs::setColor(const glm::vec3& newColor) {
    color = newColor;
}

glm::vec3 AngleArcs::getColor() const {
    return color;
}