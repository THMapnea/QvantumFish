#include "BottomRightQuadrant.h"
#include <glad/glad.h>
#include <iostream>

BottomRightQuadrant::BottomRightQuadrant()
    : backgroundColor(glm::vec3(0.15f, 0.15f, 0.15f)) {
}

BottomRightQuadrant::~BottomRightQuadrant() {
    // Cleanup if needed
}

void BottomRightQuadrant::initialize() {
    std::cout << "BottomRightQuadrant initialized." << std::endl;
    // TODO: Add initialization for bottom-right quadrant content
    // Potential content: Quantum gates panel, control parameters, algorithm visualization
}

void BottomRightQuadrant::render(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Skip rendering if viewport dimensions are invalid
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // TODO: Add your bottom-right quadrant content here
    // Potential content: Quantum gates interface, algorithm controls

    // For now, this quadrant content is handled by the background rendering in main
}