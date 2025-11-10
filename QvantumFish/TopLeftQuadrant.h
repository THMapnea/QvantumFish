#ifndef TOP_LEFT_QUADRANT_H
#define TOP_LEFT_QUADRANT_H

#include <glm/glm.hpp>
#include <string>
#include <vector>

class TopLeftQuadrant {
public:
    TopLeftQuadrant();
    ~TopLeftQuadrant() = default;

    void initialize();
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);

    glm::vec3 getBackgroundColor() const { return backgroundColor; }

private:
    glm::vec3 backgroundColor;

    // Text editor state
    std::vector<std::string> textLines;
    bool showLineNumbers;
    int cursorPosition;
    bool textModified;
    bool inputActive;
    size_t cursorColumn;

    void renderTextEditor();
    void handleInput();
    void handleCharacterInput(unsigned int c);
    void handleSpecialKeys();
    void handleNavigationKeys();
    void handleCtrlCombinations();
    
    // Helper methods
    void ensureCursorInBounds();
    void insertNewLine();
    void deleteCharacter(bool isBackspace);
};

#endif