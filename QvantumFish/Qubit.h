#ifndef QUBIT_H
#define QUBIT_H

#include <complex>
#include <array>
#include <iostream>


//personalized class for better coordinates optimization
class blochSphereCoordinates {
    private:
        std::array<double, 3> coordinates;
    
public:
    //we need to be careful here since we are working on the actual data and not the copy
    double& sphericalX() { return coordinates.at(0); }
    double& sphericalY() { return coordinates.at(1); }
    double& sphericalZ() { return coordinates.at(2); }
};

class Qubit {
private:
    //states of the qubit for the alpha|0> + beta|1> representation
    std::array<std::complex<double>, 2> states;


    // Private helper method
    bool checkCorrectInitialization() const;

public:
    // Constructors
    Qubit(const std::array<std::complex<double>, 2> input_states);

    // Copy constructor
    Qubit(const Qubit& other);

    // Assignment operator
    Qubit& operator=(const Qubit& other);

    // Destructor
    ~Qubit();

    // Getters
    std::complex<double> getAlpha() const;
    std::complex<double> getBeta() const;

    // Public member functions
    void look() const;

    // Calculate probability of measuring |0> and |1>
    double probabilityZero() const;
    double probabilityOne() const;

    //static factory
    static Qubit ketOne();
    static Qubit ketZero();
    static Qubit ketPlus();
    static Qubit ketMinus();
    static Qubit ketPlusI();
    static Qubit ketMinusI();

    //find polar angle
    double findPolarAngle() const;

    //find relative phse
    double findRelativePhase() const;

    //convert to bloch sphere coordinates
    blochSphereCoordinates getBlochSphereCoordinates() const;

};

#endif // QUBIT_H