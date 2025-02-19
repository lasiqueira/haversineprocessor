#include "platform_metrics.hpp"

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
OsMetrics g_metrics;

uint64_t ReadOSPageFaultCount()
{
	PROCESS_MEMORY_COUNTERS_EX memory_counters = {};
    memory_counters.cb = sizeof(memory_counters);
    GetProcessMemoryInfo(g_metrics.process_handle_, (PROCESS_MEMORY_COUNTERS *)&memory_counters, sizeof(memory_counters));
    
    uint64_t result = memory_counters.PageFaultCount;
    return result;
}

void InitializeOSMetrics()
{
    if(!g_metrics.initialized_)
    {
        g_metrics.initialized_ = true;
        g_metrics.process_handle = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, GetCurrentProcessId());
    }
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

	uint64_t result = GetOSTimerFreq() * (uint64_t)value.tv_sec + (uint64_t)value.tv_usec;
	return result;
}

uint64_t ReadOSPageFaultCount()
{
    // NOTE(casey): The course materials are not tested on MacOS/Linux.
    // This code was contributed to the public github. It may or may not work
    // for your system.
    
    struct rusage usage = {};
    getrusage(RUSAGE_SELF, &usage);
    
    // ru_minflt  the number of page faults serviced without any I/O activity.
    // ru_majflt  the number of page faults serviced that required I/O activity.
    uint64_t result = usage.ru_minflt + usage.ru_majflt;
    
    return result;
}

void InitializeOSMetrics()
{
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
	while (os_elapsed < os_wait_time)
	{
		os_end = ReadOSTimer();
		os_elapsed = os_end - os_start;
	}

	uint64_t cpu_end = ReadCPUTimer();
	uint64_t cpu_elapsed = cpu_end - cpu_start;
	uint64_t cpu_freq = 0;
	if (os_elapsed)
	{
		cpu_freq = os_freq * cpu_elapsed / os_elapsed;
	}

	return cpu_freq;
}