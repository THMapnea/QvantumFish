#ifndef TOP_LEFT_QUADRANT_H
#define TOP_LEFT_QUADRANT_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class TopLeftQuadrant {
public:
    TopLeftQuadrant();
    ~TopLeftQuadrant();

    void initialize();
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);

    glm::vec3 getBackgroundColor() const { return backgroundColor; }

private:
    glm::vec3 backgroundColor;

    // Text editor state
    std::vector<std::string> textLines;
    bool showLineNumbers;
    int cursorPosition;
    int scrollPosition;
    bool textModified;
    bool inputActive;
    size_t cursorColumn;  // Current cursor position within line

    // Mouse state
    bool mouseClicked;
    float lineHeight;

    void renderTextEditor();
    void handleInput();
    void handleCharacterInput(unsigned int c);
    void handleSpecialKeys();
    void handleNavigationKeys();
    void handleCtrlCombinations();
    void handleMouseInput();
    int getLineFromMousePos(float mouseY);
    size_t getColumnFromMousePos(int line, float mouseX);
};

#endif