#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "haversine_formula.hpp"
#include "perf_profiler.hpp"
#include "custom_memory_allocator.hpp"

#define CustomString std::basic_string<char, std::char_traits<char>, CustomMemoryAllocator<char>>
#define CustomVector(type) std::vector<type, CustomMemoryAllocator<type>>
#define CustomStringStream std::basic_istringstream<char, std::char_traits<char>, CustomMemoryAllocator<char>>


struct Point
{
    double x0;
    double y0;
    double x1;
    double y1;
};
uint64_t ReadPointsJson(std::string filename, CustomString& json);
void ProcessJson(CustomString& json_content, CustomVector(Point)& points);
double SumHaversine(const CustomVector(double)& haversine_vals);

void ProcessJson(CustomString& json_content, CustomVector(Point)& points)
{
    TimeBandwidth(__func__, points.size() * sizeof(Point));
    size_t pos = json_content.find("\"points\":");
    if (pos == CustomString::npos)
    {
        std::cerr << "  Invalid JSON format: missing \"points\" key" << std::endl;
        return;
    }

    pos = json_content.find('[', pos);
    if (pos == CustomString::npos)
    {
        std::cerr << "  Invalid JSON format: missing opening bracket for points array" << std::endl;
        return;
    }

    size_t end_pos = json_content.find(']', pos);
    if (end_pos == CustomString::npos)
    {
        std::cerr << "  Invalid JSON format: missing closing bracket for points array" << std::endl;
        return;
    }

    CustomString points_array(json_content.begin() + pos + 1, json_content.begin() + end_pos);
    CustomStringStream ss(points_array);
    std::string point_str;
    
    while (std::getline(ss, point_str, '{'))
    {
        if (point_str.find('}') == CustomString::npos)
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
double SumHaversine(const CustomVector(double)& haversine_vals)
{
    TimeBandwidth(__func__, haversine_vals.size() * sizeof(double));
	double sum = 0;
	for (auto& val : haversine_vals)
	{
        sum += val;
	}
	return sum;
}

uint64_t ReadPointsJson(std::string filename, CustomString& json)
{
    TimeFunction;

    std::ifstream file(filename.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "  Could not open file: " << filename << std::endl;
        return 0;
    }

    uint64_t file_size = static_cast<uint64_t>(file.tellg());
    file.seekg(0, std::ios::beg);

    constexpr std::size_t CHUNK_SIZE = 8 * 1024 * 1024; // 8MB

    CustomVector(char) buffer(CHUNK_SIZE);
    uint64_t total_read = 0;

    json.clear();

    {
        TimeBandwidth("Read file", file_size); // Profile the whole file read
        while (file)
        {
            file.read(buffer.data(), CHUNK_SIZE);
            std::streamsize bytes_read = file.gcount();
            if (bytes_read > 0)
            {
                json.append(buffer.data(), static_cast<size_t>(bytes_read));
                total_read += static_cast<uint64_t>(bytes_read);
            }
        }
    }

    file.close();
    return total_read;
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
    CustomVector(Point) points;
        
    CustomString json;
    uint64_t file_size = ReadPointsJson(filename, json);
	if (file_size == 0)
	{
		return 1;
	}
	
    ProcessJson(json, points);
    CustomVector(double) haversine_vals;
    haversine_vals.reserve(points.size());
    {
        TimeBandwidth("Haversine", points.size() * sizeof(Point));
        for (auto& point : points)
        {

            double haversine_val = ReferenceHaversine(point.x0, point.y0, point.x1, point.y1, EARTH_RAD);
            haversine_vals.push_back(haversine_val);
        }
    }
   
    long double sum = SumHaversine(haversine_vals);
    
    std::cout << "File size: " << file_size << " bytes" << std::endl;
    std::cout << "Points: " << points.size() << std::endl; 
    std::cout << std::fixed << std::setprecision(16) << "Haversine sum: " << sum << std::endl;

    EndAndPrintProfile();
    
    return 0;

}