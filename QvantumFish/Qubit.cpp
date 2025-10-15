#include "Qubit.h"
#include <cmath>
#include <stdexcept>
#include <iostream>




/*

    FUNCTION:   checkCorrectInitialization():
	                check for valid qubit creation
	                to be a valid qubit ||alpha||^2 + ||beta||^2 = 1 
	                since we are working with floating point precision we check with an error of 10^-10 since the double have a precison of around 10^-12
*/
bool Qubit::checkCorrectInitialization() const {
    double norm = std::norm(states[0]) + std::norm(states[1]);
    return std::abs(norm - 1.0) < 1e-10;
}



/*

    CONSTRUCTOR

*/
Qubit::Qubit(const std::complex<double> input_states[2]) {
    states[0] = input_states[0];
    states[1] = input_states[1];
    if (!checkCorrectInitialization()) {
        throw std::invalid_argument("Qubit states do not satisfy normalization condition");
    }
}



/*

    COPY CONSTRUCTOR

*/
Qubit::Qubit(const Qubit& other) {
    states[0] = other.states[0];
    states[1] = other.states[1];
}



/*

    ASSIGNMENT OPERATOR

*/
Qubit& Qubit::operator=(const Qubit& other) {
    if (this != &other) {
        states[0] = other.states[0];
        states[1] = other.states[1];
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
    return states[0];
}

std::complex<double> Qubit::getBeta() const {
    return states[1];
}



/*

    FUNCTION: look():
                Basic output function to show the qubit's states
                

*/
void Qubit::look() const {
    std::cout << "the qubit's states are the following: "
        << states[0].real() << "+" << states[0].imag() << "i |0> + "
        << states[1].real() << "+" << states[1].imag() << "i |1>" << std::endl;
}



/*

    FUNCTION: probabilityZero()
                Calculate probability of measuring |0>

*/
double Qubit::probabilityZero() const {
    return std::norm(states[0]);
}



/*

    FUNCTION: probabilityOne()
                Calculate probability of measuring |1>

*/
double Qubit::probabilityOne() const {
    return std::norm(states[1]);
}



/*

    STATIC FACTORY 

*/
//a qubit initialized to |0>
Qubit Qubit::ketZero() {
    std::complex<double> ket_zero_state[] = { 0.0, 0.0 };
    return Qubit(ket_zero_state);
}



//a qubit initialized to |1>
Qubit Qubit::ketOne() {
    std::complex<double> ket_one_state[] = {0.0, 1.0};
    return Qubit(ket_one_state);
}



//a qubit initialized to |+>
Qubit Qubit::ketPlus() {
    double factor = 1.0 / std::sqrt(2.0);
    std::complex<double> plus_state[2] = { factor, factor };
    return Qubit(plus_state);
}



//a qubit initialized to |->
Qubit Qubit::ketMinus() {
    double factor = 1.0 / std::sqrt(2.0);
    std::complex<double> minus_state[2] = { factor, -factor };
    return Qubit(minus_state);
}

//a qubit initialized to |+i>
Qubit Qubit::ketPlusI() {
    double factor = 1.0 / std::sqrt(2.0);
    std::complex<double> plus_i_state[2] = { factor, std::complex<double>(0.0, factor) };
    return Qubit(plus_i_state);
}



//a qubit initialized to |-i>
Qubit Qubit::ketMinusI() {
    double factor = 1.0 / std::sqrt(2.0);
    std::complex<double> minus_i_state[2] = { factor, std::complex<double>(0.0, -factor) };
    return Qubit(minus_i_state);
}