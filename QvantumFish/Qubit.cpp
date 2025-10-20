#define _USE_MATH_DEFINES
#include "Qubit.h"
#include <cmath>
#include <Eigen/Dense>
#include <stdexcept>
#include <iostream>
#include "BlochSphereCoordinates.h"

/*

    FUNCTION:   checkCorrectInitialization():
                    check for valid qubit creation
                    to be a valid qubit ||alpha||^2 + ||beta||^2 = 1
                    since we are working with floating point precision we check with an error of 10^-10 since the double have a precison of around 10^-12
*/
bool Qubit::checkCorrectInitialization() const {
    double norm = states.squaredNorm();
    return std::abs(norm - 1.0) < 1e-10;
}

/*

    CONSTRUCTORS

*/
Qubit::Qubit(const Eigen::Vector2cd& input_states) : states(input_states) {
    if (!checkCorrectInitialization()) {
        throw std::invalid_argument("Qubit states do not satisfy normalization condition");
    }
}

Qubit::Qubit(const std::complex<double>& alpha, const std::complex<double>& beta) {
    states << alpha, beta;
    if (!checkCorrectInitialization()) {
        throw std::invalid_argument("Qubit states do not satisfy normalization condition");
    }
}

/*

    COPY CONSTRUCTOR

*/
Qubit::Qubit(const Qubit& other) : states(other.states) {
}

/*

    ASSIGNMENT OPERATOR

*/
Qubit& Qubit::operator=(const Qubit& other) {
    if (this != &other) {
        states = other.states;
    }
    return *this;
}

/*

    DESTRUCTOR

*/
Qubit::~Qubit() {
    std::cout << "Qubit deleted" << std::endl;
}

/*

    GETTERS

*/
std::complex<double> Qubit::getAlpha() const {
    return states(0);
}

std::complex<double> Qubit::getBeta() const {
    return states(1);
}

Eigen::Vector2cd Qubit::getStateVector() const {
    return states;
}

/*

    FUNCTION: look()
                Basic output function to show the qubit's states


*/
void Qubit::look() const {
    std::cout << "the qubit's states are the following: "
        << states(0).real() << "+" << states(0).imag() << "i |0> + "
        << states(1).real() << "+" << states(1).imag() << "i |1>" << std::endl;
}

/*

    FUNCTION: probabilityZero()
                Calculate probability of measuring |0>

*/
double Qubit::probabilityZero() const {
    return std::norm(states(0));
}

/*

    FUNCTION: probabilityOne()
                Calculate probability of measuring |1>

*/
double Qubit::probabilityOne() const {
    return std::norm(states(1));
}

/*

    STATIC FACTORY

*/
//a qubit initialized to |0>
Qubit Qubit::ketZero() {
    Eigen::Vector2cd ket_zero_state;
    ket_zero_state << 1.0, 0.0;
    return Qubit(ket_zero_state);
}

//a qubit initialized to |1>
Qubit Qubit::ketOne() {
    Eigen::Vector2cd ket_one_state;
    ket_one_state << 0.0, 1.0;
    return Qubit(ket_one_state);
}

//a qubit initialized to |+>
Qubit Qubit::ketPlus() {
    double factor = 1.0 / std::sqrt(2.0);
    Eigen::Vector2cd plus_state;
    plus_state << factor, factor;
    return Qubit(plus_state);
}

//a qubit initialized to |->
Qubit Qubit::ketMinus() {
    double factor = 1.0 / std::sqrt(2.0);
    Eigen::Vector2cd minus_state;
    minus_state << factor, -factor;
    return Qubit(minus_state);
}

//a qubit initialized to |+i>
Qubit Qubit::ketPlusI() {
    double factor = 1.0 / std::sqrt(2.0);
    Eigen::Vector2cd plus_i_state;
    plus_i_state << factor, std::complex<double>(0.0, factor);
    return Qubit(plus_i_state);
}

//a qubit initialized to |-i>
Qubit Qubit::ketMinusI() {
    double factor = 1.0 / std::sqrt(2.0);
    Eigen::Vector2cd minus_i_state;
    minus_i_state << factor, std::complex<double>(0.0, -factor);
    return Qubit(minus_i_state);
}

/*

    FUNCTIONS: findPolarAngle() and findRelativePhase() getBlochSphereCoordinates():
                on the bloch sphere a certain state can be represented like it follows

                             |psi> = cos(theta/2)|0> + exp(i*phi)sin(theta/2)|1>

                where theta is a real number defined between [0,pi] and phi is the relative phase
                and is a real number between [0, 2pi]. the following functions return us the angle and the relative phase.
                since we supposed our sphere of radius 1 we can use spheric coordinates and use theta as the polar angle
                while phi is the azimuthal angle to get a representation of a vector on our sphere as (1, theta, phi), therefore
                a point on the bloch sphere will have coordinates defined as follows

                         (x, y, z) ===> (sin(theta)cos(phi), sin(theta)sin(phi), cos(theta))

*/
double Qubit::findPolarAngle() const {
    //no need for normalization since asin goes from 0 to pi/2 and multiplied by two stays in the correct range of [0, pi]
    double polarAngle = 2.0 * asin(std::abs(states(1)));
    return polarAngle;
}

double Qubit::findRelativePhase() const {
    double relativePhase = std::arg(states(1)) - std::arg(states(0));
    //normalize in range [0, 2pi] for visualization on the sphere
    relativePhase = relativePhase - 2 * M_PI * std::floor(relativePhase / (2 * M_PI));
    return relativePhase;
}

BlochSphereCoordinates Qubit::getBlochSphereCoordinates() const {
    //create the struct
    BlochSphereCoordinates bsc{};

    //get the angles
    double theta = findPolarAngle();
    double phi = findRelativePhase();

    //compute the transformation
    bsc.sphericalX() = std::sin(theta) * std::cos(phi);
    bsc.sphericalY() = std::sin(theta) * std::sin(phi);
    bsc.sphericalZ() = std::cos(theta);

    //return the value
    return bsc;
}

/*

    FUNCTION: computeBlochStateDensityMatrix()/computeExternalProductStateDensityMatrix():


*/