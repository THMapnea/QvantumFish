#ifndef BOTTOM_LEFT_QUADRANT_H
#define BOTTOM_LEFT_QUADRANT_H

#include <glm/glm.hpp>

class BottomLeftQuadrant {
private:
    glm::vec3 backgroundColor;

public:
    BottomLeftQuadrant();
    ~BottomLeftQuadrant();

    // Initialization
    void initialize();

    // Rendering
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);

    // Getters
    glm::vec3 getBackgroundColor() const { return backgroundColor; }

    // Setters
    void setBackgroundColor(const glm::vec3& color) { backgroundColor = color; }
};

#endif // BOTTOM_LEFT_QUADRANT_H