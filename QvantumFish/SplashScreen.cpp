#include "SplashScreen.h"
#include <iostream>
#include <chrono>
#include <thread>

SplashScreen::SplashScreen()
    : animationTime(5.0f), animationComplete(false) {
    initializeASCIIArt();
}

void SplashScreen::start() {
    startTime = std::chrono::steady_clock::now();
    animationComplete = false;
}

void SplashScreen::initializeASCIIArt() {
    asciiArt = {
            R"(________/\\\___________________________________________________________________________________________________/\\\\\\\\\\\\\\\_____________________/\\\_________\n)",
            R"( _____/\\\\/\\\\_______________________________________________________________________________________________\/\\\///////////_____________________\/\\\_________\n)",
            R"(  ___/\\\//\////\\\_________________________________________________/\\\________________________________________\/\\\______________/\\\______________\/\\\_________\n)",
            R"(   __/\\\______\//\\\__/\\\____/\\\__/\\\\\\\\\_____/\\/\\\\\\____/\\\\\\\\\\\__/\\\____/\\\____/\\\\\__/\\\\\___\/\\\\\\\\\\\_____\///___/\\\\\\\\\\_\/\\\_________\n)",
            R"(    _\//\\\______/\\\__\//\\\__/\\\__\////////\\\___\/\\\////\\\__\////\\\////__\/\\\___\/\\\__/\\\///\\\\\///\\\_\/\\\///////_______/\\\_\/\\\//////__\/\\\\\\\\\\__\n)",
            R"(     __\///\\\\/\\\\/____\//\\\/\\\_____/\\\\\\\\\\__\/\\\__\//\\\____\/\\\______\/\\\___\/\\\_\/\\\_\//\\\__\/\\\_\/\\\_____________\/\\\_\/\\\\\\\\\\_\/\\\/////\\\_\n)",
            R"(      __\///\\\\/\\\\/____\//\\\/\\\_____/\\\\\\\\\\__\/\\\__\//\\\____\/\\\______\/\\\___\/\\\_\/\\\_\//\\\__\/\\\_\/\\\_____________\/\\\_\/\\\\\\\\\\_\/\\\/////\\\_\n)",
            R"(       _______\///\\\\\\_____\//\\\_____\//\\\\\\\\/\\_\/\\\___\/\\\____\//\\\\\___\//\\\\\\\\\__\/\\\__\/\\\__\/\\\_\/\\\_____________\/\\\__/\\\\\\\\\\_\/\\\___\/\\\_\n)",
            R"(        _________\//////_______\///_______\////////\//__\///____\///______\/////_____\/////////___\///___\///___\///__\///______________\///__\//////////__\///____\///__\n)"
            
    };
}

void SplashScreen::render() {
    if (animationComplete) return;

    auto now = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration<float>(now - startTime).count();
    float progress = std::min(elapsed / animationTime, 1.0f);

    if (progress >= 1.0f) {
        animationComplete = true;
        // Clear screen when done
        std::cout << "\033[2J\033[H";
        return;
    }

    // Clear screen and move cursor to top
    std::cout << "\033[2J\033[H";

    // Calculate centering
    int terminalWidth = 120;
    int artWidth = 0;
    for (const auto& line : asciiArt) {
        artWidth = std::max(artWidth, static_cast<int>(line.length()));
    }
    int padding = std::max(0, (terminalWidth - artWidth) / 2);

    // Add some top padding
    for (int i = 0; i < 5; i++) {
        std::cout << std::endl;
    }

    // Calculate which lines should be fully visible based on progress
    int totalLines = static_cast<int>(asciiArt.size());
    int visibleLines = static_cast<int>(progress * totalLines);

    // Render ASCII art with sequential line-by-line animation
    for (int i = 0; i < totalLines; i++) {
        std::string paddingStr(padding, ' ');
        std::cout << paddingStr;

        if (i < visibleLines) {
            // Line is fully visible - show in cyan
            const std::string& line = asciiArt[i];
            std::cout << "\033[38;2;0;200;255m" << line << "\033[0m" << std::endl;
        }
        else if (i == visibleLines) {
            // Current line being animated - reveal character by character
            const std::string& line = asciiArt[i];
            float lineProgress = (progress * totalLines) - i; // Progress within current line (0.0 to 1.0)
            int charsToShow = static_cast<int>(lineProgress * line.length());

            for (int j = 0; j < line.length(); j++) {
                if (j < charsToShow) {
                    std::cout << "\033[38;2;0;200;255m" << line[j] << "\033[0m";
                }
                else {
                    std::cout << " ";
                }
            }
            std::cout << std::endl;
        }
        else {
            // Line not yet started - show as empty space
            std::cout << std::string(asciiArt[i].length(), ' ') << std::endl;
        }
    }

    // Add some bottom padding
    for (int i = 0; i < 3; i++) {
        std::cout << std::endl;
    }

    // Progress bar
    std::string progressPadding(padding, ' ');
    std::cout << progressPadding;
    std::cout << "[";
    int barWidth = 40;
    int pos = static_cast<int>(barWidth * progress);
    for (int i = 0; i < barWidth; i++) {
        if (i < pos) {
            std::cout << "\033[38;2;0;200;255m=\033[0m";
        }
        else if (i == pos) {
            std::cout << ">";
        }
        else {
            std::cout << " ";
        }
    }
    std::cout << "] " << int(progress * 100.0f) << "%";
    std::cout << std::endl;

    // Flush output
    std::cout << std::flush;
}

bool SplashScreen::isComplete() const {
    return animationComplete;
}