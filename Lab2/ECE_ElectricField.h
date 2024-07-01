/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 10/10/23
Description:

Header file for electric field class.

*/

//directives
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include "ECE_PointCharge.h"

#ifndef LAB1_ECE_ELECTRICFIELD_H
#define LAB1_ECE_ELECTRICFIELD_H

class ECE_ElectricField: public ECE_PointCharge //electric field class to calculate E-field
{
public:
    ECE_ElectricField(double x, double y, double z, double q); //constructor initializing above parameters
    void computeFieldAt(double x, double y, double z); //Calculates the electric field at the point (x, y, z) due to the charge using the above formula. Updates the Ex, Ey, Ez member variables
    void getElectricField(double &Ex, double &Ey, double &Ez) const; //get function to retrieve electric field values

protected:
    //electric field variables
    double Ex;
    double Ey;
    double Ez;
};

#endif
