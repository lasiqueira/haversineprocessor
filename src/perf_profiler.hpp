#pragma once
#include <cstdint>
#include <iomanip>
#include <iostream>
#define PERF_TIME_TO_WAIT 100
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

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
double Percent(uint64_t part, uint64_t whole);

struct ProfileAnchor
{
    uint64_t tsc_elapsed_;
    uint64_t tsc_elapsed_children_;
    uint64_t hit_count_;
    char const* label_;
};

struct Profiler
{
    ProfileAnchor anchors_[4096];

    uint64_t start_tsc_;
    uint64_t end_tsc_;
};
static Profiler g_profiler;
static uint32_t g_profiler_parent;

struct ProfileBlock
{
    char const* label_;
    uint64_t start_tsc_;
    uint32_t anchor_index_;
	uint32_t parent_index_;

    ProfileBlock(char const* label, uint32_t anchor_index);

    ~ProfileBlock();
};

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBlock(Name) ProfileBlock NameConcat(Block, __LINE__)(Name, __COUNTER__ + 1);
#define TimeFunction TimeBlock(__func__)

void PrintTimeElapsed(uint64_t total_tsc_elapsed, ProfileAnchor* anchor);
void BeginProfile();
void EndAndPrintProfile();
