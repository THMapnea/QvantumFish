#include "TopLeftQuadrant.h"
#include <glad/glad.h>
#include <iostream>
#include <imgui.h>
#include <GLFW/glfw3.h>

TopLeftQuadrant::TopLeftQuadrant()
    : backgroundColor(glm::vec3(0.12f, 0.12f, 0.12f)),
    showLineNumbers(true),
    cursorPosition(0),
    scrollPosition(0),
    textModified(false),
    inputActive(false) {

    // Initialize input buffer
    inputBuffer[0] = '\0';

    // Initialize with just the editor name
    textLines.push_back("QVantumFishEditor");
}

TopLeftQuadrant::~TopLeftQuadrant() {
    // Cleanup if needed
}

void TopLeftQuadrant::initialize() {
    std::cout << "TopLeftQuadrant initialized with VS Code style text editor." << std::endl;
}

void TopLeftQuadrant::render(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    // Skip rendering if viewport dimensions are invalid
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    // Set up viewport for this quadrant
    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);

    // Clear only depth buffer for this quadrant
    glClear(GL_DEPTH_BUFFER_BIT);

    // Render the text editor using ImGui
    renderTextEditor();
}

void TopLeftQuadrant::updateCurrentLine() {
    if (cursorPosition >= 0 && cursorPosition < (int)textLines.size() && textModified) {
        textLines[cursorPosition] = inputBuffer;
        textModified = false;
    }
}

void TopLeftQuadrant::renderTextEditor() {
    // Set up the text editor window to fill the entire quadrant
    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x / 2, ImGui::GetIO().DisplaySize.y / 2));

    // Use the same dark theme as the rest of the application
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_MenuBarBg, ImVec4(0.12f, 0.12f, 0.12f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

    ImGui::Begin("QVantumFish Editor", nullptr,
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_MenuBar);

    // Menu bar - simplified to match application style
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                textLines.clear();
                textLines.push_back("");
                cursorPosition = 0;
                textModified = false;
                inputBuffer[0] = '\0';
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                std::cout << "Save functionality placeholder" << std::endl;
                textModified = false;
            }
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit")) {
            ImGui::MenuItem("Line Numbers", nullptr, &showLineNumbers);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Text editor area with application-consistent styling
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f)); // Cyan text to match app theme
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

    float textAreaHeight = ImGui::GetContentRegionAvail().y - 25;
    if (textAreaHeight < 100) textAreaHeight = 100;

    ImGui::BeginChild("TextArea", ImVec2(0, textAreaHeight), true,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Handle keyboard input
    handleInput();

    // Display line numbers if enabled
    float lineNumberWidth = showLineNumbers ? 50.0f : 0.0f;

    if (showLineNumbers) {
        ImGui::BeginChild("LineNumbers", ImVec2(lineNumberWidth, 0), false);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        for (size_t i = 0; i < textLines.size(); i++) {
            std::string lineNum = std::to_string(i + 1);
            float textWidth = ImGui::CalcTextSize(lineNum.c_str()).x;
            ImGui::SetCursorPosX(lineNumberWidth - textWidth - 8);
            ImGui::Text("%s", lineNum.c_str());
        }

        ImGui::PopStyleColor();
        ImGui::EndChild();
        ImGui::SameLine();

        // Separator line
        ImVec2 separatorStart = ImGui::GetCursorScreenPos();
        ImVec2 separatorEnd = ImVec2(separatorStart.x, separatorStart.y + ImGui::GetWindowHeight());
        ImGui::GetWindowDrawList()->AddLine(separatorStart, separatorEnd, ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 1.0f)), 1.0f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8);
    }

    // Text content area
    ImGui::BeginChild("TextContent", ImVec2(0, 0), false);

    // Display all text lines
    for (size_t i = 0; i < textLines.size(); i++) {
        // Highlight current line
        if (i == (size_t)cursorPosition) {
            ImVec2 lineStart = ImGui::GetCursorScreenPos();
            ImVec2 lineEnd = ImVec2(lineStart.x + ImGui::GetWindowWidth(), lineStart.y + ImGui::GetTextLineHeight());
            ImGui::GetWindowDrawList()->AddRectFilled(lineStart, lineEnd, ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.15f, 1.0f)));
        }

        // Display the line
        ImGui::TextUnformatted(textLines[i].c_str());

        // Draw cursor for current line
        if (i == (size_t)cursorPosition && inputActive) {
            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            cursorPos.y -= ImGui::GetTextLineHeight();
            float cursorX = cursorPos.x + ImGui::CalcTextSize(textLines[i].c_str()).x;
            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(cursorX, cursorPos.y),
                ImVec2(cursorX, cursorPos.y + ImGui::GetTextLineHeight()),
                ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 1.0f, 1.0f)), // Cyan cursor
                2.0f
            );
        }
    }

    // Input box for editing current line - SIMPLIFIED APPROACH
    if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
        // Only update buffer if we're not currently editing
        if (!inputActive) {
            strncpy_s(inputBuffer, textLines[cursorPosition].c_str(), sizeof(inputBuffer) - 1);
            inputBuffer[sizeof(inputBuffer) - 1] = '\0';
        }

        ImGui::SetKeyboardFocusHere();
        ImGui::PushItemWidth(-1);

        // Track if input is active
        bool enterPressed = ImGui::InputText("##TextInput", inputBuffer, sizeof(inputBuffer),
            ImGuiInputTextFlags_EnterReturnsTrue);

        inputActive = ImGui::IsItemActive();

        if (enterPressed) {
            // Update the line and move to next line
            updateCurrentLine();
            if (cursorPosition < (int)textLines.size() - 1) {
                cursorPosition++;
            }
            else {
                textLines.push_back("");
                cursorPosition++;
            }
            inputActive = false;
        }
        else if (inputActive) {
            // Real-time updates while typing
            textLines[cursorPosition] = inputBuffer;
            textModified = true;
        }

        ImGui::PopItemWidth();
    }

    ImGui::EndChild();
    ImGui::EndChild();

    // Status bar
    ImGui::PopStyleColor(5); // Pop text area colors
    ImGui::PopStyleColor(3); // Pop window colors

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::BeginChild("StatusBar", ImVec2(0, 25), false);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::SetCursorPosY(5);
    ImGui::SetCursorPosX(10);
    ImGui::Text("Lines: %zu", textLines.size());

    ImGui::SameLine();
    ImGui::SetCursorPosX(100);
    ImGui::Text("Ln %d", cursorPosition + 1);

    ImGui::SameLine();
    if (textModified) {
        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Modified");
    }

    ImGui::PopStyleColor();
    ImGui::EndChild();
    ImGui::PopStyleColor();

    ImGui::End();
}

