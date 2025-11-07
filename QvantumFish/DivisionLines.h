#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>

class DivisionLines {
public:
    DivisionLines();
    ~DivisionLines();

    bool initialize();
    void render(float time, int windowWidth, int windowHeight);
    void cleanup();

    void setColor(const glm::vec3& color) { m_color = color; }
    void setLineWidth(float width) { m_lineWidth = width; }
    void setPulseSpeed(float speed) { m_pulseSpeed = speed; }

private:
    unsigned int m_vao;
    unsigned int m_vbo;
    unsigned int m_shader;

    glm::vec3 m_color;
    float m_lineWidth;
    float m_pulseSpeed;

    bool compileShaders();
    void setupBuffers();
};