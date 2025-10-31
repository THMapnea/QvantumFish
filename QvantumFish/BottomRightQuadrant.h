#ifndef BOTTOMRIGHTQUADRANT_H
#define BOTTOMRIGHTQUADRANT_H

#include <glm/glm.hpp>
#include "Qubit.h"

class BottomRightQuadrant {
private:
    glm::vec3 backgroundColor;
    const Qubit* currentQubit;

    // Qubit state controls
    int selectedState;
    float customTheta;
    float customPhi;

    void displayQubitInformation();
    void displayQubitControls();

public:
    BottomRightQuadrant();
    ~BottomRightQuadrant();

    void initialize();
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);
    void setQubit(const Qubit* qubit);

    // Getters for qubit controls state
    int getSelectedState() const { return selectedState; }
    float getCustomTheta() const { return customTheta; }
    float getCustomPhi() const { return customPhi; }

    // Setters for qubit controls state
    void setSelectedState(int state) { selectedState = state; }
    void setCustomTheta(float theta) { customTheta = theta; }
    void setCustomPhi(float phi) { customPhi = phi; }
};

#endif // BOTTOMRIGHTQUADRANT_H