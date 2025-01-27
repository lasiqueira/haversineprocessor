#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "haversine_formula.hpp"
#include "perf_profiler.hpp"

struct Point
{
    double x0;
    double y0;
    double x1;
    double y1;
};
uint64_t ReadPointsJson(std::string filename, std::vector<Point>& points, std::string& json);
void ProcessJson(std::string& json_content, std::vector<Point>& points);
double SumHaversine(const std::vector<double>& haversine_vals);


void ProcessJson(std::string& json_content, std::vector<Point>& points)
{
    TimeFunction;
    size_t pos = json_content.find("\"points\":");
    if (pos == std::string::npos)
    {
        std::cerr << "  Invalid JSON format: missing \"points\" key" << std::endl;
        return;
    }

    pos = json_content.find('[', pos);
    if (pos == std::string::npos)
    {
        std::cerr << "  Invalid JSON format: missing opening bracket for points array" << std::endl;
        return;
    }

    size_t end_pos = json_content.find(']', pos);
    if (end_pos == std::string::npos)
    {
        std::cerr << "  Invalid JSON format: missing closing bracket for points array" << std::endl;
        return;
    }

    std::string points_array = json_content.substr(pos + 1, end_pos - pos - 1);
    std::istringstream ss(points_array);
    std::string point_str;

    while (std::getline(ss, point_str, '{'))
    {
        if (point_str.find('}') == std::string::npos)
            continue;

        double x0 = 0;
        double y0 = 0;
        double x1 = 0;
        double y1 = 0;
        std::istringstream point_ss(point_str);
        std::string token;
        while (std::getline(point_ss, token, ','))
        {
            size_t colon_pos = token.find(':');
            if (colon_pos == std::string::npos)
                continue;

            std::string key = token.substr(0, colon_pos);
            std::string value = token.substr(colon_pos + 1);

            key.erase(std::remove_if(key.begin(), key.end(), isspace), key.end());
            value.erase(std::remove_if(value.begin(), value.end(), isspace), value.end());

            if (key == "\"x0\"")
                x0 = std::stod(value);
            else if (key == "\"y0\"")
                y0 = std::stod(value);
            else if (key == "\"x1\"")
                x1 = std::stod(value);
            else if (key == "\"y1\"")
                y1 = std::stod(value);
        }
        points.emplace_back(x0, y0, x1, y1);
    }
 }
double SumHaversine(const std::vector<double>& haversine_vals)
{
    TimeFunction;
	double sum = 0;
	for (auto& val : haversine_vals)
	{
        sum += val;
	}
	return sum;
}
uint64_t ReadPointsJson(std::string filename, std::vector<Point>& points, std::string& json)
{
    TimeFunction;
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "  Could not open file: " << filename << std::endl;
        return 0;
    }

    uint64_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string line;
    while (std::getline(file, line))
    {
        json += line;
    }
	file.close();
    return file_size;
}
int main(int argc, char* argv[])
{
    BeginProfile();
    if(argc < 2)
    {
        std::cerr << "      Usage: " << argv[0] << " <filename.json>" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    std::vector<Point> points;
        
    std::string json;
    uint64_t file_size = ReadPointsJson(filename, points, json);
	if (file_size == 0)
	{
		return 1;
	}
	
    ProcessJson(json, points);
    std::vector<double> haversine_vals;
    haversine_vals.reserve(points.size());
    
    for(auto& point : points)
    {    
		TimeBlock("Haversine");
        double haversine_val = ReferenceHaversine(point.x0, point.y0, point.x1, point.y1, EARTH_RAD);
        haversine_vals.push_back(haversine_val);
    }
   
    long double sum = SumHaversine(haversine_vals);
    
    std::cout << "File size: " << file_size << " bytes" << std::endl;
    std::cout << "Points: " << points.size() << std::endl; 
    std::cout << std::fixed << std::setprecision(16) << "Haversine sum: " << sum << std::endl;

    EndAndPrintProfile();
    
    return 0;

}