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
#include "Qubit.h"

const unsigned int WIDTH = 800;
const unsigned int HEIGHT = 600;

// Global variables
BlochSphere* blochSphere = nullptr;
VectorArrow* quantumVector = nullptr;
CoordinateAxes* coordinateAxes = nullptr;
ProjectionLines* projectionLines = nullptr;
AngleArcs* angleArcs = nullptr;

// Animation control
bool enableAnimations = true; // Set to false to disable all animations
float animationStartTime = 0.0f;
const float VECTOR_ANIMATION_DURATION = 1.5f;
const float LINES_ANIMATION_DURATION = 1.0f;
const float ARCS_ANIMATION_DURATION = 1.2f;

// Camera variables
glm::vec3 cameraPos = glm::vec3(2.5f, 2.5f, 2.5f);
float zoomLevel = 1.0f;
const float MIN_ZOOM = 0.3f;
const float MAX_ZOOM = 3.0f;
const float ZOOM_SPEED = 0.1f;

// Mouse input variables
double lastX = WIDTH / 2.0f;
double lastY = HEIGHT / 2.0f;
double yaw = 0.0f;
double pitch = 0.0f;
bool firstMouse = true;
bool mousePressed = false;

// Animation easing functions
float easeOutCubic(float t) {
    return 1.0f - pow(1.0f - t, 3.0f);
}

float easeInOutCubic(float t) {
    return t < 0.5f ? 4.0f * t * t * t : 1.0f - pow(-2.0f * t + 2.0f, 3.0f) / 2.0f;
}

float easeOutBack(float t) {
    const float c1 = 1.70158f;
    const float c3 = c1 + 1.0f;
    return 1.0f + c3 * pow(t - 1.0f, 3.0f) + c1 * pow(t - 1.0f, 2.0f);
}

// Key input
static void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Reset scene when R key is pressed
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        yaw = 0.0f;
        pitch = 0.0f;
        zoomLevel = 1.0f;
        animationStartTime = static_cast<float>(glfwGetTime());
        std::cout << "Scene reset to default view!" << std::endl;
    }

    // Toggle animations with A key
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
        static bool keyProcessed = false;
        if (!keyProcessed) {
            enableAnimations = !enableAnimations;
            std::cout << "Animations " << (enableAnimations ? "ENABLED" : "DISABLED") << std::endl;
            keyProcessed = true;
        }
    }
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_RELEASE) {
        static bool keyProcessed = false;
        keyProcessed = false;
    }

    // Zoom in with + key
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        zoomLevel -= ZOOM_SPEED;
        if (zoomLevel < MIN_ZOOM) zoomLevel = MIN_ZOOM;
        std::cout << "Zoom level: " << zoomLevel << std::endl;
    }

    // Zoom out with - key
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        zoomLevel += ZOOM_SPEED;
        if (zoomLevel > MAX_ZOOM) zoomLevel = MAX_ZOOM;
        std::cout << "Zoom level: " << zoomLevel << std::endl;
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

