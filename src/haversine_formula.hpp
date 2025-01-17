#pragma once

#include <cmath>
#include <cstdint>
#define EARTH_RAD 6372.8

double Square(double a);
double RadiansFromDegrees(double degrees);
// NOTE(casey): EarthRadius is generally expected to be 6372.8
double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double earth_radius);
