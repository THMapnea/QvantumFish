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
#include <thread>
#include <chrono>
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
#include "TopLeftQuadrant.h"
#include "BottomLeftQuadrant.h"
#include "BottomRightQuadrant.h"
#include "SplashScreen.h"

// Global variables
TopRightQuadrant* topRightQuadrant = nullptr;
TopLeftQuadrant* topLeftQuadrant = nullptr;
BottomLeftQuadrant* bottomLeftQuadrant = nullptr;
BottomRightQuadrant* bottomRightQuadrant = nullptr;
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

// Splash screen rendering
unsigned int splashVAO = 0;
unsigned int splashVBO = 0;
unsigned int splashShader = 0;
unsigned int splashTexture = 0;
bool splashScreenComplete = false;
std::chrono::steady_clock::time_point splashStartTime;
SplashScreen splashScreen;

// Current window dimensions
int windowWidth = 1200;
int windowHeight = 800;
bool windowMinimized = false;

// State tracking for qubit controls
static int lastSelectedState = 0;
static float lastCustomTheta = 45.0f;
static float lastCustomPhi = 90.0f;

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
    glDeleteBuffers(1, &backgroundEBO);
}

static void renderBackgroundQuad(const glm::vec3& color, int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Save current viewport
    GLint prevViewport[4];
    glGetIntegerv(GL_VIEWPORT, prevViewport);

    // Set viewport for this specific quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Use background shader
    glUseProgram(backgroundShader);
    glUniform3f(glGetUniformLocation(backgroundShader, "color"), color.r, color.g, color.b);

    // Render the quad
    glBindVertexArray(backgroundVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Restore previous viewport
    glViewport(prevViewport[0], prevViewport[1], prevViewport[2], prevViewport[3]);
}

static void initializeSplashScreen() {
    // Simple quad for splash screen
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

    // Create and compile shader for splash screen
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
    uniform float progress;
    void main() {
        // Pure black background - no animation effects
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
)";

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);

    splashShader = glCreateProgram();
    glAttachShader(splashShader, vertexShader);
    glAttachShader(splashShader, fragmentShader);
    glLinkProgram(splashShader);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Create VAO, VBO, and EBO for splash
    glGenVertexArrays(1, &splashVAO);
    glGenBuffers(1, &splashVBO);
    unsigned int splashEBO;
    glGenBuffers(1, &splashEBO);

    glBindVertexArray(splashVAO);

    glBindBuffer(GL_ARRAY_BUFFER, splashVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, splashEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glDeleteBuffers(1, &splashEBO);

    // Start splash screen
    splashScreen.start();
    splashStartTime = std::chrono::steady_clock::now();
}

static void renderSplashScreen(float time) {
    if (splashScreenComplete) return;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - splashStartTime).count();
    float progress = std::min(elapsed / splashScreen.getAnimationTime(), 1.0f);

    if (progress >= 1.0f) {
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        splashScreenComplete = true;
        return;
    }

    // Clear the screen with pure black background
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Disable depth test for 2D rendering
    glDisable(GL_DEPTH_TEST);

    // Use splash shader for solid black background
    glUseProgram(splashShader);
    glUniform3f(glGetUniformLocation(splashShader, "color"), 0.0f, 0.0f, 0.0f);
    glUniform1f(glGetUniformLocation(splashShader, "progress"), progress);

    // Render the fullscreen black quad
    glBindVertexArray(splashVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    // Re-enable depth test
    glEnable(GL_DEPTH_TEST);

    // Render ASCII art animation using ImGui
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));

    // Use ImGuiWindowFlags_NoScrollbar to prevent scrollbar calculations
    ImGui::Begin("SplashScreen", nullptr,
        ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoInputs |
        ImGuiWindowFlags_NoBackground);

    // Set the window background to black
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.0f, 0.0f, 0.0f, 1.0f));

    // Get ASCII art from splash screen
    const auto& asciiArt = splashScreen.getASCIIArt();

    // Calculate scaling - make it bigger
    float scale = 1.2f;
    if (windowWidth >= 1920) scale = 1.5f;
    if (windowWidth < 1400) scale = 1.0f;
    if (windowWidth < 1000) scale = 0.8f;

    ImGui::SetWindowFontScale(scale);

    // Use a monospace font for proper ASCII art alignment
    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[0]);

    // Calculate which lines should be fully visible based on progress
    int totalLines = static_cast<int>(asciiArt.size());
    int visibleLines = static_cast<int>(progress * totalLines);

    // Calculate total height of visible ASCII art to center it vertically
    float lineHeight = ImGui::GetTextLineHeightWithSpacing();
    float totalTextHeight = totalLines * lineHeight;
    float startY = (windowHeight - totalTextHeight) * 0.5f;

    // Ensure we don't set cursor outside window boundaries
    startY = std::max(0.0f, startY);
    ImGui::SetCursorPosY(startY);

    // Render ASCII art with sequential line-by-line animation
    for (int i = 0; i < totalLines; i++) {
        // Center each line horizontally with a small offset to the left
        std::string lineToMeasure = asciiArt[i].empty() ? " " : asciiArt[i];
        float lineWidth = ImGui::CalcTextSize(lineToMeasure.c_str()).x;

        // Apply a small left offset (about 2% of window width) to make it appear more centered
        float leftOffset = windowWidth * 0.02f;
        float cursorX = (windowWidth - lineWidth) * 0.7f - leftOffset;

        // Ensure we don't set cursor outside window boundaries
        cursorX = std::max(0.0f, cursorX);
        ImGui::SetCursorPosX(cursorX);

        if (i < visibleLines) {
            // Line is fully visible - show in bright cyan
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", asciiArt[i].c_str());
        }
        else if (i == visibleLines) {
            // Current line being animated - reveal character by character
            const std::string& line = asciiArt[i];
            float lineProgress = (progress * totalLines) - i;
            int charsToShow = static_cast<int>(lineProgress * line.length());

            if (charsToShow > 0) {
                std::string visiblePart = line.substr(0, charsToShow);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", visiblePart.c_str());
            }
            else {
                // Render empty space to maintain layout
                ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 0.0f), " ");
            }
        }
        else {
            // Line not yet started - render as invisible to maintain spacing
            ImGui::TextColored(ImVec4(0.0f, 0.0f, 0.0f, 0.0f), " ");
        }

        // Add a dummy item to properly extend the window boundaries
        if (i == totalLines - 1) {
            ImGui::Dummy(ImVec2(0, 0));
        }
    }

    // Pop the font and style
    ImGui::PopFont();
    ImGui::PopStyleColor();

    ImGui::End();
}

