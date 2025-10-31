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
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include "BlochSphere.h"
#include "VectorSphere.h"
#include "VectorArrow.h"
#include "CoordinatesAxes.h"
#include "ProjectionLines.h"
#include "AngleArcs.h"
#include "SceneController.h"
#include "Qubit.h"
#include "TopRightQuadrant.h"

// Global variables
TopRightQuadrant* topRightQuadrant = nullptr;
SceneController* sceneController = nullptr;

// ImGui state
bool showDemoWindow = false;

// Division lines shader and buffers
unsigned int divisionLinesVAO = 0;
unsigned int divisionLinesVBO = 0;
unsigned int divisionLinesShader = 0;

// Background quad shader and buffers
unsigned int backgroundVAO = 0;
unsigned int backgroundVBO = 0;
unsigned int backgroundShader = 0;

// Current window dimensions
int windowWidth = 1200;
int windowHeight = 800;
bool windowMinimized = false;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;
    windowMinimized = (width == 0 || height == 0);

    // Update scene controller with new dimensions (only if not minimized)
    if (sceneController && !windowMinimized) {
        sceneController->updateWindowSize(width, height);
    }
}

static void initializeBackgroundQuad() {
    // Background quad vertices (normalized device coordinates)
    float vertices[] = {
        -1.0f, -1.0f, 0.0f,
         1.0f, -1.0f, 0.0f,
         1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f, 0.0f
    };

    unsigned int indices[] = {
        0, 1, 2,
        2, 3, 0
    };

    // Create and compile shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        void main() {
            gl_Position = vec4(aPos, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        void main() {
            FragColor = vec4(color, 1.0);
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    backgroundShader = glCreateProgram();
    glAttachShader(backgroundShader, vertexShader);
    glAttachShader(backgroundShader, fragmentShader);
    glLinkProgram(backgroundShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create VAO, VBO, and EBO
    glGenVertexArrays(1, &backgroundVAO);
    glGenBuffers(1, &backgroundVBO);
    unsigned int backgroundEBO;
    glGenBuffers(1, &backgroundEBO);

    glBindVertexArray(backgroundVAO);

    glBindBuffer(GL_ARRAY_BUFFER, backgroundVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, backgroundEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &backgroundEBO); // EBO is stored in VAO state
}

static void renderBackgroundQuad(const glm::vec3& color) {
    glUseProgram(backgroundShader);
    glUniform3f(glGetUniformLocation(backgroundShader, "color"), color.r, color.g, color.b);

    glBindVertexArray(backgroundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

static void initializeScene() {
    // Create and initialize the top-right quadrant (Bloch sphere visualization)
    topRightQuadrant = new TopRightQuadrant();
    topRightQuadrant->initialize();

    // Set line width for the entire scene
    glLineWidth(2.0f);
}

static void initializeDivisionLines() {
    // Division lines vertices in normalized device coordinates
    float vertices[] = {
        // Vertical line (center X)
        0.0f,  1.0f,  // top
        0.0f, -1.0f,  // bottom
        // Horizontal line (center Y)
        -1.0f, 0.0f,  // left
         1.0f, 0.0f   // right
    };

    // Create and compile shader
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec2 aPos;
        void main() {
            gl_Position = vec4(aPos, 0.0, 1.0);
        }
    )";

    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 color;
        uniform float time;
        void main() {
            vec3 glowColor = color;
            float pulse = sin(time * 2.0) * 0.1 + 0.9;
            glowColor *= pulse;
            FragColor = vec4(glowColor, 0.8);
        }
    )";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    divisionLinesShader = glCreateProgram();
    glAttachShader(divisionLinesShader, vertexShader);
    glAttachShader(divisionLinesShader, fragmentShader);
    glLinkProgram(divisionLinesShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create VAO and VBO
    glGenVertexArrays(1, &divisionLinesVAO);
    glGenBuffers(1, &divisionLinesVBO);

    glBindVertexArray(divisionLinesVAO);
    glBindBuffer(GL_ARRAY_BUFFER, divisionLinesVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
}

static void cleanupScene() {
    delete topRightQuadrant;
    delete sceneController;

    // Cleanup division lines
    if (divisionLinesVAO) {
        glDeleteVertexArrays(1, &divisionLinesVAO);
        glDeleteBuffers(1, &divisionLinesVBO);
        glDeleteProgram(divisionLinesShader);
    }

    // Cleanup background quad
    if (backgroundVAO) {
        glDeleteVertexArrays(1, &backgroundVAO);
        glDeleteBuffers(1, &backgroundVBO);
        glDeleteProgram(backgroundShader);
    }
}

static void renderDivisionLines(float time) {
    if (divisionLinesShader == 0 || windowMinimized) return;

    // Save current state
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);
    GLboolean prevDepthTest = glIsEnabled(GL_DEPTH_TEST);
    GLint prevProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &prevProgram);

    // Set up for 2D rendering (full screen)
    glDisable(GL_DEPTH_TEST);
    glViewport(0, 0, windowWidth, windowHeight);

    // Use division lines shader
    glUseProgram(divisionLinesShader);
    glUniform3f(glGetUniformLocation(divisionLinesShader, "color"), 0.0f, 0.7f, 0.9f);
    glUniform1f(glGetUniformLocation(divisionLinesShader, "time"), time);

    // Render lines
    glLineWidth(2.0f);
    glBindVertexArray(divisionLinesVAO);
    glDrawArrays(GL_LINES, 0, 4);
    glLineWidth(1.0f);

    // Restore state
    if (prevDepthTest) glEnable(GL_DEPTH_TEST);
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
    glUseProgram(prevProgram);
}

static void renderTopRightQuadrant(float time) {
    if (!topRightQuadrant || windowMinimized) return;

    // Calculate quadrant dimensions based on current window size
    int sphereViewportX = windowWidth / 2;
    int sphereViewportY = windowHeight / 2;
    int sphereViewportWidth = windowWidth / 2;
    int sphereViewportHeight = windowHeight / 2;

    // Render the top-right quadrant (Bloch sphere)
    topRightQuadrant->render(time, sceneController,
        sphereViewportX, sphereViewportY,
        sphereViewportWidth, sphereViewportHeight);
}

static void renderTopLeftQuadrant() {
    // Skip rendering if window is minimized
    if (windowMinimized) return;

    // Ensure we have valid dimensions
    int quadrantWidth = windowWidth / 2;
    int quadrantHeight = windowHeight / 2;

    if (quadrantWidth <= 0 || quadrantHeight <= 0) {
        return;
    }

    // Top-left quadrant - Set viewport
    glViewport(0, windowHeight / 2, quadrantWidth, quadrantHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set a solid background color for this quadrant
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(glm::vec3(0.12f, 0.12f, 0.12f));
    glEnable(GL_DEPTH_TEST);

    // TODO: Add your top-left quadrant content here
    // For now, it's just a solid color background
}

static void renderBottomLeftQuadrant() {
    // Skip rendering if window is minimized
    if (windowMinimized) return;

    // Ensure we have valid dimensions
    int quadrantWidth = windowWidth / 2;
    int quadrantHeight = windowHeight / 2;

    if (quadrantWidth <= 0 || quadrantHeight <= 0) {
        return;
    }

    // Bottom-left quadrant - Set viewport
    glViewport(0, 0, quadrantWidth, quadrantHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set a solid background color for this quadrant
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(glm::vec3(0.1f, 0.1f, 0.1f));
    glEnable(GL_DEPTH_TEST);

    // TODO: Add your bottom-left quadrant content here
    // For now, it's just a solid color background
}

static void renderBottomRightQuadrant() {
    // Skip rendering if window is minimized
    if (windowMinimized) return;

    // Ensure we have valid dimensions
    int quadrantWidth = windowWidth / 2;
    int quadrantHeight = windowHeight / 2;

    if (quadrantWidth <= 0 || quadrantHeight <= 0) {
        return;
    }

    // Bottom-right quadrant - Set viewport
    glViewport(windowWidth / 2, 0, quadrantWidth, quadrantHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set a solid background color for this quadrant
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(glm::vec3(0.15f, 0.15f, 0.15f));
    glEnable(GL_DEPTH_TEST);

    // TODO: Add your bottom-right quadrant content here
    // For now, it's just a solid color background
}

int main() {
    // Initialize GLFW
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "QvantumFish - Quantum Visualization Dashboard", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // Set framebuffer size callback
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize Scene Controller with initial window size
    sceneController = new SceneController(windowWidth, windowHeight);

    // Setup GLFW callbacks
    sceneController->setupCallbacks(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 330");

    // Setup ImGui style
    ImGui::StyleColorsDark();

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Initialize the scene
    initializeScene();

    // Initialize background quad
    initializeBackgroundQuad();

    // Initialize division lines
    initializeDivisionLines();

    std::cout << "Controls:" << std::endl;
    std::cout << "  - Mouse drag: Rotate Bloch Sphere view" << std::endl;
    std::cout << "  - Mouse wheel: Zoom in/out Bloch Sphere" << std::endl;
    std::cout << "  - R key: Reset Bloch Sphere view" << std::endl;
    std::cout << "  - ESC key: Exit" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        // Skip rendering if window is minimized to save resources
        if (windowMinimized) {
            glfwWaitEvents();
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        sceneController->processInput(window);

        // Main control panel window
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 250), ImGuiCond_FirstUseEver);

        ImGui::Begin("Quantum Visualization Controls");

        ImGui::Text("View Controls");
        if (ImGui::Button("Reset View")) {
            sceneController->reset();
        }

        ImGui::Separator();
        ImGui::Text("Component Visibility");
        if (topRightQuadrant) {
            // Use local variables bound to the quadrant's state
            static bool showSphere = topRightQuadrant->getShowSphere();
            static bool showAxes = topRightQuadrant->getShowAxes();
            static bool showVector = topRightQuadrant->getShowVector();
            static bool showProjections = topRightQuadrant->getShowProjections();
            static bool showArcs = topRightQuadrant->getShowArcs();
            static float sphereScale = topRightQuadrant->getSphereScale();

            if (ImGui::Checkbox("Show Sphere", &showSphere)) {
                topRightQuadrant->setShowSphere(showSphere);
            }
            if (ImGui::Checkbox("Show Axes", &showAxes)) {
                topRightQuadrant->setShowAxes(showAxes);
            }
            if (ImGui::Checkbox("Show Vector", &showVector)) {
                topRightQuadrant->setShowVector(showVector);
            }
            if (ImGui::Checkbox("Show Projections", &showProjections)) {
                topRightQuadrant->setShowProjections(showProjections);
            }
            if (ImGui::Checkbox("Show Arcs", &showArcs)) {
                topRightQuadrant->setShowArcs(showArcs);
            }

            ImGui::Separator();
            ImGui::Text("Sphere Scale");
            if (ImGui::SliderFloat("Scale", &sphereScale, 0.5f, 2.0f)) {
                topRightQuadrant->setSphereScale(sphereScale);
            }

            // Add a button to toggle all components
            if (ImGui::Button("Toggle All")) {
                topRightQuadrant->toggleAllComponents();
                // Update local variables after toggle
                showSphere = topRightQuadrant->getShowSphere();
                showAxes = topRightQuadrant->getShowAxes();
                showVector = topRightQuadrant->getShowVector();
                showProjections = topRightQuadrant->getShowProjections();
                showArcs = topRightQuadrant->getShowArcs();
            }
        }

        ImGui::Separator();
        ImGui::Text("Quadrant Layout");
        ImGui::BulletText("Top-right: Bloch Sphere");
        ImGui::BulletText("Right mouse key and move to rotate the sphere");
        ImGui::BulletText("Right mouse key and scroll with mouse wheel to zoom in and out");
        ImGui::BulletText("R key to reset the view");
        ImGui::Text("Window Size: %d x %d", windowWidth, windowHeight);

        // Display current vector position
        if (topRightQuadrant) {
            glm::vec3 vectorPos = topRightQuadrant->getVectorPosition();
            ImGui::Separator();
            ImGui::Text("Current Qubit State:");
            ImGui::Text("Position: (%.3f, %.3f, %.3f)", vectorPos.x, vectorPos.y, vectorPos.z);

            // Calculate angles
            float theta = acos(vectorPos.z / glm::length(vectorPos));
            float phi = atan2(vectorPos.y, vectorPos.x);
            ImGui::Text("Theta: %.2f°", theta * 180.0f / M_PI);
            ImGui::Text("Phi: %.2f°", phi * 180.0f / M_PI);
        }

        ImGui::End();

        // Demo window (optional)
        if (showDemoWindow) {
            ImGui::ShowDemoWindow(&showDemoWindow);
        }

        // Clear the entire window with black background ONCE at the beginning
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = static_cast<float>(glfwGetTime());

        // Render the 4 quadrants - each one only clears depth buffer, not color buffer
        renderBottomLeftQuadrant();
        renderBottomRightQuadrant();
        renderTopLeftQuadrant();
        renderTopRightQuadrant(time);

        // Render division lines (must be done after all viewport rendering)
        renderDivisionLines(time);

        // Render ImGui on top of everything
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    cleanupScene();
    glfwTerminate();
    return 0;
}