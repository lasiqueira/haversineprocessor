#pragma once
#include <cstdint>
#include <iomanip>
#include <iostream>
#define ArrayCount(array) (sizeof(array) / sizeof((array)[0]))

#ifndef PROFILER
#define PROFILER 1
#endif

#ifndef READ_BLOCK_TIMER
#define READ_BLOCK_TIMER ReadCPUTimer
#endif

#if PROFILER

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
void PrintPerf(Perf& perf);
double Percent(uint64_t part, uint64_t whole);

struct ProfileAnchor
{
    uint64_t tsc_elapsed_exclusive_;
    uint64_t tsc_elapsed_inclusive_;
    uint64_t hit_count_;
    uint64_t processed_byte_count_;
    char const* label_;
};

extern ProfileAnchor g_profile_anchors[4096];
extern uint32_t g_profiler_parent;
extern uint32_t g_profiler_anchor_count;

struct ProfileBlock
{
    char const* label_;
    uint64_t old_tsc_elapsed_inclusive_;
    uint64_t start_tsc_;
    uint32_t anchor_index_;
	uint32_t parent_index_;

    ProfileBlock(char const* label, uint32_t anchor_index, uint64_t byte_count);

    ~ProfileBlock();
};

void PrintAnchorData(uint64_t total_cpu_elapsed, uint64_t timer_freq);
void PrintTimeElapsed(uint64_t total_tsc_elapsed, uint64_t timer_freq, ProfileAnchor* anchor);

#define NameConcat2(A, B) A##B
#define NameConcat(A, B) NameConcat2(A, B)
#define TimeBandwidth(Name, ByteCount) ProfileBlock NameConcat(Block, __LINE__)(Name, ++g_profiler_anchor_count, ByteCount);
#define ProfilerEndOfCompilationUnit static_assert(g_profiler_anchor_countc< ArrayCount(g_profiler_anchors), "Number of profile points exceeds size of profiler::anchors_ array")
#else

#define TimeBandwidth(...)
#define PrintAnchorData(...)
#define ProfilerEndOfCompilationUnit

#endif

#define TimeBlock(Name) TimeBandwidth(Name, 0)
#define TimeFunction TimeBlock(__func__)
struct Profiler
{
    uint64_t start_tsc_;
    uint64_t end_tsc_;
};
extern Profiler g_profiler;
uint64_t EstimateBlockTimerFreq();
void BeginProfile();
void EndAndPrintProfile();
