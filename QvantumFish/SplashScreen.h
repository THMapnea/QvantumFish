#ifndef SPLASHSCREEN_H
#define SPLASHSCREEN_H

#include <vector>
#include <string>
#include <chrono>

class SplashScreen {
public:
    SplashScreen();
    void start();
    void render();
    bool isComplete() const;
    float getAnimationTime() const { return animationTime; }
    const std::vector<std::string>& getASCIIArt() const { return asciiArt; }

private:
    void initializeASCIIArt();

    std::vector<std::string> asciiArt;
    std::chrono::steady_clock::time_point startTime;
    float animationTime;
    bool animationComplete;
};

#endif // SPLASHSCREEN_H