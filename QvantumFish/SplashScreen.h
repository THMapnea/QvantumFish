#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <string>
#include <vector>
#include <chrono>

class SplashScreen {
private:
    std::vector<std::string> asciiArt;
    float animationTime;
    bool animationComplete;
    std::chrono::steady_clock::time_point startTime;

    void initializeASCIIArt();

public:
    SplashScreen();
    void start();
    void render();
    bool isComplete() const;
};

#endif