// Scroll callback for mouse wheel zoom
static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {
    zoomLevel -= yoffset * ZOOM_SPEED * 0.5f;
    if (zoomLevel < MIN_ZOOM) zoomLevel = MIN_ZOOM;
    if (zoomLevel > MAX_ZOOM) zoomLevel = MAX_ZOOM;
    std::cout << "Zoom level: " << zoomLevel << std::endl;
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

    // Set mouse callbacks
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glfwSetScrollCallback(window, scroll_callback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    glEnable(GL_DEPTH_TEST);

    // Enable blending for opacity effect
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Color scheme matching the sphere style
    glm::vec3 axesColor = glm::vec3(0.4f, 0.6f, 0.8f); // Light blue-gray similar to sphere grid
    glm::vec3 vectorColor = glm::vec3(1.0f, 0.3f, 0.3f); // Bright red for contrast
    glm::vec3 projectionColor = glm::vec3(0.8f, 0.8f, 0.2f); // Yellow for projection lines
    glm::vec3 arcColor = glm::vec3(0.2f, 0.8f, 0.2f); // Green for angle arcs

    // Initialize animation start time
    animationStartTime = static_cast<float>(glfwGetTime());

    // 1. FIRST: Create Coordinate Axes with uniform color
    coordinateAxes = new CoordinateAxes(
        1.2f,    // axis length (slightly larger than sphere)
        0.02f,   // axis thickness
        axesColor,  // X-axis color (same for all)
        axesColor,  // Y-axis color (same for all)
        axesColor   // Z-axis color (same for all)
    );

    // 2. SECOND: Create Bloch Sphere (centered at origin)
    blochSphere = new BlochSphere(1.0f, 32, 32);

    // 3. THIRD: Create qubit and quantum vector
    Qubit q = Qubit(std::cos(M_PI / 9), std::exp(std::complex<double>(0, 1) * (M_PI / 2)) * std::sin(M_PI / 9));
    // Print the qubit information
    q.advancedLook();

    // Get the Bloch sphere coordinates for the vector
    glm::vec3 vectorPos = q.getBlochSphereCoordinates().convertToVec3();

    // Create Quantum Vector with VectorArrow
    quantumVector = new VectorArrow(
        vectorPos,  // position
        1.0f,       // vector length (radius)
        0.15f,      // arrowhead height
        0.06f,      // arrowhead base radius
        8,          // line segments
        16          // cone slices
    );

    // Set color to bright red for the vector (good contrast)
    quantumVector->setColor(vectorColor);

    // 4. FOURTH: Create projection lines
    projectionLines = new ProjectionLines(
        vectorPos,      // same position as the vector
        projectionColor, // yellow color for projection lines
        0.03f,          // dash size
        25              // number of segments
    );

    // 5. FIFTH: Create angle arcs
    angleArcs = new AngleArcs(
        vectorPos,      // same position as the vector
        arcColor,       // green color for angle arcs
        0.25f,          // arc radius
        32              // number of segments per arc
    );

    // Set line widths for proper visibility
    glLineWidth(2.0f);

    glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)WIDTH / (float)HEIGHT, 0.1f, 100.0f);
    glm::mat4 model = glm::mat4(1.0f);

    // Calculate actual angles for display
    float theta = acos(vectorPos.z / glm::length(vectorPos));
    float phi = atan2(vectorPos.y, vectorPos.x);

    std::cout << "Bloch Sphere Visualization initialized." << std::endl;
    std::cout << "Visualization order:" << std::endl;
    std::cout << "1. Coordinate Axes (light blue-gray)" << std::endl;
    std::cout << "2. Bloch Sphere (white grid)" << std::endl;
    std::cout << "3. Projection Lines (yellow dashed lines)" << std::endl;
    std::cout << "4. Angle Arcs (green arcs)" << std::endl;
    std::cout << "5. Quantum State Vector (red arrow)" << std::endl;
    std::cout << "Controls:" << std::endl;
    std::cout << "  - Mouse drag: Rotate view" << std::endl;
    std::cout << "  - Mouse wheel: Zoom in/out" << std::endl;
    std::cout << "  - R key: Reset view and restart animations" << std::endl;
    std::cout << "  - A key: Toggle animations ON/OFF" << std::endl;
    std::cout << "  - ESC key: Exit" << std::endl;
    std::cout << "Current qubit state:" << std::endl;
    std::cout << "  Theta (polar angle): " << theta << " rad (" << theta * 180.0f / M_PI << "°)" << std::endl;
    std::cout << "  Phi (azimuthal angle): " << phi << " rad (" << phi * 180.0f / M_PI << "°)" << std::endl;
    std::cout << "  Vector position: (" << vectorPos.x << ", " << vectorPos.y << ", " << vectorPos.z << ")" << std::endl;
    std::cout << "Animations: " << (enableAnimations ? "ENABLED" : "DISABLED") << std::endl;

    while (!glfwWindowShouldClose(window)) {
        processInput(window);

        // Pure black background like oscilloscope
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float currentTime = glfwGetTime();
        float animationTime = currentTime - animationStartTime;

        // Calculate camera position based on zoom level
        glm::vec3 currentCameraPos = cameraPos * zoomLevel;
        glm::mat4 view = glm::lookAt(currentCameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

        // Animation progress calculations
        float vectorProgress = enableAnimations ?
            glm::clamp(animationTime / VECTOR_ANIMATION_DURATION, 0.0f, 1.0f) : 1.0f;
        float linesProgress = enableAnimations ?
            glm::clamp((animationTime - VECTOR_ANIMATION_DURATION * 0.3f) / LINES_ANIMATION_DURATION, 0.0f, 1.0f) : 1.0f;
        float arcsProgress = enableAnimations ?
            glm::clamp((animationTime - VECTOR_ANIMATION_DURATION * 0.6f) / ARCS_ANIMATION_DURATION, 0.0f, 1.0f) : 1.0f;

        // Apply easing to animation progress
        vectorProgress = easeOutBack(vectorProgress);
        linesProgress = easeOutCubic(linesProgress);
        arcsProgress = easeInOutCubic(arcsProgress);

        // RENDER ORDER for proper depth management:

        // 1. FIRST: Render Coordinate Axes (background reference) - always visible
        coordinateAxes->render(currentTime, view, projection, model, yaw, pitch);

        // 2. SECOND: Render the Bloch Sphere (centered at origin) - always visible
        blochSphere->render(currentTime, view, projection, model, yaw, pitch);

        // 3. THIRD: Render Projection Lines (dashed yellow lines) - animated
        if (linesProgress > 0.0f) {
            // Modify projection lines opacity based on animation progress
            glm::vec3 animatedProjectionColor = projectionColor * linesProgress;
            projectionLines->setColor(animatedProjectionColor);
            projectionLines->render(currentTime, view, projection, model, yaw, pitch);
            // Restore original color
            projectionLines->setColor(projectionColor);
        }

        // 4. FOURTH: Render Angle Arcs (green arcs showing angles) - animated
        if (arcsProgress > 0.0f) {
            // Modify arcs opacity based on animation progress
            glm::vec3 animatedArcColor = arcColor * arcsProgress;
            angleArcs->setColor(animatedArcColor);
            angleArcs->render(currentTime, view, projection, model, yaw, pitch);
            // Restore original color
            angleArcs->setColor(arcColor);
        }

        // 5. FIFTH: Render the Quantum Vector Arrow - animated growth
        if (vectorProgress > 0.0f) {
            glLineWidth(2.5f * vectorProgress); // Animated line width
            // Modify vector color opacity based on animation progress
            glm::vec3 animatedVectorColor = vectorColor * vectorProgress;
            quantumVector->setColor(animatedVectorColor);
            quantumVector->render(currentTime, view, projection, model, yaw, pitch);
            // Restore original color and line width
            quantumVector->setColor(vectorColor);
            glLineWidth(2.0f);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    delete angleArcs;
    delete projectionLines;
    delete quantumVector;
    delete blochSphere;
    delete coordinateAxes;
    glfwTerminate();

    return 0;
}