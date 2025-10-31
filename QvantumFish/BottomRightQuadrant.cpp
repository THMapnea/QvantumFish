#define _USE_MATH_DEFINES

#include "BottomRightQuadrant.h"
#include <glad/glad.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_glfw.h>
#include <ImGui/imgui_impl_opengl3.h>
#include <iostream>
#include <cmath>
#include <complex>

BottomRightQuadrant::BottomRightQuadrant()
    : backgroundColor(glm::vec3(0.15f, 0.15f, 0.15f)), currentQubit(nullptr),
    selectedState(0), customTheta(45.0f), customPhi(90.0f) {
}

BottomRightQuadrant::~BottomRightQuadrant() {
    // Cleanup if needed
}

void BottomRightQuadrant::initialize() {
    std::cout << "BottomRightQuadrant initialized." << std::endl;
}

void BottomRightQuadrant::setQubit(const Qubit* qubit) {
    currentQubit = qubit;
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

    // Calculate split: 1/3 for controls (top), 2/3 for information (bottom)
    int controlsHeight = viewportHeight / 3;
    int infoHeight = viewportHeight * 2 / 3;
    int infoY = imguiY + controlsHeight;

    // Controls window (top 1/3)
    ImGui::SetNextWindowPos(ImVec2(viewportX, imguiY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(viewportWidth, controlsHeight));

    ImGui::Begin("Qubit State Controls", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        displayQubitControls();
    }
    ImGui::End();

    // Information window (bottom 2/3)
    ImGui::SetNextWindowPos(ImVec2(viewportX, infoY), ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2(viewportWidth, infoHeight));

    ImGui::Begin("Qubit Information", nullptr,
        ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar);
    {
        if (currentQubit) {
            displayQubitInformation();
        }
        else {
            ImGui::Text("No qubit data available");
            ImGui::Text("Please initialize a qubit state");
        }
    }
    ImGui::End();
}

void BottomRightQuadrant::displayQubitControls() {
    ImGui::Text("QUANTUM STATE CONTROLS");
    ImGui::Separator();

    ImGui::Text("Predefined States:");
    const char* states[] = { "|0>", "|1>", "|+>", "|->", "|+i>", "|-i>", "Custom" };

    if (ImGui::Combo("State", &selectedState, states, IM_ARRAYSIZE(states))) {
        // State changed - main application will detect this change
    }

    if (selectedState == 6) {
        ImGui::Separator();
        ImGui::Text("Custom State Parameters:");

        if (ImGui::SliderFloat("Theta (degrees)", &customTheta, 0.0f, 180.0f, "%.1f")) {
            // Theta changed - main application will detect this change
        }
        if (ImGui::SliderFloat("Phi (degrees)", &customPhi, 0.0f, 360.0f, "%.1f")) {
            // Phi changed - main application will detect this change
        }

        // Display the current custom state equation
        ImGui::Text("Custom State: |psi> = cos(%.1f)|0> + e^(i%.1f)sin(%.1f)|1>",
            customTheta / 2, customPhi, customTheta / 2);
    }

    // Display current state information in controls section
    if (currentQubit) {
        ImGui::Separator();
        // Display current state type
        const char* currentStateName = "";
        switch (selectedState) {
        case 0: currentStateName = "|0>"; break;
        case 1: currentStateName = "|1>"; break;
        case 2: currentStateName = "|+>"; break;
        case 3: currentStateName = "|->"; break;
        case 4: currentStateName = "|+>>"; break;
        case 5: currentStateName = "|->>"; break;
        case 6: currentStateName = "Custom"; break;
        }
        ImGui::Text("Current State: %s", currentStateName);
    }
}

void BottomRightQuadrant::displayQubitInformation() {
    // Get qubit state information
    std::complex<double> alpha = currentQubit->getAlpha();
    std::complex<double> beta = currentQubit->getBeta();

    double probZero = currentQubit->probabilityZero();
    double probOne = currentQubit->probabilityOne();

    double theta = currentQubit->findPolarAngle();
    double phi = currentQubit->findRelativePhase();

    BlochSphereCoordinates coords = currentQubit->getBlochSphereCoordinates();

    // Display state vector information
    ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.2f, 1.0f), "QUANTUM STATE VECTOR");
    ImGui::Separator();

    // Display state in Dirac notation
    ImGui::Text("State: |psi> = a|0> + b|1>");

    // Format complex numbers for display
    char alphaReal[64], alphaImag[64], betaReal[64], betaImag[64];
    snprintf(alphaReal, sizeof(alphaReal), "%.4f", alpha.real());
    snprintf(alphaImag, sizeof(alphaImag), "%.4f", std::abs(alpha.imag()));
    snprintf(betaReal, sizeof(betaReal), "%.4f", beta.real());
    snprintf(betaImag, sizeof(betaImag), "%.4f", std::abs(beta.imag()));

    ImGui::Text("a = %s %s i", alphaReal, alpha.imag() >= 0 ? "+" : "-");
    if (std::abs(alpha.imag()) > 1e-10) {
        ImGui::SameLine();
        ImGui::Text("%s", alphaImag);
    }

    ImGui::Text("b = %s %s i", betaReal, beta.imag() >= 0 ? "+" : "-");
    if (std::abs(beta.imag()) > 1e-10) {
        ImGui::SameLine();
        ImGui::Text("%s", betaImag);
    }

    ImGui::Spacing();
    ImGui::Spacing();

    // Display measurement probabilities
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "MEASUREMENT PROBABILITIES");
    ImGui::Separator();

    ImGui::Text("P(|0>) = %.4f (%.1f%%)", probZero, probZero * 100.0);
    ImGui::ProgressBar(probZero, ImVec2(-1, 20), "");

    ImGui::Text("P(|1>) = %.4f (%.1f%%)", probOne, probOne * 100.0);
    ImGui::ProgressBar(probOne, ImVec2(-1, 20), "");

    ImGui::Spacing();
    ImGui::Spacing();

    // Display Bloch sphere coordinates
    ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.8f, 1.0f), "BLOCH SPHERE COORDINATES");
    ImGui::Separator();

    ImGui::Text("Spherical Coordinates:");
    ImGui::Text("  Polar angle (theta): %.4f rad (%.1f)", theta, theta * 180.0 / M_PI);
    ImGui::Text("  Azimuthal angle (phi): %.4f rad (%.1f)", phi, phi * 180.0 / M_PI);

    ImGui::Spacing();

    ImGui::Text("Cartesian Coordinates:");
    ImGui::Text("  X: %.4f", coords.sphericalX());
    ImGui::Text("  Y: %.4f", coords.sphericalY());
    ImGui::Text("  Z: %.4f", coords.sphericalZ());

    ImGui::Spacing();
    ImGui::Spacing();

    // Display state equation
    ImGui::TextColored(ImVec4(0.2f, 0.8f, 0.2f, 1.0f), "STATE REPRESENTATION");
    ImGui::Separator();

    char stateEq[256];
    snprintf(stateEq, sizeof(stateEq),
        "|psi> = cos(theta/2)|0> + e^(i phi)sin(theta/2)|1>\n"
        "    = cos(%.1f)|0> + e^(i%.1f)sin(%.1f)|1>",
        theta * 90.0 / M_PI, phi * 180.0 / M_PI, theta * 90.0 / M_PI);
    ImGui::TextWrapped("%s", stateEq);

    // Display normalization check
    ImGui::Spacing();
    ImGui::Spacing();
    double totalProb = probZero + probOne;
    ImGui::TextColored(totalProb > 0.999 && totalProb < 1.001 ?
        ImVec4(0.2f, 0.8f, 0.2f, 1.0f) : ImVec4(1.0f, 0.2f, 0.2f, 1.0f),
        "Normalization: |a|^2 + |b|^2 = %.6f", totalProb);

    // Additional quantum information
    ImGui::Spacing();
    ImGui::Spacing();
    ImGui::TextColored(ImVec4(0.8f, 0.5f, 0.2f, 1.0f), "QUANTUM PROPERTIES");
    ImGui::Separator();

    // Calculate and display superposition information
    if (probZero > 0.45 && probZero < 0.55 && probOne > 0.45 && probOne < 0.55) {
        ImGui::Text("State is in equal superposition");
    }

    // Display phase information
    if (std::abs(phi) < 1e-10) {
        ImGui::Text("Phase: Real (phi circa 0)");
    }
    else if (std::abs(phi - M_PI) < 1e-10) {
        ImGui::Text("Phase: Negative real (phi = pi)");
    }
    else if (std::abs(phi - M_PI / 2) < 1e-10) {
        ImGui::Text("Phase: Imaginary (phi = pi/2)");
    }
    else if (std::abs(phi - 3 * M_PI / 2) < 1e-10) {
        ImGui::Text("Phase: Negative imaginary (phi = 3pi/2)");
    }
    else {
        ImGui::Text("Phase: Complex (phi = %.2f rad)", phi);
    }
}