static void initializeScene() {
    // Create and initialize all quadrants
    topRightQuadrant = new TopRightQuadrant();
    topRightQuadrant->initialize();

    topLeftQuadrant = new TopLeftQuadrant();
    topLeftQuadrant->initialize();

    bottomLeftQuadrant = new BottomLeftQuadrant();
    bottomLeftQuadrant->initialize();

    bottomRightQuadrant = new BottomRightQuadrant();
    bottomRightQuadrant->initialize();

    // Connect the bottom right quadrant to the top right quadrant's qubit
    bottomRightQuadrant->setQubit(&topRightQuadrant->getCurrentQubit());

    // Set line width for the entire scene
    glLineWidth(2.0f);

    std::cout << "All quadrants initialized successfully." << std::endl;
    std::cout << "Bottom right quadrant connected to top right quadrant's qubit." << std::endl;
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
    delete topLeftQuadrant;
    delete bottomLeftQuadrant;
    delete bottomRightQuadrant;
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

    // Cleanup splash screen
    if (splashVAO) {
        glDeleteVertexArrays(1, &splashVAO);
        glDeleteBuffers(1, &splashVBO);
        glDeleteProgram(splashShader);
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

static void renderTopLeftQuadrant() {
    if (!topLeftQuadrant || windowMinimized) return;

    // Calculate quadrant dimensions
    int viewportX = 0;
    int viewportY = windowHeight / 2;
    int viewportWidth = windowWidth / 2;
    int viewportHeight = windowHeight / 2;

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set background for this quadrant (with proper viewport)
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(topLeftQuadrant->getBackgroundColor(), viewportX, viewportY, viewportWidth, viewportHeight);
    glEnable(GL_DEPTH_TEST);

    // Render the top-left quadrant content
    topLeftQuadrant->render(viewportX, viewportY, viewportWidth, viewportHeight);
}

static void renderBottomLeftQuadrant() {
    if (!bottomLeftQuadrant || windowMinimized) return;

    // Calculate quadrant dimensions
    int viewportX = 0;
    int viewportY = 0;
    int viewportWidth = windowWidth / 2;
    int viewportHeight = windowHeight / 2;

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set background for this quadrant (with proper viewport)
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(bottomLeftQuadrant->getBackgroundColor(), viewportX, viewportY, viewportWidth, viewportHeight);
    glEnable(GL_DEPTH_TEST);

    // Render the bottom-left quadrant content
    bottomLeftQuadrant->render(viewportX, viewportY, viewportWidth, viewportHeight);
}

static void renderBottomRightQuadrant() {
    if (!bottomRightQuadrant || windowMinimized) return;

    // Calculate quadrant dimensions
    int viewportX = windowWidth / 2;
    int viewportY = 0;
    int viewportWidth = windowWidth / 2;
    int viewportHeight = windowHeight / 2;

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set background for this quadrant (with proper viewport)
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(glm::vec3(0.1f, 0.1f, 0.1f), viewportX, viewportY, viewportWidth, viewportHeight);
    glEnable(GL_DEPTH_TEST);

    // Render the bottom-right quadrant content
    bottomRightQuadrant->render(viewportX, viewportY, viewportWidth, viewportHeight);
}

static void renderTopRightQuadrant(float time) {
    if (!topRightQuadrant || windowMinimized) return;

    // Calculate quadrant dimensions
    int viewportX = windowWidth / 2;
    int viewportY = windowHeight / 2;
    int viewportWidth = windowWidth / 2;
    int viewportHeight = windowHeight / 2;

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Set black background for Bloch sphere quadrant (with proper viewport)
    glDisable(GL_DEPTH_TEST);
    renderBackgroundQuad(glm::vec3(0.0f, 0.0f, 0.0f), viewportX, viewportY, viewportWidth, viewportHeight);
    glEnable(GL_DEPTH_TEST);

    // Render the top-right quadrant (Bloch sphere)
    topRightQuadrant->render(time, sceneController,
        viewportX, viewportY,
        viewportWidth, viewportHeight);
}

static void handleQubitStateChanges() {
    if (!topRightQuadrant || !bottomRightQuadrant) return;

    // Check if state selection changed
    int currentSelectedState = bottomRightQuadrant->getSelectedState();

    if (currentSelectedState != lastSelectedState) {
        Qubit newQubit = Qubit::ketZero();
        switch (currentSelectedState) {
        case 0: newQubit = Qubit::ketZero(); break;
        case 1: newQubit = Qubit::ketOne(); break;
        case 2: newQubit = Qubit::ketPlus(); break;
        case 3: newQubit = Qubit::ketMinus(); break;
        case 4: newQubit = Qubit::ketPlusI(); break;
        case 5: newQubit = Qubit::ketMinusI(); break;
        case 6:
            // Custom state - handled by parameter changes
            break;
        }
        if (currentSelectedState != 6) {
            topRightQuadrant->updateQubitState(newQubit);
            bottomRightQuadrant->setQubit(&topRightQuadrant->getCurrentQubit());
        }
        lastSelectedState = currentSelectedState;
    }

    // Check if custom parameters changed
    if (currentSelectedState == 6) {
        float currentTheta = bottomRightQuadrant->getCustomTheta();
        float currentPhi = bottomRightQuadrant->getCustomPhi();

        if (currentTheta != lastCustomTheta || currentPhi != lastCustomPhi) {
            double theta_rad = currentTheta * M_PI / 180.0;
            double phi_rad = currentPhi * M_PI / 180.0;
            Qubit customQubit(std::cos(theta_rad / 2.0), std::exp(std::complex<double>(0, phi_rad)) * std::sin(theta_rad / 2.0));
            topRightQuadrant->updateQubitState(customQubit);
            bottomRightQuadrant->setQubit(&topRightQuadrant->getCurrentQubit());

            lastCustomTheta = currentTheta;
            lastCustomPhi = currentPhi;
        }
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

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

    // Initialize GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cerr << "Failed to initialize GLAD\n";
        return -1;
    }

    // Now initialize the rest of the application
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Initialize Scene Controller with initial window size
    sceneController = new SceneController(windowWidth, windowHeight);

    // Setup GLFW callbacks
    sceneController->setupCallbacks(window);

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

    // Initialize the scene components
    initializeBackgroundQuad();
    initializeDivisionLines();
    initializeSplashScreen(); // Initialize splash screen first

    // Main application loop
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

        float time = static_cast<float>(glfwGetTime());

        // Render splash screen if not complete
        if (!splashScreenComplete) {
            renderSplashScreen(time);

            // Check if splash screen should complete
            auto now = std::chrono::steady_clock::now();
            float elapsed = std::chrono::duration<float>(now - splashStartTime).count();
            if (elapsed >= splashScreen.getAnimationTime()) {
                splashScreenComplete = true;
                // Initialize the main scene after splash screen
                initializeScene();
            }
        }
        else {
            // Main application rendering
            sceneController->processInput(window);

            // Handle qubit state changes from bottom right quadrant controls
            handleQubitStateChanges();

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
            ImGui::BulletText("Top-left: [Future Content]");
            ImGui::BulletText("Bottom-left: [Future Content]");
            ImGui::BulletText("Bottom-right: Qubit Information & Controls");
            ImGui::BulletText("Right mouse key and move to rotate the sphere");
            ImGui::BulletText("Right mouse key and scroll with mouse wheel to zoom in and out");
            ImGui::BulletText("R key to reset the view");
            ImGui::Text("Window Size: %d x %d", windowWidth, windowHeight);

            ImGui::End();

            // Demo window (optional)
            if (showDemoWindow) {
                ImGui::ShowDemoWindow(&showDemoWindow);
            }

            // Clear the entire window with black background ONCE at the beginning
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // Render the 4 quadrants
            renderBottomLeftQuadrant();
            renderBottomRightQuadrant();
            renderTopLeftQuadrant();
            renderTopRightQuadrant(time);

            // Render division lines (must be done after all viewport rendering)
            renderDivisionLines(time);
        }

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

    std::cout << "QvantumFish application terminated successfully." << std::endl;
    return 0;
}