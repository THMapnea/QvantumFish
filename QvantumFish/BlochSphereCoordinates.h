#ifndef BLOCH_SPHERE_COORDINATES_H
#define BLOCH_SPHERE_COORDINATES_H

#include <array>

//personalized class for better coordinates optimization
class BlochSphereCoordinates {
private:
    std::array<double, 3> coordinates;

public:
    //we need to be careful here since we are working on the actual data and not the copy
    double& sphericalX() { return coordinates.at(0); }
    double& sphericalY() { return coordinates.at(1); }
    double& sphericalZ() { return coordinates.at(2); }
};


#endif // !BLOCH_SPHERE_COORDINATES_H