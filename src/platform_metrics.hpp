#pragma once
#include <cstdint>
#if _WIN32

#include <intrin.h>
#include <windows.h>
#include <psapi.h>

struct OsMetrics
{
    bool initialized_;
    HANDLE process_handle_;
};
extern OsMetrics g_metrics;
#else

#include <sys/time.h>
#include <sys/resource.h>
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)

#include <x86intrin.h>

#endif

#endif
#define PERF_TIME_TO_WAIT 100

uint64_t GetOSTimerFreq();
uint64_t ReadOSTimer();
uint64_t ReadCPUTimer();
uint64_t GetCPUFreqEstimate();
void InitializeOSMetrics();
uint64_t ReadOSPageFaultCount();