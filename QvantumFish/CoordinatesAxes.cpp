#define _USE_MATH_DEFINES

#include "CoordinatesAxes.h"
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <GLFW/glfw3.h>
#include <cmath>
#include <ft2build.h>
#include FT_FREETYPE_H

// Vertex shader for axes
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

// Fragment shader for axes
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

// Text rendering shaders
static const char* text_vertex_shader_source = R"(
#version 330 core
layout (location = 0) in vec4 vertex; // <vec2 pos, vec2 tex>
out vec2 TexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main() {
    gl_Position = projection * view * model * vec4(vertex.xy, 0.0, 1.0);
    TexCoords = vertex.zw;
}
)";

static const char* text_fragment_shader_source = R"(
#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform float time;

void main() {
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);
    
    // Add subtle pulse effect
    float pulse = sin(time * 1.5) * 0.1 + 0.9;
    color = vec4(textColor * pulse, sampled.a);
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
    // Compile axes shader
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

    // Compile text shader
    unsigned int text_vertex_shader = compileShader(GL_VERTEX_SHADER, text_vertex_shader_source);
    unsigned int text_fragment_shader = compileShader(GL_FRAGMENT_SHADER, text_fragment_shader_source);

    textShaderProgram = glCreateProgram();
    glAttachShader(textShaderProgram, text_vertex_shader);
    glAttachShader(textShaderProgram, text_fragment_shader);
    glLinkProgram(textShaderProgram);

    glDeleteShader(text_vertex_shader);
    glDeleteShader(text_fragment_shader);
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

void CoordinateAxes::initTextRendering() {
    // Configure VAO/VBO for texture quads
    glGenVertexArrays(1, &textVAO);
    glGenBuffers(1, &textVBO);
    glBindVertexArray(textVAO);
    glBindBuffer(GL_ARRAY_BUFFER, textVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Initialize FreeType
    FT_Library ft;
    if (FT_Init_FreeType(&ft)) {
        std::cout << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        return;
    }

    // Load font (you'll need to provide a .ttf font file)
    FT_Face face;
    if (FT_New_Face(ft, "fonts/arial.ttf", 0, &face)) {
        std::cout << "ERROR::FREETYPE: Failed to load font" << std::endl;
        // Fallback to basic geometry if font loading fails
        FT_Done_FreeType(ft);
        return;
    }

    // Set size to load glyphs as
    FT_Set_Pixel_Sizes(face, 0, 48);

    // Disable byte-alignment restriction
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load first 128 characters of ASCII set
    for (unsigned char c = 0; c < 128; c++) {
        // Load character glyph 
        if (FT_Load_Char(face, c, FT_LOAD_RENDER)) {
            std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
            continue;
        }

        // Generate texture
        unsigned int texture;
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            face->glyph->bitmap.width,
            face->glyph->bitmap.rows,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            face->glyph->bitmap.buffer
        );

        // Set texture options
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Now store character for later use
        Character character = {
            texture,
            glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
            glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
            static_cast<unsigned int>(face->glyph->advance.x)
        };
        Characters.insert(std::pair<char, Character>(c, character));
    }

    glBindTexture(GL_TEXTURE_2D, 0);

    // Destroy FreeType once we're finished
    FT_Done_Face(face);
    FT_Done_FreeType(ft);
}

