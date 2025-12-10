#define _USE_MATH_DEFINES

#include "TopRightQuadrant.h"
#include <iostream>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>
#include <complex>
#include <imgui.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

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
    showProjections(true), showArcs(true), sphereScale(1.0f),
    settingsWindowOpen(false), windowWidth(1200), windowHeight(800) {
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
    currentQubit = Qubit::ketZero();
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

//the 0+ 20 is to fix the position
void TopRightQuadrant::showSettingsWindow(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    if (!settingsWindowOpen) return;

    // Position the settings window in the top-right of the quadrant
    ImGui::SetNextWindowPos(ImVec2(viewportX + viewportWidth - 320, 0 + 20), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(310, 300), ImGuiCond_Always);

    ImGui::Begin("Bloch Sphere Settings", &settingsWindowOpen,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse);

    ImGui::Text("Component Visibility");
    ImGui::Separator();

    if (ImGui::Checkbox("Show Sphere", &showSphere)) {
        // Checkbox automatically updates showSphere
    }
    if (ImGui::Checkbox("Show Axes", &showAxes)) {
        // Checkbox automatically updates showAxes
    }
    if (ImGui::Checkbox("Show Vector", &showVector)) {
        // Checkbox automatically updates showVector
    }
    if (ImGui::Checkbox("Show Projections", &showProjections)) {
        // Checkbox automatically updates showProjections
    }
    if (ImGui::Checkbox("Show Arcs", &showArcs)) {
        // Checkbox automatically updates showArcs
    }

    ImGui::Separator();
    ImGui::Text("Sphere Scale");
    if (ImGui::SliderFloat("Scale", &sphereScale, 0.5f, 2.0f, "%.2f")) {
        // Slider automatically updates sphereScale
    }

    ImGui::Separator();
    if (ImGui::Button("Toggle All Components")) {
        toggleAllComponents();
    }

    ImGui::Separator();
    ImGui::Text("Controls:");
    ImGui::BulletText("Right-click + drag: Rotate");
    ImGui::BulletText("Right-click + scroll: Zoom");

    ImGui::End();
}

void TopRightQuadrant::renderSettingsIcon(float time, int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Calculate icon position in screen coordinates (top-right corner)
    int iconX = viewportX + viewportWidth - 40;
    int iconY = 0 + 20;

    // Save current ImGui state
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    // Create a small window for the settings icon
    ImGui::SetNextWindowPos(ImVec2(iconX, iconY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(30, 30), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.0f); // Transparent background

    ImGui::Begin("##SettingsIcon", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoBackground |
        ImGuiWindowFlags_NoFocusOnAppearing);

    // Draw settings icon (gear icon)
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float sz = 24.0f;
    ImVec2 center = ImVec2(p.x + sz * 0.5f, p.y + sz * 0.5f);

    // Draw gear icon with subtle pulse effect
    float pulse = 0.8f + 0.2f * sin(time * 2.0f);
    ImU32 gearColor = settingsWindowOpen ?
        IM_COL32(100, 200, 255, 255) :
        IM_COL32(static_cast<int>(200 * pulse), static_cast<int>(200 * pulse), static_cast<int>(200 * pulse), 255);

    draw_list->AddCircle(center, sz * 0.5f, gearColor, 12, 2.0f);

    // Draw gear teeth
    for (int i = 0; i < 8; i++) {
        float angle = i * (3.14159f / 4.0f);
        ImVec2 inner = ImVec2(
            center.x + cos(angle) * sz * 0.3f,
            center.y + sin(angle) * sz * 0.3f
        );
        ImVec2 outer = ImVec2(
            center.x + cos(angle) * sz * 0.5f,
            center.y + sin(angle) * sz * 0.5f
        );
        draw_list->AddLine(inner, outer, gearColor, 2.0f);
    }

    // Center circle
    draw_list->AddCircleFilled(center, sz * 0.15f, gearColor);

    // Make the entire icon clickable
    ImGui::SetCursorPos(ImVec2(0, 0));
    ImGui::InvisibleButton("settings_button", ImVec2(sz, sz));
    if (ImGui::IsItemClicked()) {
        toggleSettingsWindow();
    }

    // Add tooltip
    if (ImGui::IsItemHovered()) {
        ImGui::SetTooltip("Bloch Sphere Settings");
    }

    ImGui::End();

    // Restore ImGui state
    ImGui::PopStyleVar(3);

    // Show settings window if open
    if (settingsWindowOpen) {
        showSettingsWindow(viewportX, viewportY, viewportWidth, viewportHeight);
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