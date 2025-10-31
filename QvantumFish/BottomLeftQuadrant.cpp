#include "BottomLeftQuadrant.h"
#include <glad/glad.h>
#include <iostream>

BottomLeftQuadrant::BottomLeftQuadrant()
    : backgroundColor(glm::vec3(0.1f, 0.1f, 0.1f)) {
}

BottomLeftQuadrant::~BottomLeftQuadrant() {
    // Cleanup if needed
}

void BottomLeftQuadrant::initialize() {
    std::cout << "BottomLeftQuadrant initialized." << std::endl;
    // TODO: Add initialization for bottom-left quadrant content
    // Potential content: Measurement statistics, probability distributions
}

void BottomLeftQuadrant::render(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Skip rendering if viewport dimensions are invalid
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // TODO: Add your bottom-left quadrant content here
    // Potential content: Probability histograms, measurement results

    // For now, this quadrant content is handled by the background rendering in main
}