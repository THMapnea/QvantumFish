#ifndef BOTTOMRIGHTQUADRANT_H
#define BOTTOMRIGHTQUADRANT_H

#include <glm/glm.hpp>
#include "Qubit.h"

class BottomRightQuadrant {
private:
    glm::vec3 backgroundColor;
    const Qubit* currentQubit;
    
    void displayQubitInformation();

public:
    BottomRightQuadrant();
    ~BottomRightQuadrant();

    void initialize();
    void render(int viewportX, int viewportY, int viewportWidth, int viewportHeight);
    void setQubit(const Qubit* qubit);
};

#endif // BOTTOMRIGHTQUADRANT_H