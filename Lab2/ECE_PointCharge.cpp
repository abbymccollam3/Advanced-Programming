/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 10/10/23
Description:

Point charge source file that sets user inputted location and charge values to charge object.

*/

//directives
#include "ECE_ElectricField.h"

using namespace std;

ECE_PointCharge::ECE_PointCharge(double x, double y, double z, double q): x(x), y(y), z(z), q(q) {} //initializing the constructor

double ECE_PointCharge::getX() const {return x;} //returning coordinates of point charge
double ECE_PointCharge::getY() const {return y;}
double ECE_PointCharge::getZ() const {return z;}

