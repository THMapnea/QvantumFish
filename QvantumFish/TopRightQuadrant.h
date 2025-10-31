#ifndef TOP_RIGHT_QUADRANT_H
#define TOP_RIGHT_QUADRANT_H

#include <glm/glm.hpp>
#include "BlochSphere.h"
#include "VectorArrow.h"
#include "CoordinatesAxes.h"
#include "ProjectionLines.h"
#include "AngleArcs.h"
#include "SceneController.h"
#include "Qubit.h"

class TopRightQuadrant {
private:
    // Scene components
    CoordinateAxes* coordinateAxes;
    BlochSphere* blochSphere;
    VectorArrow* quantumVector;
    ProjectionLines* projectionLines;
    AngleArcs* angleArcs;

    // Current qubit state
    Qubit currentQubit;

    // Colors
    glm::vec3 axesColor;
    glm::vec3 vectorColor;
    glm::vec3 projectionColor;
    glm::vec3 arcColor;

    // Visibility flags
    bool showSphere;
    bool showAxes;
    bool showVector;
    bool showProjections;
    bool showArcs;

    float sphereScale;

public:
    TopRightQuadrant();
    ~TopRightQuadrant();

    // Initialization
    void initialize();

    // Rendering
    void render(float time, SceneController* sceneController, int viewportX, int viewportY, int viewportWidth, int viewportHeight);

    // Getters for visibility flags
    bool getShowSphere() const { return showSphere; }
    bool getShowAxes() const { return showAxes; }
    bool getShowVector() const { return showVector; }
    bool getShowProjections() const { return showProjections; }
    bool getShowArcs() const { return showArcs; }
    float getSphereScale() const { return sphereScale; }

    // Setters for visibility flags
    void setShowSphere(bool visible) { showSphere = visible; }
    void setShowAxes(bool visible) { showAxes = visible; }
    void setShowVector(bool visible) { showVector = visible; }
    void setShowProjections(bool visible) { showProjections = visible; }
    void setShowArcs(bool visible) { showArcs = visible; }
    void setSphereScale(float scale) { sphereScale = scale; }

    // Toggle all components
    void toggleAllComponents();

    // Update qubit state
    void updateQubitState(const Qubit& qubit);

    // Get current vector position
    glm::vec3 getVectorPosition() const;

    // Get current qubit (for bottom quadrant)
    const Qubit& getCurrentQubit() const { return currentQubit; }

    void cleanup();
};

#endif // TOP_RIGHT_QUADRANT_H