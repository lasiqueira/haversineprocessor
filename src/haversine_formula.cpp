#include "haversine_formula.hpp"

double Square(double a)
{
    double result = (a*a);
    return result;
}

double RadiansFromDegrees(double degrees)
{
    double result = 0.01745329251994329577 * degrees;
    return result;
}

// NOTE(casey): EarthRadius is generally expected to be 6372.8
double ReferenceHaversine(double X0, double Y0, double X1, double Y1, double earth_radius)
{   
    double lat1 = Y0;
    double lat2 = Y1;
    double lon1 = X0;
    double lon2 = X1;
    
    double d_lat = RadiansFromDegrees(lat2 - lat1);
    double d_lon = RadiansFromDegrees(lon2 - lon1);
    lat1 = RadiansFromDegrees(lat1);
    lat2 = RadiansFromDegrees(lat2);
    
    double a = Square(sin(d_lat/2.0)) + cos(lat1)*cos(lat2)*Square(sin(d_lon/2));
    double c = 2.0*asin(sqrt(a));
    
    double result = earth_radius * c;
    return result;
}