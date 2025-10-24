#ifndef SCENE_CONTROLLER_H
#define SCENE_CONTROLLER_H

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <functional>
#include <iostream>

class SceneController {
private:
    // Camera control variables
    glm::vec3 cameraPos;
    float zoomLevel;
    float minZoom;
    float maxZoom;
    float zoomSpeed;

    // Mouse control variables
    double lastX, lastY;
    double yaw, pitch;
    bool firstMouse;
    bool mousePressed;
    float rotationSensitivity;

    // Window dimensions
    unsigned int windowWidth, windowHeight;

    // Callback functions
    std::function<void()> onResetCallback;
    std::function<void(float)> onZoomCallback;

public:
    SceneController(unsigned int width = 800, unsigned int height = 600,
        glm::vec3 initialCameraPos = glm::vec3(2.5f, 2.5f, 2.5f),
        float initialZoom = 1.0f,
        float minZoomLevel = 0.3f,
        float maxZoomLevel = 3.0f,
        float zoomSpeedValue = 0.1f,
        float sensitivity = 0.2f);

    // GLFW callback functions (static for C compatibility)
    static void mouseCallback(GLFWwindow* window, double xpos, double ypos);
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

    // Instance-based callback handlers
    void handleMouseMovement(double xpos, double ypos);
    void handleMouseButton(int button, int action, int mods);
    void handleScroll(double xoffset, double yoffset);
    void processInput(GLFWwindow* window);

    // Setup GLFW callbacks for a window
    void setupCallbacks(GLFWwindow* window);

    // Getters
    glm::mat4 getViewMatrix() const;
    float getYaw() const { return yaw; }
    float getPitch() const { return pitch; }
    float getZoomLevel() const { return zoomLevel; }
    glm::vec3 getCameraPosition() const { return cameraPos * zoomLevel; }

    // Setters for callbacks
    void setOnResetCallback(std::function<void()> callback) { onResetCallback = callback; }
    void setOnZoomCallback(std::function<void(float)> callback) { onZoomCallback = callback; }

    // Reset function
    void reset();

    // Update window size (for responsive design)
    void updateWindowSize(unsigned int width, unsigned int height);
};

#endif // SCENE_CONTROLLER_H