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
#include "CoordinatesAxes.h"  // Aggiungi questo include
#include "Qubit.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Global variables
BlochSphere* blochSphere = nullptr;
VectorArrow* quantumVector = nullptr;
CoordinateAxes* coordinateAxes = nullptr;  // Aggiungi questa variabile

// Mouse input variables
double lastX = WIDTH / 2.0f;
double lastY = HEIGHT / 2.0f;
double yaw = 0.0f;
double pitch = 0.0f;
bool firstMouse = true;
bool mousePressed = false;

// Key input
static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Reset scene when R key is pressed
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        yaw = 0.0f;
        pitch = 0.0f;
        std::cout << "Scene reset to default view!" << std::endl;
    }
}

// Mouse callback
static void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (mousePressed) {
        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        double sensitivity = 0.2f;
        xoffset *= sensitivity;
        yoffset *= sensitivity;

        yaw += xoffset;
        pitch += yoffset;

        // Constrain pitch to avoid flipping
        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;
    }

    lastX = xpos;
    lastY = ypos;
}

// Mouse button callback
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            mousePressed = true;
            firstMouse = true; // Reset first mouse to get smooth movement
        }
        else if (action == GLFW_RELEASE) {
            mousePressed = false;
        }
    }
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "QvantumFish - Bloch Sphere with Coordinate Axes", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set mouse callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Enable blending for opacity effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // 1. FIRST: Create Coordinate Axes
    coordinateAxes = new CoordinateAxes(
        1.3f,    // axis length (slightly larger than sphere)
        0.02f,   // axis thickness
        glm::vec3(1.0f, 0.3f, 0.3f),  // X-axis color (red)
        glm::vec3(0.3f, 1.0f, 0.3f),  // Y-axis color (green) 
        glm::vec3(0.3f, 0.3f, 1.0f)   // Z-axis color (blue)
    );

    // 2. SECOND: Create Bloch Sphere (centered at origin)
    blochSphere = new BlochSphere(1.0f, 32, 32);

    // 3. THIRD: Create qubit and quantum vector
    Qubit q = Qubit::ketZero();

    // Create Quantum Vector with VectorArrow
    quantumVector = new VectorArrow(
        q.getBlochSphereCoordinates().convertToVec3(),  // position
        1.0f,                                           // vector length (radius)
        0.15f,                                          // arrowhead height
        0.06f,                                          // arrowhead base radius
        8,                                              // line segments
        16                                              // cone slices
    );

    // Set color to red for the vector
    quantumVector->setColor(glm::vec3(1.0f, 0.2f, 0.2f));

    // Set line width for better visibility
    glLineWidth(2.0f);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(2.5f, 2.5f, 2.5f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    glm::mat4 model = glm::mat4(1.0f);

    std::cout << "3D Coordinate System with Bloch Sphere initialized." << std::endl;
    std::cout << "Visualization order:" << std::endl;
    std::cout << "1. Coordinate Axes (X=Red, Y=Green, Z=Blue)" << std::endl;
    std::cout << "2. Bloch Sphere (centered at origin)" << std::endl;
    std::cout << "3. Quantum State Vector (Red arrow)" << std::endl;
    std::cout << "Controls: Mouse drag to rotate, R to reset view, ESC to exit" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Pure black background like oscilloscope
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = glfwGetTime();

        // RENDER ORDER:
        // 1. FIRST: Render Coordinate Axes (background reference)
        coordinateAxes->render(time, view, projection, model, yaw, pitch);

        // 2. SECOND: Render the Bloch Sphere (centered at origin)
        blochSphere->render(time, view, projection, model, yaw, pitch);

        // 3. THIRD: Render the Quantum Vector Arrow (on top)
        quantumVector->render(time, view, projection, model, yaw, pitch);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup - nell'ordine inverso di creazione
    delete quantumVector;
    delete blochSphere;
    delete coordinateAxes;
    glfwTerminate();

    return 0;
}