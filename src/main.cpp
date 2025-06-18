#include <vector>
#include <string>
#include "haversine_formula.hpp"
#include "perf_profiler.hpp"
#include "custom_memory_allocator.hpp"

#define CustomVector(type) std::vector<type, CustomMemoryAllocator<type>>


struct Point
{
    double x0;
    double y0;
    double x1;
    double y1;
};
uint64_t ReadPointsJson(const std::string& filename, const CustomVector(char)& buffer);
void ProcessJson(const char* data, size_t size, CustomVector(Point)& points);
double SumHaversine(const CustomVector(double)& haversine_vals);

void ProcessJson(const char* data, size_t size, CustomVector(Point)& points)
{
    TimeBandwidth(__func__, size);

    const char* end = data + size;

    const char* key = "\"points\"";
    const char* pos = std::search(data, end, key, key + 8);

    if (pos == end) {
        std::cerr << "Missing \"points\" key\n";
        return;
    }

    pos = std::find(pos, end, '[');
    if (pos == end) {
        std::cerr << "Missing '[' after \"points\"\n";
        return;
    }
    ++pos; // skip '['

    while (pos < end) {
        // Skip to next '{'
        while (pos < end && *pos != '{') ++pos;
        if (pos == end) break;
        ++pos; // skip '{'

        double x0 = 0, y0 = 0, x1 = 0, y1 = 0;

        while (pos < end && *pos != '}') {
            // Skip whitespace
            while (pos < end && isspace(*pos)) ++pos;

            if (*pos != '"') break; // expected a key

            ++pos;
            const char* key_start = pos;
            while (pos < end && *pos != '"') ++pos;
            std::string_view key(key_start, pos - key_start);
            ++pos; // skip closing quote

            // Skip colon
            while (pos < end && *pos != ':') ++pos;
            if (pos == end) break;
            ++pos;

            while (pos < end && isspace(*pos)) ++pos;

            // Parse number
            const char* val_start = pos;
            while (pos < end && (*pos == '-' || isdigit(*pos) || *pos == '.')) ++pos;
            std::string_view val(val_start, pos - val_start);

            double num = std::strtod(val.data(), nullptr);

            if (key == "x0") x0 = num;
            else if (key == "y0") y0 = num;
            else if (key == "x1") x1 = num;
            else if (key == "y1") y1 = num;

            // Skip until next key or end of object
            while (pos < end && *pos != ',' && *pos != '}') ++pos;
            if (*pos == ',') ++pos;
        }

        points.emplace_back(x0, y0, x1, y1);

        // Skip past '}'
        while (pos < end && *pos != '}') ++pos;
        if (pos < end) ++pos;

        // Check if there's a closing ']'
        while (pos < end && isspace(*pos)) ++pos;
        if (*pos == ']') break;
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

uint64_t ReadPointsJson(const std::string& filename, CustomVector(char)& buffer)
{
    TimeFunction;
    FILE* file = fopen(filename.c_str(), "rb");
    if (!file) {
        std::cerr << "  Could not open file: " << filename << std::endl;
        return 0;
    }

    fseek(file, 0, SEEK_END);
    uint64_t file_size = static_cast<uint64_t>(ftell(file));
    fseek(file, 0, SEEK_SET);

    buffer.reserve(file_size); // allocate exactly
    {
        TimeBandwidth("Read file", file_size);
        size_t read_size = fread(buffer.data(), 1, file_size, file);
        if (read_size != file_size) {
            std::cerr << "  Read size mismatch\n";
            fclose(file);
            return 0;
        }
    }

    fclose(file);
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
    CustomVector(Point) points;
        
    CustomVector(char) json;
    uint64_t file_size = ReadPointsJson(filename, json);
	if (file_size == 0)
	{
		return 1;
	}
	
    ProcessJson(json.data(), file_size, points);
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