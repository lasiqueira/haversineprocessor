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
Perf g_perf;
uint64_t ReadPointsJson(std::string filename, std::vector<Point>& points);

uint64_t ReadPointsJson(std::string filename, std::vector<Point>& points)
{
    g_perf.read_ = ReadCPUTimer();
    std::ifstream file(filename, std::ios::binary | std::ios::ate);
    if (!file.is_open())
    {
        std::cerr << "Could not open file: " << filename << std::endl;
        return 0;
    }

    uint64_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::string line;
    std::string json_content;
    while (std::getline(file, line))
    {
        json_content += line;
    }
    g_perf.read_ = ReadCPUTimer() - g_perf.read_;

    g_perf.parse_ = ReadCPUTimer();
    size_t pos = json_content.find("\"points\":");
    if (pos == std::string::npos)
    {
        std::cerr << "Invalid JSON format: missing \"points\" key" << std::endl;
        return 0;
    }

    pos = json_content.find('[', pos);
    if (pos == std::string::npos)
    {
        std::cerr << "Invalid JSON format: missing opening bracket for points array" << std::endl;
        return 0;
    }

    size_t end_pos = json_content.find(']', pos);
    if (end_pos == std::string::npos)
    {
        std::cerr << "Invalid JSON format: missing closing bracket for points array" << std::endl;
        return 0;
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
    
    file.close();
    g_perf.parse_ = ReadCPUTimer() - g_perf.parse_;
    return file_size;
}
int main(int argc, char* argv[])
{
    g_perf.cpu_freq_ = GetCPUFreqEstimate();
    g_perf.start_up_ = ReadCPUTimer();
    if(argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename.json>" << std::endl;
        return 1;
    }
    std::string filename = argv[1];
    std::vector<Point> points;
    
    g_perf.start_up_ = ReadCPUTimer() - g_perf.start_up_;
    
    uint64_t file_size = ReadPointsJson(filename, points);
    
    std::vector<double> haversine_vals;
    haversine_vals.reserve(points.size());
    
    g_perf.misc_setup_ = ReadCPUTimer();
    for(auto& point : points)
    {    
        double haversine_val = ReferenceHaversine(point.x0, point.y0, point.x1, point.y1, EARTH_RAD);
        haversine_vals.push_back(haversine_val);
    }
    g_perf.misc_setup_ = ReadCPUTimer() - g_perf.misc_setup_;
    long double sum = 0;
    g_perf.sum_ = ReadCPUTimer();
    for(auto& val : haversine_vals)
    {
        sum += val;
    }
    g_perf.sum_ = ReadCPUTimer() - g_perf.sum_;
    
    g_perf.misc_output_ = ReadCPUTimer();
    std::cout << "File size: " << file_size << " bytes" << std::endl;
    std::cout << "Points: " << points.size() << std::endl; 
    std::cout << std::fixed << std::setprecision(16) << "Haversine sum: " << sum << std::endl;


    g_perf.misc_output_ = ReadCPUTimer() - g_perf.misc_output_;

    g_perf.total_time_ = g_perf.read_ + g_perf.parse_ + g_perf.sum_ + g_perf.misc_output_ + g_perf.misc_setup_ + g_perf.start_up_;
    
    PrintPerf(g_perf);
    
    return 0;

}