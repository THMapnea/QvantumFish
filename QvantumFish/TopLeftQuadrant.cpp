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
    inputActive(false),
    cursorColumn(0),
    mouseClicked(false),
    lineHeight(0.0f) {

    // Initialize with editor name
    textLines.push_back("QVantumFishEditor");
}

TopLeftQuadrant::~TopLeftQuadrant() {
    // Cleanup if needed
}

void TopLeftQuadrant::initialize() {
    std::cout << "TopLeftQuadrant initialized with direct text editing." << std::endl;
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

    // Menu bar
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {
                textLines.clear();
                textLines.push_back("");
                cursorPosition = 0;
                cursorColumn = 0;
                textModified = false;
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
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.0f, 1.0f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

    float textAreaHeight = ImGui::GetContentRegionAvail().y - 25;
    if (textAreaHeight < 100) textAreaHeight = 100;

    ImGui::BeginChild("TextArea", ImVec2(0, textAreaHeight), true,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Calculate line height for mouse positioning
    lineHeight = ImGui::GetTextLineHeight();

    // Handle keyboard and mouse input for text editing
    handleInput();
    handleMouseInput();

    // Display line numbers if enabled
    float lineNumberWidth = showLineNumbers ? 50.0f : 0.0f;

    if (showLineNumbers) {
        ImGui::BeginChild("LineNumbers", ImVec2(lineNumberWidth, 0), false);
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        for (size_t i = 0; i < textLines.size(); i++) {
            std::string lineNum = std::to_string(i + 1);
            float textWidth = ImGui::CalcTextSize(lineNum.c_str()).x;
            ImGui::SetCursorPosX(lineNumberWidth - textWidth - 8);

            // Highlight current line number
            if (i == (size_t)cursorPosition) {
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 1.0f, 1.0f), "%s", lineNum.c_str());
            }
            else {
                ImGui::Text("%s", lineNum.c_str());
            }
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
        // Highlight current line background
        if (i == (size_t)cursorPosition) {
            ImVec2 lineStart = ImGui::GetCursorScreenPos();
            ImVec2 lineEnd = ImVec2(lineStart.x + ImGui::GetWindowWidth(), lineStart.y + ImGui::GetTextLineHeight());
            ImGui::GetWindowDrawList()->AddRectFilled(lineStart, lineEnd, ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.15f, 1.0f)));
        }

        // Display the line text
        ImGui::TextUnformatted(textLines[i].c_str());

        // Draw cursor for current line
        if (i == (size_t)cursorPosition && inputActive) {
            // Ensure cursor column is within bounds
            if (cursorColumn > textLines[i].length()) {
                cursorColumn = textLines[i].length();
            }

            // Calculate cursor position based on text width
            std::string textBeforeCursor = textLines[i].substr(0, cursorColumn);
            float textWidth = ImGui::CalcTextSize(textBeforeCursor.c_str()).x;

            ImVec2 cursorPos = ImGui::GetCursorScreenPos();
            cursorPos.y -= ImGui::GetTextLineHeight();
            cursorPos.x += textWidth - ImGui::GetScrollX();

            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(cursorPos.x, cursorPos.y),
                ImVec2(cursorPos.x, cursorPos.y + ImGui::GetTextLineHeight()),
                ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 1.0f, 1.0f)),
                2.0f
            );
        }
    }

    ImGui::EndChild();
    ImGui::EndChild();

    // Status bar
    ImGui::PopStyleColor(5);
    ImGui::PopStyleColor(3);

    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::BeginChild("StatusBar", ImVec2(0, 25), false);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::SetCursorPosY(5);
    ImGui::SetCursorPosX(10);
    ImGui::Text("Lines: %zu", textLines.size());

    ImGui::SameLine();
    ImGui::SetCursorPosX(100);
    ImGui::Text("Ln %d, Col %zu", cursorPosition + 1, cursorColumn + 1);

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

    // Set input as active when the text area is focused
    inputActive = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

    if (!inputActive) return;

    // Handle character input
    for (int i = 0; i < io.InputQueueCharacters.Size; i++) {
        unsigned int c = io.InputQueueCharacters[i];
        handleCharacterInput(c);
    }

    // Handle special keys
    handleSpecialKeys();

    // Handle navigation keys
    handleNavigationKeys();

    // Handle Ctrl combinations
    handleCtrlCombinations();
}

void TopLeftQuadrant::handleMouseInput() {
    ImGuiIO& io = ImGui::GetIO();

    // Get the text content area position and size
    ImVec2 textContentPos = ImGui::GetCursorScreenPos(); // Position of text content area
    ImVec2 textContentSize = ImGui::GetContentRegionAvail(); // Size of text content area

    // Check if mouse is within the text content area
    bool isMouseInTextArea = io.MousePos.x >= textContentPos.x &&
        io.MousePos.x <= textContentPos.x + textContentSize.x &&
        io.MousePos.y >= textContentPos.y &&
        io.MousePos.y <= textContentPos.y + textContentSize.y;

    if (isMouseInTextArea) {
        // Handle left mouse click
        if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            // Calculate relative mouse position within text content area
            float relativeMouseY = io.MousePos.y - textContentPos.y;
            float relativeMouseX = io.MousePos.x - textContentPos.x;

            // Get line and column from mouse position
            int newLine = getLineFromMousePos(relativeMouseY);
            size_t newColumn = getColumnFromMousePos(newLine, relativeMouseX);

            // Update cursor position
            if (newLine >= 0 && newLine < (int)textLines.size()) {
                cursorPosition = newLine;
                cursorColumn = newColumn;
                inputActive = true;
            }
        }

        // Handle mouse wheel scrolling
        if (io.MouseWheel != 0.0f) {
            // You can implement scroll functionality here if needed
            // For now, we'll rely on ImGui's built-in scrolling
        }
    }
}

