/*
Author: Abby McCollam
Class: ECE4122 Section A
Last Date Modified: 10/10/23
Description:

Header file for point charge base class.

*/

#ifndef LAB1_ECE_POINTCHARGE_H
#define LAB1_ECE_POINTCHARGE_H

class ECE_PointCharge //class to assign user given values for the point charge
{
public: //functions
    ECE_PointCharge (double x, double y, double z, double q); //constructor initializing below parameters
    [[nodiscard]] double getX() const; //get functions to check position of point charge
    [[nodiscard]] double getY() const;
    [[nodiscard]] double getZ() const;

protected:
    double x; //x-coordinate
    double y; //y-coordinate
    double z; //z-coordinate
    double q; //charge of the point
};

#endif
