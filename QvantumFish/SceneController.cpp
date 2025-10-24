#define _USE_MATH_DEFINES
#include "SceneController.h"
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>


// Static instance pointer for GLFW callbacks
static SceneController* s_SceneControllerInstance = nullptr;

SceneController::SceneController(unsigned int width, unsigned int height,
    glm::vec3 initialCameraPos,
    float initialZoom,
    float minZoomLevel,
    float maxZoomLevel,
    float zoomSpeedValue,
    float sensitivity)
    : cameraPos(initialCameraPos), zoomLevel(initialZoom),
    minZoom(minZoomLevel), maxZoom(maxZoomLevel), zoomSpeed(zoomSpeedValue),
    windowWidth(width), windowHeight(height),
    lastX(width / 2.0f), lastY(height / 2.0f),
    yaw(0.0f), pitch(0.0f),
    firstMouse(true), mousePressed(false),
    rotationSensitivity(sensitivity) {

    // Store instance for static callbacks
    s_SceneControllerInstance = this;
}

void SceneController::setupCallbacks(GLFWwindow* window) {
    glfwSetCursorPosCallback(window, SceneController::mouseCallback);
    glfwSetMouseButtonCallback(window, SceneController::mouseButtonCallback);
    glfwSetScrollCallback(window, SceneController::scrollCallback);
}

void SceneController::mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (s_SceneControllerInstance) {
        s_SceneControllerInstance->handleMouseMovement(xpos, ypos);
    }
}

void SceneController::mouseButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    if (s_SceneControllerInstance) {
        s_SceneControllerInstance->handleMouseButton(button, action, mods);
    }
}

void SceneController::scrollCallback(GLFWwindow* window, double xoffset, double yoffset) {
    if (s_SceneControllerInstance) {
        s_SceneControllerInstance->handleScroll(xoffset, yoffset);
    }
}

void SceneController::handleMouseMovement(double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    if (mousePressed) {
        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

        xoffset *= rotationSensitivity;
        yoffset *= rotationSensitivity;

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

void SceneController::handleMouseButton(int button, int action, int mods) {
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

void SceneController::handleScroll(double xoffset, double yoffset) {
    zoomLevel -= yoffset * zoomSpeed * 0.5f;
    if (zoomLevel < minZoom) zoomLevel = minZoom;
    if (zoomLevel > maxZoom) zoomLevel = maxZoom;

    std::cout << "Zoom level: " << zoomLevel << std::endl;

    // Call zoom callback if set
    if (onZoomCallback) {
        onZoomCallback(zoomLevel);
    }
}

void SceneController::processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);

    // Reset scene when R key is pressed
    if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
        reset();
    }

    // Zoom in with + key
    if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_ADD) == GLFW_PRESS) {
        zoomLevel -= zoomSpeed;
        if (zoomLevel < minZoom) zoomLevel = minZoom;
        std::cout << "Zoom level: " << zoomLevel << std::endl;

        if (onZoomCallback) {
            onZoomCallback(zoomLevel);
        }
    }

    // Zoom out with - key
    if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS || glfwGetKey(window, GLFW_KEY_KP_SUBTRACT) == GLFW_PRESS) {
        zoomLevel += zoomSpeed;
        if (zoomLevel > maxZoom) zoomLevel = maxZoom;
        std::cout << "Zoom level: " << zoomLevel << std::endl;

        if (onZoomCallback) {
            onZoomCallback(zoomLevel);
        }
    }
}

glm::mat4 SceneController::getViewMatrix() const {
    glm::vec3 currentCameraPos = cameraPos * zoomLevel;
    glm::mat4 view = glm::lookAt(currentCameraPos, glm::vec3(0, 0, 0), glm::vec3(0, 0, 1));

    // Apply rotation
    glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(static_cast<float>(yaw)), glm::vec3(0.0f, 0.0f, 1.0f));
    rotation = glm::rotate(rotation, glm::radians(static_cast<float>(pitch)), glm::vec3(1.0f, 0.0f, 0.0f));

    return view * rotation;
}

void SceneController::reset() {
    yaw = 0.0f;
    pitch = 0.0f;
    zoomLevel = 1.0f;
    std::cout << "Scene reset to default view!" << std::endl;

    // Call reset callback if set
    if (onResetCallback) {
        onResetCallback();
    }
}

void SceneController::updateWindowSize(unsigned int width, unsigned int height) {
    windowWidth = width;
    windowHeight = height;
}