int TopLeftQuadrant::getLineFromMousePos(float mouseY) {
    if (lineHeight <= 0) return cursorPosition;

    int line = static_cast<int>(mouseY / lineHeight);

    // Clamp line to valid range
    if (line < 0) return 0;
    if (line >= (int)textLines.size()) return (int)textLines.size() - 1;

    return line;
}

size_t TopLeftQuadrant::getColumnFromMousePos(int line, float mouseX) {
    if (line < 0 || line >= (int)textLines.size()) {
        return 0;
    }

    const std::string& currentLine = textLines[line];

    // If the line is empty, cursor goes to position 0
    if (currentLine.empty()) {
        return 0;
    }

    // Find the closest character position based on text width
    float accumulatedWidth = 0.0f;
    size_t closestColumn = 0;
    float minDistance = std::numeric_limits<float>::max();

    // Check each character position
    for (size_t i = 0; i <= currentLine.length(); i++) {
        std::string substring = currentLine.substr(0, i);
        float width = ImGui::CalcTextSize(substring.c_str()).x;

        float distance = std::abs(mouseX - width);
        if (distance < minDistance) {
            minDistance = distance;
            closestColumn = i;
        }
    }

    return closestColumn;
}

void TopLeftQuadrant::handleCharacterInput(unsigned int c) {
    if (c > 0 && c < 256 && c != 13) { // Skip Enter key (handled separately)
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            // Insert character at cursor position
            std::string& line = textLines[cursorPosition];
            if (cursorColumn <= line.length()) {
                line.insert(cursorColumn, 1, (char)c);
                cursorColumn++;
                textModified = true;
            }
        }
    }
}

void TopLeftQuadrant::handleSpecialKeys() {
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            std::string& line = textLines[cursorPosition];
            if (cursorColumn > 0 && !line.empty()) {
                line.erase(cursorColumn - 1, 1);
                cursorColumn--;
                textModified = true;
            }
            else if (cursorColumn == 0 && cursorPosition > 0) {
                // Merge with previous line
                std::string& prevLine = textLines[cursorPosition - 1];
                size_t prevLineLength = prevLine.length();
                prevLine += line;
                textLines.erase(textLines.begin() + cursorPosition);
                cursorPosition--;
                cursorColumn = prevLineLength;
                textModified = true;
            }
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            std::string& currentLine = textLines[cursorPosition];
            std::string newLine = currentLine.substr(cursorColumn);
            currentLine = currentLine.substr(0, cursorColumn);

            textLines.insert(textLines.begin() + cursorPosition + 1, newLine);
            cursorPosition++;
            cursorColumn = 0;
            textModified = true;
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            textLines[cursorPosition].insert(cursorColumn, "    ");
            cursorColumn += 4;
            textModified = true;
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            std::string& line = textLines[cursorPosition];
            if (cursorColumn < line.length()) {
                line.erase(cursorColumn, 1);
                textModified = true;
            }
            else if (cursorPosition < (int)textLines.size() - 1) {
                // Merge with next line
                line += textLines[cursorPosition + 1];
                textLines.erase(textLines.begin() + cursorPosition + 1);
                textModified = true;
            }
        }
    }
}

void TopLeftQuadrant::handleNavigationKeys() {
    if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow)) {
        if (cursorColumn > 0) {
            cursorColumn--;
        }
        else if (cursorPosition > 0) {
            cursorPosition--;
            cursorColumn = textLines[cursorPosition].length();
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) {
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            if (cursorColumn < textLines[cursorPosition].length()) {
                cursorColumn++;
            }
            else if (cursorPosition < (int)textLines.size() - 1) {
                cursorPosition++;
                cursorColumn = 0;
            }
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        if (cursorPosition < (int)textLines.size() - 1) {
            cursorPosition++;
            // Keep cursor column within bounds of new line
            if (cursorColumn > textLines[cursorPosition].length()) {
                cursorColumn = textLines[cursorPosition].length();
            }
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (cursorPosition > 0) {
            cursorPosition--;
            // Keep cursor column within bounds of new line
            if (cursorColumn > textLines[cursorPosition].length()) {
                cursorColumn = textLines[cursorPosition].length();
            }
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
        cursorColumn = 0;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_End)) {
        if (cursorPosition >= 0 && cursorPosition < (int)textLines.size()) {
            cursorColumn = textLines[cursorPosition].length();
        }
    }
}

void TopLeftQuadrant::handleCtrlCombinations() {
    ImGuiIO& io = ImGui::GetIO();

    if (io.KeyCtrl) {
        if (ImGui::IsKeyPressed(ImGuiKey_N)) {
            textLines.clear();
            textLines.push_back("");
            cursorPosition = 0;
            cursorColumn = 0;
            textModified = false;
        }
        else if (ImGui::IsKeyPressed(ImGuiKey_S)) {
            std::cout << "Save placeholder - Content has " << textLines.size() << " lines" << std::endl;
            textModified = false;
        }
    }
}