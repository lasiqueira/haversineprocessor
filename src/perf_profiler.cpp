#include "perf_profiler.hpp"
#include <iostream>

#if _WIN32

uint64_t GetOSTimerFreq()
{
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	return freq.QuadPart;
}

uint64_t ReadOSTimer()
{
	LARGE_INTEGER value;
	QueryPerformanceCounter(&value);
	return value.QuadPart;
}

#else

uint64_t GetOSTimerFreq()
{
	return 1000000;
}

uint64_t ReadOSTimer()
{
	struct timeval value;
	gettimeofday(&value, 0);
	
	uint64_t result = GetOSTimerFreq()*(uint64_t)value.tv_sec + (uint64_t)value.tv_usec;
	return result;
}

#endif

uint64_t ReadCPUTimer()
{
	#if defined(__APPLE__) && defined(__arm64__)
	
	uint64_t cntvct;
	asm volatile("mrs %0, CNTVCT_EL0" : "=r"(cntvct));
	return cntvct;
	
	#else
	
	return __rdtsc();
	
	#endif
	
}

uint64_t GetCPUFreqEstimate(void)
{
	uint64_t milliseconds_to_wait = PERF_TIME_TO_WAIT;

	uint64_t os_freq = GetOSTimerFreq();
	//printf("	OS Freq: %llu (reported)\n", os_freq);

	uint64_t cpu_start = ReadCPUTimer();
	uint64_t os_start = ReadOSTimer();
	uint64_t os_end = 0;
	uint64_t os_elapsed = 0;
	uint64_t os_wait_time = os_freq * milliseconds_to_wait / 1000;
	while(os_elapsed < os_wait_time)
	{
		os_end = ReadOSTimer();
		os_elapsed = os_end - os_start;
	}
	
	uint64_t cpu_end = ReadCPUTimer();
	uint64_t cpu_elapsed = cpu_end - cpu_start;
	uint64_t cpu_freq = 0;
	if(os_elapsed)
	{
		cpu_freq = os_freq * cpu_elapsed / os_elapsed;
	}

	return cpu_freq;
}
double Percent(uint64_t part, uint64_t whole)
{
    if (whole == 0) return 0.0;
    return (static_cast<double>(part) / static_cast<double>(whole)) * 100.0;
}


void PrintPerf(Perf& perf)
{
    std::cout << "	Total time: " << perf.total_time_*1000/perf.cpu_freq_ << " ms - Cpu freq(estimated): " << perf.cpu_freq_/1000000 << " MHz" << std::endl;
    std::cout << "	Start time: " << perf.start_up_ << " (" << std::fixed << std::setprecision(2) << Percent(perf.start_up_, perf.total_time_) << "%)" << std::endl;
    std::cout << "	Read time: " << perf.read_ << " (" << std::fixed << std::setprecision(2) << Percent(perf.read_, perf.total_time_) << "%)" << std::endl;
    std::cout << "	Parse time: " << perf.parse_ << " (" << std::fixed << std::setprecision(2) << Percent(perf.parse_, perf.total_time_) << "%)" << std::endl;
    std::cout << "	Misc setup time: " << perf.misc_setup_ << " (" << std::fixed << std::setprecision(2) << Percent(perf.misc_setup_, perf.total_time_) << "%)" << std::endl;
    std::cout << "	Sum time: " << perf.sum_ << " (" << std::fixed << std::setprecision(2) << Percent(perf.sum_, perf.total_time_) << "%)" << std::endl;
    std::cout << "	Misc output time: " << perf.misc_output_ << " (" << std::fixed << std::setprecision(2) << Percent(perf.misc_output_, perf.total_time_) << "%)" << std::endl;
}

ProfileBlock::ProfileBlock(char const* label, uint32_t anchor_index)
{
	anchor_index_ = anchor_index;
	label_ = label;
	start_tsc_ = ReadCPUTimer();
}

ProfileBlock::~ProfileBlock()
{
	uint64_t elapsed = ReadCPUTimer() - start_tsc_;

	ProfileAnchor* anchor = g_profiler.anchors_ + anchor_index_;
	anchor->tsc_elapsed_ += elapsed;
	++anchor->hit_count_;

	/* NOTE(casey): This write happens every time solely because there is no
	   straightforward way in C++ to have the same ease-of-use. In a better programming
	   language, it would be simple to have the anchor points gathered and labeled at compile
	   time, and this repetative write would be eliminated. */
	anchor->label_ = label_;
}

void PrintTimeElapsed(uint64_t total_tsc_elapsed, ProfileAnchor* anchor)
{
	uint64_t elapsed = anchor->tsc_elapsed_;
	double percent = 100.0 * ((double)elapsed / (double)total_tsc_elapsed);
	printf("  %s[%llu]: %llu (%.2f%%)\n", anchor->label_, anchor->hit_count_, elapsed, percent);
}

void BeginProfile()
{
	g_profiler.start_tsc_ = ReadCPUTimer();
}

void EndAndPrintProfile()
{
	g_profiler.end_tsc_ = ReadCPUTimer();
	uint64_t cpu_freq = GetCPUFreqEstimate();

	uint64_t total_cpu_elapsed = g_profiler.end_tsc_ - g_profiler.start_tsc_;

	if (cpu_freq)
	{
		printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (double)total_cpu_elapsed / (double)cpu_freq, cpu_freq);
	}

	for (uint32_t anchor_index = 0; anchor_index < ArrayCount(g_profiler.anchors_); ++anchor_index)
	{
		ProfileAnchor* anchor = g_profiler.anchors_ + anchor_index;
		if (anchor->tsc_elapsed_)
		{
			PrintTimeElapsed(total_cpu_elapsed, anchor);
		}
	}
}