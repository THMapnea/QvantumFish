#include "TopLeftQuadrant.h"
#include <glad/glad.h>
#include <iostream>

TopLeftQuadrant::TopLeftQuadrant()
    : backgroundColor(glm::vec3(0.12f, 0.12f, 0.12f)) {
}

TopLeftQuadrant::~TopLeftQuadrant() {
    // Cleanup if needed
}

void TopLeftQuadrant::initialize() {
    std::cout << "TopLeftQuadrant initialized." << std::endl;
    // TODO: Add initialization for top-left quadrant content
    // Potential content: Quantum circuit diagrams, state evolution visualization
}

void TopLeftQuadrant::render(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Skip rendering if viewport dimensions are invalid
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // TODO: Add your top-left quadrant content here
    // Potential content: Quantum circuit visualization, gate operations display

    // For now, this quadrant content is handled by the background rendering in main
}