#include <cmath>
#include "haversine_formula.hpp"

double Square(double A)
{
    double Result = (A*A);
    return Result;
}

double RadiansFromDegrees(double Degrees)
{
    double Result = 0.01745329251994329577 * Degrees;
    return Result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double EarthRadius)
{   
    double lat1 = Y0;
    double lat2 = Y1;
    double lon1 = X0;
    double lon2 = X1;
    
    double dLat = RadiansFromDegrees(lat2 - lat1);
    double dLon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    double a = Square(sin(dLat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(dLon/2));
    double c = 2.0*asin(sqrt(a));
    
    double Result = EarthRadius * c;
    
    return Result;
}