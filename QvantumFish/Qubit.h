#ifndef QUBIT_H
#define QUBIT_H

#include <complex>
#include <iostream>


/*

    TODO: change to class approach for better optimization

*/

typedef struct{
    double sphericalX;
    double sphericalY;
    double sphericalZ;
}blochSphereCoordinates;

class Qubit {
private:
    std::complex<double> states[2];


    // Private helper method
    bool checkCorrectInitialization() const;

public:
    // Constructors
    Qubit(const std::complex<double> input_states[2]);

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