#ifndef BOTTOM_RIGHT_QUADRANT_H
#define BOTTOM_RIGHT_QUADRANT_H

#include <glm/glm.hpp>


class BottomRightQuadrant {
private:
    glm::vec3 backgroundColor;

public:
    BottomRightQuadrant();
    ~BottomRightQuadrant();

    // Initialization
    void initialize();

    // Rendering
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);

    // Getters
    glm::vec3 getBackgroundColor() const { return backgroundColor; }

    // Setters
    void setBackgroundColor(const glm::vec3& color) { backgroundColor = color; }
};

#endif // BOTTOM_RIGHT_QUADRANT_H