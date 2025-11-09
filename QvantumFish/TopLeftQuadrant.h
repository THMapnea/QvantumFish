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
    char inputBuffer[1024];
    bool textModified;
    bool inputActive;

    void renderTextEditor();
    void handleInput();
    void updateCurrentLine();
};

#endif