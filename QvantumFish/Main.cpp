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

// Global variables
BlochSphere* blochSphere = nullptr;
VectorArrow* quantumVector = nullptr;
CoordinateAxes* coordinateAxes = nullptr;
ProjectionLines* projectionLines = nullptr;
AngleArcs* angleArcs = nullptr;
SceneController* sceneController = nullptr;

// ImGui state
bool showDemoWindow = false;
float sphereScale = 1.0f;

// Division lines shader and buffers
unsigned int divisionLinesVAO = 0;
unsigned int divisionLinesVBO = 0;
unsigned int divisionLinesShader = 0;

// Current window dimensions
int windowWidth = 1200;
int windowHeight = 800;

static void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    windowWidth = width;
    windowHeight = height;

    // Update scene controller with new dimensions
    if (sceneController) {
        sceneController->updateWindowSize(width, height);
    }
}

static void initializeScene() {
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
    std::cout << "  Theta (polar angle): " << theta << " rad (" << theta * 180.0f / M_PI << ")" << std::endl;
    std::cout << "  Phi (azimuthal angle): " << phi << " rad (" << phi * 180.0f / M_PI << ")" << std::endl;
    std::cout << "  Vector position: (" << vectorPos.x << ", " << vectorPos.y << ", " << vectorPos.z << ")" << std::endl;
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
    delete angleArcs;
    delete projectionLines;
    delete quantumVector;
    delete blochSphere;
    delete coordinateAxes;
    delete sceneController;

    // Cleanup division lines
    if (divisionLinesVAO) {
        glDeleteVertexArrays(1, &divisionLinesVAO);
        glDeleteBuffers(1, &divisionLinesVBO);
        glDeleteProgram(divisionLinesShader);
    }
}

static void renderDivisionLines(float time) {
    if (divisionLinesShader == 0) return;

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

static void renderBlochSphereView(float time) {
    // Calculate quadrant dimensions based on current window size
    int sphereViewportX = windowWidth / 2;
    int sphereViewportY = windowHeight / 2;
    int sphereViewportWidth = windowWidth / 2;
    int sphereViewportHeight = windowHeight / 2;

    // Set up viewport for top-right quadrant (1/4 of screen)
    glViewport(sphereViewportX, sphereViewportY, sphereViewportWidth, sphereViewportHeight);

    // Get matrices from scene controller
    glm::mat4 view = sceneController->getViewMatrix();
    glm::mat4 projection = sceneController->getProjectionMatrix();
    glm::mat4 model = glm::mat4(1.0f);
    float yaw = sceneController->getYaw();
    float pitch = sceneController->getPitch();

    // Apply scaling to the sphere if needed
    glm::mat4 scaledModel = glm::scale(model, glm::vec3(sphereScale));

    // Render the Bloch Sphere components
    coordinateAxes->render(time, view, projection, scaledModel, yaw, pitch);
    blochSphere->render(time, view, projection, scaledModel, yaw, pitch);
    projectionLines->render(time, view, projection, scaledModel, yaw, pitch);
    angleArcs->render(time, view, projection, scaledModel, yaw, pitch);

    glLineWidth(2.5f);
    quantumVector->render(time, view, projection, scaledModel, yaw, pitch);
    glLineWidth(2.0f);
}

static void renderOtherQuadrants() {
    // Bottom-left quadrant - Placeholder for future content
    glViewport(0, 0, windowWidth / 2, windowHeight / 2);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Bottom-right quadrant - Placeholder for future content
    glViewport(windowWidth / 2, 0, windowWidth / 2, windowHeight / 2);
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Top-left quadrant - Placeholder for future content
    glViewport(0, windowHeight / 2, windowWidth / 2, windowHeight / 2);
    glClearColor(0.12f, 0.12f, 0.12f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
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

    // Initialize division lines
    initializeDivisionLines();

    std::cout << "Controls:" << std::endl;
    std::cout << "  - Mouse drag: Rotate Bloch Sphere view" << std::endl;
    std::cout << "  - Mouse wheel: Zoom in/out Bloch Sphere" << std::endl;
    std::cout << "  - R key: Reset Bloch Sphere view" << std::endl;
    std::cout << "  - ESC key: Exit" << std::endl;

    while (!glfwWindowShouldClose(window)) {
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        sceneController->processInput(window);

        // Main control panel window
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(300, 150), ImGuiCond_FirstUseEver);

        ImGui::Begin("Quantum Visualization Controls");

        ImGui::Text("View Controls");
        if (ImGui::Button("Reset View")) {
            sceneController->reset();
        }

        ImGui::Separator();
        ImGui::Text("Quadrant Layout");
        ImGui::BulletText("Top-right: Bloch Sphere");
        ImGui::BulletText("Right mouse key and\n move to rotate the sphere");
        ImGui::BulletText("Right mouse key and scroll\n with mouse wheel to zoom in and out");
        ImGui::Text("Window Size: %d x %d", windowWidth, windowHeight);

        ImGui::End();

        // Clear the entire window with black background
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        float time = static_cast<float>(glfwGetTime());

        // Render the 4 quadrants
        renderOtherQuadrants();
        renderBlochSphereView(time);

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