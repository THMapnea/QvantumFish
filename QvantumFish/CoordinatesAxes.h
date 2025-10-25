#ifndef COORDINATES_AXES_H
#define COORDINATES_AXES_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <map>

// Character structure for text rendering
struct Character {
    unsigned int TextureID; // ID handle of the glyph texture
    glm::ivec2   Size;      // Size of glyph
    glm::ivec2   Bearing;   // Offset from baseline to left/top of glyph
    unsigned int Advance;   // Horizontal offset to advance to next glyph
};

class CoordinateAxes {
private:
    unsigned int axesVAO, axesVBO;
    unsigned int labelsVAO, labelsVBO;
    unsigned int textVAO, textVBO;
    unsigned int shaderProgram;
    unsigned int textShaderProgram;

    float axisLength;
    float axisThickness;
    glm::vec3 xColor, yColor, zColor;

    // Text rendering
    std::map<char, Character> Characters;
    static int windowWidth;
    static int windowHeight;

    // Private methods
    unsigned int compileShader(unsigned int type, const char* source);
    std::vector<float> generateAxisVertices();
    std::vector<float> generateLabelVertices();
    void compileShaders();
    void createAxesGeometry();
    void createLabelsGeometry();
    void initTextRendering();

    // Text rendering methods
    void renderText(const std::string& text, float x, float y, float scale, const glm::vec3& color);
    void renderText3D(const std::string& text, const glm::vec3& position, float scale, const glm::vec3& color,
        const glm::mat4& view, const glm::mat4& projection, const glm::mat4& model);

public:
    CoordinateAxes(float length = 1.2f,
        float thickness = 0.02f,
        glm::vec3 xAxisColor = glm::vec3(0.6f, 0.6f, 0.8f),
        glm::vec3 yAxisColor = glm::vec3(0.6f, 0.6f, 0.8f),
        glm::vec3 zAxisColor = glm::vec3(0.6f, 0.6f, 0.8f));
    ~CoordinateAxes();

    void cleanup();
    void render(float time, const glm::mat4& view, const glm::mat4& projection,
        const glm::mat4& model, float yaw = 0.0f, float pitch = 0.0f);
    void setAxisLength(float length);
    void setAxisColors(const glm::vec3& xColor, const glm::vec3& yColor, const glm::vec3& zColor);
    float getAxisLength() const;

    // Static method to update window size for text rendering
    static void setWindowSize(int width, int height) {
        windowWidth = width;
        windowHeight = height;
    }
};

#endif