void TopLeftQuadrant::handleInput() {
    ImGuiIO& io = ImGui::GetIO();

    // Only handle navigation keys when not actively typing
    if (!inputActive) {
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
            updateCurrentLine();
            cursorPosition++;
            if (cursorPosition >= (int)textLines.size()) {
                cursorPosition = (int)textLines.size() - 1;
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
            updateCurrentLine();
            cursorPosition--;
            if (cursorPosition < 0) {
                cursorPosition = 0;
            }
        }
        else if ((ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) && !inputActive) {
            updateCurrentLine();
            // Insert new line at current position
            textLines.insert(textLines.begin() + cursorPosition + 1, "");
            cursorPosition++;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Backspace) && !inputActive) {
            if (cursorPosition > 0 && textLines.size() > 1) {
                textLines.erase(textLines.begin() + cursorPosition);
                cursorPosition--;
                if (cursorPosition < 0) cursorPosition = 0;
            }
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_Tab) && !inputActive) {
            if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
                textLines[cursorPosition] = "    " + textLines[cursorPosition];
                textModified = true;
            }
        }
    }

    // Handle Ctrl combinations (work regardless of input state)
    if (io.KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_N)) {
            textLines.clear();
            textLines.push_back("");
            cursorPosition = 0;
            textModified = false;
            inputBuffer[0] = '\0';
            inputActive = false;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_S)) {
            updateCurrentLine();
            std::cout << "Save placeholder - Content has " << textLines.size() << " lines" << std::endl;
            textModified = false;
        }
    }
}