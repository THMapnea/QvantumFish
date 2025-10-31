#ifndef TOP_LEFT_QUADRANT_H
#define TOP_LEFT_QUADRANT_H

#include <glm/glm.hpp>

class TopLeftQuadrant {
private:
    glm::vec3 backgroundColor;

public:
    TopLeftQuadrant();
    ~TopLeftQuadrant();
    
    // Initialization
    void initialize();
    
    // Rendering
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);
    
    // Getters
    glm::vec3 getBackgroundColor() const { return backgroundColor; }
    
    // Setters
    void setBackgroundColor(const glm::vec3& color) { backgroundColor = color; }
};

#endif // TOP_LEFT_QUADRANT_H