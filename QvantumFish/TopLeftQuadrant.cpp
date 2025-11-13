#include "TopLeftQuadrant.h"
#include <glad/glad.h>
#include <iostream>
#include <imgui.h>
#include <GLFW/glfw3.h>

TopLeftQuadrant::TopLeftQuadrant()
    : backgroundColor(glm::vec3(0.12f, 0.12f, 0.12f)),
    showLineNumbers(true),
    cursorPosition(0),
    textModified(false),
    inputActive(false),
    cursorColumn(0) {

    textLines.push_back("QVantumFishEditor");
    textLines.push_back("// Welcome to your code editor");
    textLines.push_back("// Start typing your code here...");
}

void TopLeftQuadrant::initialize() {
    std::cout << "TopLeftQuadrant initialized with text editor functionality." << std::endl;
}

void TopLeftQuadrant::render(int viewportX, int viewportY, int viewportWidth, int viewportHeight) {
    if (viewportWidth <= 0 || viewportHeight <= 0) {
        return;
    }

    glViewport(viewportX, viewportY, viewportWidth, viewportHeight);
    glClear(GL_DEPTH_BUFFER_BIT);

    renderTextEditor();
}

void TopLeftQuadrant::renderTextEditor() {
    const ImGuiIO& io = ImGui::GetIO();
    const float displayWidth = io.DisplaySize.x;
    const float displayHeight = io.DisplaySize.y;

    ImGui::SetNextWindowPos(ImVec2(0, 0));
    ImGui::SetNextWindowSize(ImVec2(displayWidth / 2, displayHeight / 2));

    // Dark theme styling
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

    // Text editor area styling
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.06f, 0.06f, 0.06f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.9f, 0.9f, 0.9f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(0.08f, 0.08f, 0.08f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));

    const float textAreaHeight = std::max(ImGui::GetContentRegionAvail().y - 25.0f, 100.0f);

    // MAIN TEXT AREA - Only this should have scrolling
    ImGui::BeginChild("TextArea", ImVec2(0, textAreaHeight), true,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_AlwaysVerticalScrollbar);

    // Create a horizontal layout for line numbers and text content
    ImGui::BeginGroup();

    // Line numbers panel - NO SCROLLING, synchronized with main scroll
    if (showLineNumbers) {
        ImGui::BeginChild("LineNumbers", ImVec2(50, 0), false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.5f, 0.5f, 0.5f, 1.0f));

        for (size_t i = 0; i < textLines.size(); ++i) {
            const std::string lineNum = std::to_string(i + 1);
            const float textWidth = ImGui::CalcTextSize(lineNum.c_str()).x;
            ImGui::SetCursorPosX(50.0f - textWidth - 8.0f);

            if (i == static_cast<size_t>(cursorPosition)) {
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
        const ImVec2 separatorStart = ImGui::GetCursorScreenPos();
        const ImVec2 separatorEnd = ImVec2(separatorStart.x, separatorStart.y + ImGui::GetWindowHeight());
        ImGui::GetWindowDrawList()->AddLine(separatorStart, separatorEnd,
            ImGui::GetColorU32(ImVec4(0.3f, 0.3f, 0.3f, 1.0f)), 1.0f);
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 8.0f);
    }

    // Text content area - SCROLLING ENABLED (but synchronized with parent)
    ImGui::BeginChild("TextContent", ImVec2(0, 0), false,
        ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

    handleInput();

    // Display text lines with syntax highlighting and cursor
    for (size_t i = 0; i < textLines.size(); ++i) {
        // Highlight current line background
        if (i == static_cast<size_t>(cursorPosition)) {
            const float scrollX = ImGui::GetScrollX();
            const ImVec2 lineStart = ImVec2(ImGui::GetCursorScreenPos().x - scrollX, ImGui::GetCursorScreenPos().y);
            const ImVec2 lineEnd = ImVec2(lineStart.x + ImGui::GetContentRegionAvail().x + scrollX,
                lineStart.y + ImGui::GetTextLineHeight());
            ImGui::GetWindowDrawList()->AddRectFilled(lineStart, lineEnd,
                ImGui::GetColorU32(ImVec4(0.15f, 0.15f, 0.15f, 1.0f)));
        }

        // Display line text
        ImGui::TextUnformatted(textLines[i].c_str());

        // Draw cursor for current line
        if (i == static_cast<size_t>(cursorPosition) && inputActive) {
            ensureCursorInBounds();

            const std::string textBeforeCursor = textLines[i].substr(0, cursorColumn);
            const float textWidth = ImGui::CalcTextSize(textBeforeCursor.c_str()).x;

            const ImVec2 cursorPos = ImVec2(
                ImGui::GetCursorScreenPos().x + textWidth - ImGui::GetScrollX(),
                ImGui::GetCursorScreenPos().y - ImGui::GetTextLineHeight()
            );

            ImGui::GetWindowDrawList()->AddLine(
                ImVec2(cursorPos.x, cursorPos.y),
                ImVec2(cursorPos.x, cursorPos.y + ImGui::GetTextLineHeight()),
                ImGui::GetColorU32(ImVec4(0.0f, 1.0f, 1.0f, 1.0f)),
                2.0f
            );
        }
    }

    ImGui::EndChild(); // TextContent
    ImGui::EndGroup(); // Horizontal layout

    ImGui::EndChild(); // TextArea (main scrolling area)

    ImGui::PopStyleColor(5); // Text area colors
    ImGui::PopStyleColor(3); // Window colors

    // Status bar - NO SCROLLING
    ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::BeginChild("StatusBar", ImVec2(0, 25), false, ImGuiWindowFlags_NoScrollbar);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.7f, 0.7f, 0.7f, 1.0f));
    ImGui::SetCursorPosY(5.0f);
    ImGui::SetCursorPosX(10.0f);
    ImGui::Text("Lines: %zu", textLines.size());

    ImGui::SameLine();
    ImGui::SetCursorPosX(100.0f);
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
    const ImGuiIO& io = ImGui::GetIO();
    inputActive = ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows);

    if (!inputActive) return;

    // Handle character input
    for (int i = 0; i < io.InputQueueCharacters.Size; ++i) {
        const unsigned int c = io.InputQueueCharacters[i];
        handleCharacterInput(c);
    }

    handleSpecialKeys();
    handleNavigationKeys();
    handleCtrlCombinations();
}

