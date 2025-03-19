#pragma once
#include <cstdint>
#if _WIN32

#include <intrin.h>
#include <windows.h>
#include <psapi.h>

struct OsPlatform
{
    bool initialized_;
    uint64_t large_page_size_;
    HANDLE process_handle_;
    uint64_t cpu_timer_freq_;
};

#else

#include <sys/time.h>
#include <sys/resource.h>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)

#include <x86intrin.h>

#endif
struct OsPlatform
{
    bool initialized_;
    uint64_t cpu_timer_freq_;
};
#endif
#define PERF_TIME_TO_WAIT 100
extern OsPlatform g_platform;
uint64_t GetOSTimerFreq();
uint64_t ReadOSTimer();
uint64_t ReadCPUTimer();
uint64_t GetCPUFreqEstimate();
void InitializeOSMetrics();
uint64_t ReadOSPageFaultCount();