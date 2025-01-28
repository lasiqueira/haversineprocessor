#pragma once
#include <cstdint>
#if _WIN32

#include <intrin.h>
#include <windows.h>

#else

#include <sys/time.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)

#include <x86intrin.h>

#endif

#endif
#define PERF_TIME_TO_WAIT 100

uint64_t GetOSTimerFreq();
uint64_t ReadOSTimer();
uint64_t ReadCPUTimer();
uint64_t GetCPUFreqEstimate();