void TopLeftQuadrant::handleCharacterInput(unsigned int c) {
    if (c >= 32 && c < 256 && c != 13) { // Printable characters only, skip Enter
        if (cursorPosition >= 0 && cursorPosition < static_cast<int>(textLines.size())) {
            std::string& line = textLines[cursorPosition];
            if (cursorColumn <= line.length()) {
                line.insert(cursorColumn, 1, static_cast<char>(c));
                cursorColumn++;
                textModified = true;
            }
        }
    }
}

void TopLeftQuadrant::handleSpecialKeys() {
    if (ImGui::IsKeyPressed(ImGuiKey_Backspace)) {
        deleteCharacter(true);
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Enter) || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)) {
        insertNewLine();
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Tab)) {
        if (cursorPosition >= 0 && cursorPosition < static_cast<int>(textLines.size())) {
            textLines[cursorPosition].insert(cursorColumn, "    ");
            cursorColumn += 4;
            textModified = true;
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Delete)) {
        deleteCharacter(false);
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
        if (cursorPosition >= 0 && cursorPosition < static_cast<int>(textLines.size())) {
            if (cursorColumn < textLines[cursorPosition].length()) {
                cursorColumn++;
            }
            else if (cursorPosition < static_cast<int>(textLines.size()) - 1) {
                cursorPosition++;
                cursorColumn = 0;
            }
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_DownArrow)) {
        if (cursorPosition < static_cast<int>(textLines.size()) - 1) {
            cursorPosition++;
            ensureCursorInBounds();
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_UpArrow)) {
        if (cursorPosition > 0) {
            cursorPosition--;
            ensureCursorInBounds();
        }
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_Home)) {
        cursorColumn = 0;
    }
    else if (ImGui::IsKeyPressed(ImGuiKey_End)) {
        if (cursorPosition >= 0 && cursorPosition < static_cast<int>(textLines.size())) {
            cursorColumn = textLines[cursorPosition].length();
        }
    }
}

void TopLeftQuadrant::handleCtrlCombinations() {
    const ImGuiIO& io = ImGui::GetIO();

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

void TopLeftQuadrant::ensureCursorInBounds() {
    if (cursorPosition >= 0 && cursorPosition < static_cast<int>(textLines.size())) {
        if (cursorColumn > textLines[cursorPosition].length()) {
            cursorColumn = textLines[cursorPosition].length();
        }
    }
}

void TopLeftQuadrant::insertNewLine() {
    if (cursorPosition >= 0 && cursorPosition < static_cast<int>(textLines.size())) {
        std::string& currentLine = textLines[cursorPosition];
        const std::string newLine = currentLine.substr(cursorColumn);
        currentLine = currentLine.substr(0, cursorColumn);

        textLines.insert(textLines.begin() + cursorPosition + 1, newLine);
        cursorPosition++;
        cursorColumn = 0;
        textModified = true;
    }
}

void TopLeftQuadrant::deleteCharacter(bool isBackspace) {
    if (cursorPosition < 0 || cursorPosition >= static_cast<int>(textLines.size())) {
        return;
    }

    std::string& line = textLines[cursorPosition];

    if (isBackspace) {
        if (cursorColumn > 0 && !line.empty()) {
            line.erase(cursorColumn - 1, 1);
            cursorColumn--;
            textModified = true;
        }
        else if (cursorColumn == 0 && cursorPosition > 0) {
            // Merge with previous line
            std::string& prevLine = textLines[cursorPosition - 1];
            const size_t prevLineLength = prevLine.length();
            prevLine += line;
            textLines.erase(textLines.begin() + cursorPosition);
            cursorPosition--;
            cursorColumn = prevLineLength;
            textModified = true;
        }
    }
    else { // Delete key
        if (cursorColumn < line.length()) {
            line.erase(cursorColumn, 1);
            textModified = true;
        }
        else if (cursorPosition < static_cast<int>(textLines.size()) - 1) {
            // Merge with next line
            line += textLines[cursorPosition + 1];
            textLines.erase(textLines.begin() + cursorPosition + 1);
            textModified = true;
        }
    }
}