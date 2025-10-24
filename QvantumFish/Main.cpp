#define _USE_MATH_DEFINES

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include <array>
#include <complex>
#include <Eigen/Dense>
#include "BlochSphere.h"
#include "VectorSphere.h"
#include "VectorArrow.h"
#include "CoordinatesAxes.h"
#include "ProjectionLines.h"
#include "AngleArcs.h"
#include "SceneController.h"
#include "Qubit.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Global variables
BlochSphere* blochSphere = nullptr;
VectorArrow* quantumVector = nullptr;
CoordinateAxes* coordinateAxes = nullptr;
ProjectionLines* projectionLines = nullptr;
AngleArcs* angleArcs = nullptr;
SceneController* sceneController = nullptr;

void initializeScene() {
    // Color scheme matching the sphere style
    glm::vec3 axesColor = glm::vec3(0.4f, 0.6f, 0.8f);
    glm::vec3 vectorColor = glm::vec3(1.0f, 0.3f, 0.3f);
    glm::vec3 projectionColor = glm::vec3(0.8f, 0.8f, 0.2f);
    glm::vec3 arcColor = glm::vec3(0.2f, 0.8f, 0.2f);

    // 1. Create Coordinate Axes
    coordinateAxes = new CoordinateAxes(1.2f, 0.02f, axesColor, axesColor, axesColor);

    // 2. Create Bloch Sphere
    blochSphere = new BlochSphere(1.0f, 32, 32);

    // 3. Create qubit and quantum vector
    Qubit q = Qubit(std::cos(M_PI / 9), std::exp(std::complex<double>(0, 1) * (M_PI / 2)) * std::sin(M_PI / 9));
    q.advancedLook();

    glm::vec3 vectorPos = q.getBlochSphereCoordinates().convertToVec3();

    // Create Quantum Vector
    quantumVector = new VectorArrow(vectorPos, 1.0f, 0.15f, 0.06f, 8, 16);
    quantumVector->setColor(vectorColor);

    // 4. Create projection lines
    projectionLines = new ProjectionLines(vectorPos, projectionColor, 0.03f, 25);

    // 5. Create angle arcs
    angleArcs = new AngleArcs(vectorPos, arcColor, 0.25f, 32);

    // Set line width
    glLineWidth(2.0f);

    // Print scene information
    float theta = acos(vectorPos.z / glm::length(vectorPos));
    float phi = atan2(vectorPos.y, vectorPos.x);

    std::cout << "Bloch Sphere Visualization initialized." << std::endl;
    std::cout << "Current qubit state:" << std::endl;
    std::cout << "  Theta (polar angle): " << theta << " rad (" << theta * 180.0f / M_PI << "°)" << std::endl;
    std::cout << "  Phi (azimuthal angle): " << phi << " rad (" << phi * 180.0f / M_PI << "°)" << std::endl;
    std::cout << "  Vector position: (" << vectorPos.x << ", " << vectorPos.y << ", " << vectorPos.z << ")" << std::endl;
}

void cleanupScene() {
    delete angleArcs;
    delete projectionLines;
    delete quantumVector;
    delete blochSphere;
    delete coordinateAxes;
    delete sceneController;
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "QvantumFish - Bloch Sphere Visualization", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Initialize Scene Controller
    sceneController = new SceneController(WIDTH, HEIGHT);

    // Setup GLFW callbacks
    sceneController->setupCallbacks(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize the scene
    initializeScene();

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f);

    std::cout << "Controls:" << std::endl;
    std::cout << "  - Mouse drag: Rotate view" << std::endl;
    std::cout << "  - Mouse wheel: Zoom in/out" << std::endl;
    std::cout << "  - R key: Reset view" << std::endl;
    std::cout << "  - ESC key: Exit" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        sceneController->processInput(window);

        // Pure black background like oscilloscope
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();

        // Get view matrix from scene controller
        glm::mat4 view = sceneController->getViewMatrix();
        float yaw = sceneController->getYaw();
        float pitch = sceneController->getPitch();

        // RENDER ORDER for proper depth management:
        coordinateAxes->render(time, view, projection, model, yaw, pitch);
        blochSphere->render(time, view, projection, model, yaw, pitch);
        projectionLines->render(time, view, projection, model, yaw, pitch);
        angleArcs->render(time, view, projection, model, yaw, pitch);

        glLineWidth(2.5f);
        quantumVector->render(time, view, projection, model, yaw, pitch);
        glLineWidth(2.0f);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    cleanupScene();
    glfwTerminate();
    return 0;
}