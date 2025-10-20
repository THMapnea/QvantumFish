#ifndef QUBIT_H
#define QUBIT_H

#include <complex>
#include <iostream>
#include <Eigen/Dense>
#include "BlochSphereCoordinates.h"


/*

    Qubit class

*/
class Qubit {
private:
    //states of the qubit for the alpha|0> + beta|1> representation
    Eigen::Vector2cd states;

    // Private helper method
    bool checkCorrectInitialization() const;

public:
    // Constructors
    Qubit(const Eigen::Vector2cd& input_states);
    Qubit(const std::complex<double>& alpha, const std::complex<double>& beta);

    // Copy constructor
    Qubit(const Qubit& other);

    // Assignment operator
    Qubit& operator=(const Qubit& other);

    // Destructor
    ~Qubit();

    // Getters
    std::complex<double> getAlpha() const;
    std::complex<double> getBeta() const;
    Eigen::Vector2cd getStateVector() const;

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

    //find relative phase
    double findRelativePhase() const;

    //convert to bloch sphere coordinates
    BlochSphereCoordinates getBlochSphereCoordinates() const;

    //density matrix functions
    Eigen::Matrix2cd computeBlochStateDensityMatrix() const;

};

#endif // QUBIT_H