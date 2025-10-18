#define _USE_MATH_DEFINES


#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cmath>
#include "BlochSphere.h"
#include "VectorSphere.h"
#include "Qubit.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Global variables
BlochSphere* blochSphere = nullptr;
VectorSphere* quantumVector = nullptr;

// Mouse input variables
double lastX = WIDTH / 2.0f;
double lastY = HEIGHT / 2.0f;
double yaw = 0.0f;
double pitch = 0.0f;
bool firstMouse = true;
bool mousePressed = false;

// Key input
void processInput(GLFWwindow* window) {
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
void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
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
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
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

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "QvantumFish - Bloch Sphere", NULL, NULL);
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

    // Create Bloch Sphere
    blochSphere = new BlochSphere(1.0f, 32, 32);

    // Create Quantum Vector pointing to |0> state (north pole: 0,0,1)
    quantumVector = new VectorSphere(glm::vec3(0.0f, 0.0f, 1.0f));

    // Set line width for better visibility
    glLineWidth(2.0f);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 view = glm::lookAt(glm::vec3(2.5f, 2.5f, 2.5f), glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));
    glm::mat4 model = glm::mat4(1.0f);

    std::cout << "Bloch Sphere with Quantum Vector initialized." << std::endl;
    std::cout << "Vector position: (0, 0, 1) - |0> state" << std::endl;
    std::cout << "Controls: Mouse drag to rotate, R to reset view, ESC to exit" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Pure black background like oscilloscope
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Render the Bloch Sphere
        float time = glfwGetTime();
        blochSphere->render(time, view, projection, model, yaw, pitch);

        // Render the Quantum Vector
        quantumVector->render(time, view, projection, model, yaw, pitch);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete blochSphere;
    delete quantumVector;
    glfwTerminate();

    return 0;
}