#pragma once
#include <cstdint>
#include <iomanip>
#define PERF_TIME_TO_WAIT 100

#if _WIN32

#include <intrin.h>
#include <windows.h>

#else

#include <sys/time.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)

#include <x86intrin.h>

#endif

#endif
struct Perf 
{
    uint64_t total_time_;
    uint64_t cpu_freq_;
    uint64_t start_up_;
    uint64_t read_;
    uint64_t misc_setup_;
    uint64_t parse_;
    uint64_t sum_;
    uint64_t misc_output_;
};

uint64_t GetOSTimerFreq();
uint64_t ReadOSTimer();
uint64_t GetOSTimerFreq();
uint64_t ReadCPUTimer();
uint64_t GetCPUFreqEstimate();
void PrintPerf(Perf& perf);
double percent(uint64_t part, uint64_t whole);