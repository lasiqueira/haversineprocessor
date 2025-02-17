#pragma once
#include <cstdint>

enum TestMode : uint32_t
{
    UNINITIALIZED,
    TESTING,
    COMPLETED,
    ERROR,
};

struct RepetitionTestResults
{
    uint64_t test_count_;
    uint64_t total_time_;
    uint64_t max_time_;
    uint64_t min_time_;
};

struct RepetitionTester
{
    uint64_t target_processed_byte_count_;
    uint64_t cpu_timer_freq_;
    uint64_t try_for_time_;
    uint64_t tests_started_at_;

    TestMode mode_;
    bool print_new_minimums_;
    uint32_t open_block_count_;
    uint32_t close_block_count_;
    uint64_t time_accumulated_on_this_test_;
    uint64_t bytes_accumulated_on_this_test_;

    RepetitionTestResults results_;
};

double SecondsFromCpuTime(double cpu_time, uint64_t cpu_timer_freq);
void PrintTime(char const *label, double cpu_time,  uint64_t cpu_timer_freq, uint64_t byte_count);
void PrintTime(char const *label, uint64_t cpu_time, uint64_t cpu_timer_freq, uint64_t byte_count);
void PrintResults(const RepetitionTestResults &results, uint64_t cpu_timer_freq, uint64_t byte_count);
void Error(RepetitionTester &tester, char const *error_message);
void NewTestWave(RepetitionTester &tester, uint64_t target_processed_byte_count, uint64_t cpu_timer_freq, uint32_t seconds_to_try = 10);
void BeginTime(RepetitionTester &tester);
void EndTime(RepetitionTester &tester);
void CountBytes(RepetitionTester &tester, uint64_t byte_count);
bool IsTesting(RepetitionTester &tester);