void CoordinateAxes::renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color) {
    // Activate corresponding render state
    glUseProgram(textShaderProgram);
    glUniform3f(glGetUniformLocation(textShaderProgram, "textColor"), color.r, color.g, color.b);
    glUniform1f(glGetUniformLocation(textShaderProgram, "time"), static_cast<float>(glfwGetTime()));
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(textVAO);

    // Iterate through all characters
    std::string::const_iterator c;
    for (c = text.begin(); c != text.end(); c++) {
        Character ch = Characters[*c];

        float xpos = x + ch.Bearing.x * scale;
        float ypos = y - (ch.Size.y - ch.Bearing.y) * scale;

        float w = ch.Size.x * scale;
        float h = ch.Size.y * scale;

        // Update VBO for each character
        float vertices[6][4] = {
            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos,     ypos,       0.0f, 1.0f },
            { xpos + w, ypos,       1.0f, 1.0f },

            { xpos,     ypos + h,   0.0f, 0.0f },
            { xpos + w, ypos,       1.0f, 1.0f },
            { xpos + w, ypos + h,   1.0f, 0.0f }
        };

        // Render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, ch.TextureID);

        // Update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, textVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        // Render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);

        // Now advance cursors for next glyph (note that advance is number of 1/64 pixels)
        x += (ch.Advance >> 6) * scale; // Bitshift by 6 to get value in pixels (2^6 = 64)
    }

    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void CoordinateAxes::renderText3D(const std::string& text, const glm::vec3& position, float scale, const glm::vec3& color,
    const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model) {
    // For 3D text, we'll create a billboard effect
    // Calculate screen space position
    glm::vec4 screenPos = projection * view * model * glm::vec4(position, 1.0f);
    screenPos /= screenPos.w;

    // Convert to pixel coordinates
    float x = (screenPos.x + 1.0f) * 0.5f * windowWidth;
    float y = (1.0f - screenPos.y) * 0.5f * windowHeight;

    // Save current state
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);

    // Set up for 2D text rendering
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, windowWidth, windowHeight);

    // Create orthographic projection for text
    glm::mat4 textProj = glm::ortho(0.0f, static_cast<float>(windowWidth), 0.0f, static_cast<float>(windowHeight));

    // Render text in 2D
    renderText(text, x, y, scale, color);

    // Restore state
    if (prevDepthTest) glEnable(GL_DEPTH_TEST);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

CoordinateAxes::CoordinateAxes(float length, float thickness,
    glm::vec3 xAxisColor, glm::vec3 yAxisColor, glm::vec3 zAxisColor)
    : axisLength(length), axisThickness(thickness),
    xColor(xAxisColor), yColor(yAxisColor), zColor(zColor) {

    compileShaders();
    createAxesGeometry();
    createLabelsGeometry();
    initTextRendering();
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
    glDeleteProgram(textShaderProgram);

    // Delete character textures
    for (auto& character : Characters) {
        glDeleteTextures(1, &character.second.TextureID);
    }
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
    glUniform3f(glGetUniformLocation(shaderProgram, "color"), xColor.r, xColor.g, xColor.b);
    glDrawArrays(GL_LINES, 0, 12); // Draw all arrowheads at once

    // Reset line width
    glLineWidth(1.0f);

    // Render Bloch sphere quantum state labels
    float labelOffset = axisLength * 1.15f;
    float labelScale = 0.5f; // Adjust text size
    glm::vec3 labelColor = glm::vec3(0.9f, 0.9f, 1.0f); // Bright cyan-white

    // X-axis labels
    renderText3D("|+>", glm::vec3(labelOffset, 0.0f, 0.0f), labelScale, labelColor, view, projection, finalModel);
    renderText3D("|->", glm::vec3(-labelOffset, 0.0f, 0.0f), labelScale, labelColor, view, projection, finalModel);

    // Y-axis labels  
    renderText3D("|+i>", glm::vec3(0.0f, labelOffset, 0.0f), labelScale, labelColor, view, projection, finalModel);
    renderText3D("|-i>", glm::vec3(0.0f, -labelOffset, 0.0f), labelScale, labelColor, view, projection, finalModel);

    // Z-axis labels
    renderText3D("|0>", glm::vec3(0.0f, 0.0f, labelOffset), labelScale, labelColor, view, projection, finalModel);
    renderText3D("|1>", glm::vec3(0.0f, 0.0f, -labelOffset), labelScale, labelColor, view, projection, finalModel);
}

void CoordinateAxes::setAxisLength(float length) {
    axisLength = length;
    cleanup();
    createAxesGeometry();
    createLabelsGeometry();
    initTextRendering();
}

void CoordinateAxes::setAxisColors(const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor) {
    this->xColor = xColor;
    this->yColor = yColor;
    this->zColor = zColor;
}

float CoordinateAxes::getAxisLength() const {
    return axisLength;
}

// Static window dimensions (you'll need to set these from main.cpp)
int CoordinateAxes::windowWidth = 1200;
int CoordinateAxes::windowHeight = 800;