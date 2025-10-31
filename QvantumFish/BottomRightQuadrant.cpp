#include "BottomRightQuadrant.h"
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <iostream>

BottomRightQuadrant::BottomRightQuadrant()
    : backgroundColor(glm::vec3(0.15f, 0.15f, 0.15f)) {
}

BottomRightQuadrant::~BottomRightQuadrant() {
    // Cleanup if needed
}

void BottomRightQuadrant::initialize() {
    std::cout << "BottomRightQuadrant initialized." << std::endl;
}


void BottomRightQuadrant::render(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    glClear(GL_DEPTH_BUFFER_BIT);

    // Convert Y coordinate for ImGui (from bottom-left to top-left coordinate system)
    int screenHeight = ImGui::GetIO().DisplaySize.y;
    int imguiY = screenHeight - viewportY - viewportHeight;

    ImGui::SetNextWindowPos(ImVec2(viewportX, imguiY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(viewportWidth, viewportHeight));

    ImGui::Begin("Fixed Window", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        ImGui::Text("Quantum State Information");
    }
    ImGui::End();
}