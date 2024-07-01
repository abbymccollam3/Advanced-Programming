/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 10/10/23
Description:

Electric field source file that calculates electric field of point charge.

*/

//directives
//#include <iostream>
#include <cmath>
#include "ECE_ElectricField.h"

using namespace std;

ECE_ElectricField::ECE_ElectricField(double x, double y, double z, double q): ECE_PointCharge(x, y, z, q), Ex(0.0), Ey(0.0), Ez(0.0) {} //initializing constructor

void ECE_ElectricField::computeFieldAt(double x, double y, double z) //function to compute electric field at point (x, y, z)
{
    //distance from location in space to point charge in all directions
    double dx = x - this->x;
    double dy = y - this->y;
    double dz = z - this->z;

    double r = sqrt((dx * dx) + (dy * dy) + (dz * dz)); //calculating value of r, the radial distance between point charge and x, y, z location
    double k = 8.99e9; // Coulomb's constant

    double vecx = dx/r;
    double vecy = dy/r;
    double vecz = dz/r;

    // calculating electric field in different directions using given equation
    Ex = ((k * q) / (r * r)) * vecx;
    Ey = ((k * q) / (r * r)) * vecy;
    Ez = ((k * q) / (r * r)) * vecz;

}

void ECE_ElectricField::getElectricField(double &Ex, double &Ey, double &Ez) const
{
//accessing member variables of current object using this pointer and assigning it to local variable
    Ex = this->Ex;
    Ey = this->Ey;
    Ez = this->Ez;

}
