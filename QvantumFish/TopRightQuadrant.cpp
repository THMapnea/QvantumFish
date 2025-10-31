#define _USE_MATH_DEFINES

#include "TopRightQuadrant.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <complex>

TopRightQuadrant::TopRightQuadrant()
    : coordinateAxes(nullptr), blochSphere(nullptr), quantumVector(nullptr),
    projectionLines(nullptr), angleArcs(nullptr),
    // Initialize with default qubit
    currentQubit(Qubit::ketZero()),
    axesColor(glm::vec3(0.4f, 0.6f, 0.8f)),
    vectorColor(glm::vec3(1.0f, 0.3f, 0.3f)),
    projectionColor(glm::vec3(0.8f, 0.8f, 0.2f)),
    arcColor(glm::vec3(0.2f, 0.8f, 0.2f)),
    showSphere(true), showAxes(true), showVector(true),
    showProjections(true), showArcs(true), sphereScale(1.0f) {
}

TopRightQuadrant::~TopRightQuadrant() {
    cleanup();
}

void TopRightQuadrant::initialize() {
    // 1. Create Coordinate Axes
    coordinateAxes = new CoordinateAxes(1.2f, 0.02f, axesColor, axesColor, axesColor);

    // 2. Create Bloch Sphere
    blochSphere = new BlochSphere(1.0f, 32, 32);

    // 3. Create initial qubit and quantum vector
    currentQubit = Qubit(std::cos(M_PI / 9), std::exp(std::complex<double>(0, 1) * (M_PI / 2)) * std::sin(M_PI / 9));
    currentQubit.advancedLook();

    glm::vec3 vectorPos = currentQubit.getBlochSphereCoordinates().convertToVec3();

    // Create Quantum Vector
    quantumVector = new VectorArrow(vectorPos, 1.0f, 0.15f, 0.06f, 8, 16);
    quantumVector->setColor(vectorColor);

    // 4. Create projection lines
    projectionLines = new ProjectionLines(vectorPos, projectionColor, 0.03f, 25);

    // 5. Create angle arcs
    angleArcs = new AngleArcs(vectorPos, arcColor, 0.25f, 32);

    // Print scene information
    float theta = acos(vectorPos.z / glm::length(vectorPos));
    float phi = atan2(vectorPos.y, vectorPos.x);

    std::cout << "TopRightQuadrant - Bloch Sphere Visualization initialized." << std::endl;
    std::cout << "Current qubit state:" << std::endl;
    std::cout << "  Theta (polar angle): " << theta << " rad (" << theta * 180.0f / M_PI << "°)" << std::endl;
    std::cout << "  Phi (azimuthal angle): " << phi << " rad (" << phi * 180.0f / M_PI << "°)" << std::endl;
    std::cout << "  Vector position: (" << vectorPos.x << ", " << vectorPos.y << ", " << vectorPos.z << ")" << std::endl;
}

void TopRightQuadrant::render(float time, SceneController* sceneController,
    int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Skip rendering if viewport dimensions are invalid
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear only this quadrant's depth buffer to allow proper depth testing
    glClear(GL_DEPTH_BUFFER_BIT);

    // Get matrices from scene controller
    glm::mat4 view = sceneController->getViewMatrix();
    glm::mat4 projection = sceneController->getProjectionMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    float yaw = sceneController->getYaw();
    float pitch = sceneController->getPitch();

    // Apply scaling to the sphere if needed
    glm::mat4 scaledModel = glm::scale(model, glm::vec3(sphereScale));

    // Render the Bloch Sphere components based on visibility flags
    if (showAxes && coordinateAxes) {
        coordinateAxes->render(time, view, projection, scaledModel, yaw, pitch);
    }

    if (showSphere && blochSphere) {
        blochSphere->render(time, view, projection, scaledModel, yaw, pitch);
    }

    if (showProjections && projectionLines) {
        projectionLines->render(time, view, projection, scaledModel, yaw, pitch);
    }

    if (showArcs && angleArcs) {
        angleArcs->render(time, view, projection, scaledModel, yaw, pitch);
    }

    if (showVector && quantumVector) {
        glLineWidth(2.5f);
        quantumVector->render(time, view, projection, scaledModel, yaw, pitch);
        glLineWidth(2.0f);
    }
}

void TopRightQuadrant::toggleAllComponents() {
    bool allVisible = showSphere && showAxes && showVector && showProjections && showArcs;
    showSphere = !allVisible;
    showAxes = !allVisible;
    showVector = !allVisible;
    showProjections = !allVisible;
    showArcs = !allVisible;
}

void TopRightQuadrant::updateQubitState(const Qubit& qubit) {
    // Update the stored qubit
    currentQubit = qubit;

    glm::vec3 vectorPos = currentQubit.getBlochSphereCoordinates().convertToVec3();

    if (quantumVector) {
        delete quantumVector;
    }
    if (projectionLines) {
        delete projectionLines;
    }
    if (angleArcs) {
        delete angleArcs;
    }

    // Recreate components with new qubit state
    quantumVector = new VectorArrow(vectorPos, 1.0f, 0.15f, 0.06f, 8, 16);
    quantumVector->setColor(vectorColor);

    projectionLines = new ProjectionLines(vectorPos, projectionColor, 0.03f, 25);
    angleArcs = new AngleArcs(vectorPos, arcColor, 0.25f, 32);

    std::cout << "Qubit state updated." << std::endl;
    std::cout << "Vector position: (" << vectorPos.x << ", " << vectorPos.y << ", " << vectorPos.z << ")" << std::endl;
}

glm::vec3 TopRightQuadrant::getVectorPosition() const {
    if (quantumVector) {
        return quantumVector->getPosition();
    }
    return glm::vec3(0.0f);
}

void TopRightQuadrant::cleanup() {
    delete angleArcs;
    delete projectionLines;
    delete quantumVector;
    delete blochSphere;
    delete coordinateAxes;

    // Reset pointers
    coordinateAxes = nullptr;
    blochSphere = nullptr;
    quantumVector = nullptr;
    projectionLines = nullptr;
    angleArcs = nullptr;
}