#include <iostream>
#include "repetition_tester.hpp"
#include "platform_metrics.hpp"

double SecondsFromCpuTime(double cpu_time, uint64_t cpu_timer_freq)
{
    double result = 0.0;
    if(cpu_timer_freq)
    {
        result = (cpu_time / (double)cpu_timer_freq);
    }
    
    return result;
}

void PrintTime(char const *label, double cpu_time,  uint64_t cpu_timer_freq, uint64_t byte_count)
{
    printf("%s: %.0f", label, cpu_time);
    if(cpu_timer_freq)
    {
        double seconds = SecondsFromCpuTime(cpu_time, cpu_timer_freq);
        printf(" (%fms)", 1000.0f * seconds);
    
        if(byte_count)
        {
            double gigabyte = (1024.0f * 1024.0f * 1024.0f);
            double best_bandwidth = byte_count / (gigabyte * seconds);
            printf(" %fgb/s", best_bandwidth);
        }
    }
}

void PrintTime(char const *label, uint64_t cpu_time,  uint64_t cpu_timer_freq, uint64_t byte_count)
{
    PrintTime(label, (double)cpu_time, cpu_timer_freq, byte_count);
}

void PrintResults(const RepetitionTestResults &results, uint64_t cpu_timer_freq, uint64_t byte_count)
{
    PrintTime("Min", (double)results.min_time_, cpu_timer_freq, byte_count);
    printf("\n");
    
    PrintTime("Max", (double)results.max_time_, cpu_timer_freq, byte_count);
    printf("\n");
    
    if(results.test_count_)
    {
        PrintTime("Avg", (double)results.total_time_ / (double)results.test_count_, cpu_timer_freq, byte_count);
        printf("\n");
    }
}

void Error(RepetitionTester &tester, char const *error_message)
{
    tester.mode_ = TestMode::ERROR;
    fprintf(stderr, "ERROR: %s\n", error_message);
}

void NewTestWave(RepetitionTester &tester, uint64_t target_processed_byte_count, uint64_t cpu_timer_freq, uint32_t seconds_to_try)
{
    if(tester.mode_ == TestMode::UNINITIALIZED)
    {
        tester.mode_ = TestMode::TESTING;
        tester.target_processed_byte_count_ = target_processed_byte_count;
        tester.cpu_timer_freq_ = cpu_timer_freq;
        tester.print_new_minimums_ = true;
        tester.results_.min_time_ = (double)-1;
    }
    else if(tester.mode_ == TestMode::COMPLETED)
    {
        tester.mode_ = TestMode::TESTING;
        
        if(tester.target_processed_byte_count_ != target_processed_byte_count)
        {
            Error(tester, "target_processed_byte_count_ changed");
        }
        
        if(tester.cpu_timer_freq_ != cpu_timer_freq)
        {
            Error(tester, "CPU frequency changed");
        }
    }

    tester.try_for_time_ = seconds_to_try*cpu_timer_freq;
    tester.tests_started_at_ = ReadCPUTimer();
}

void BeginTime(RepetitionTester &tester)
{
    ++tester.open_block_count_;
    tester.time_accumulated_on_this_test_ -= ReadCPUTimer();
}

void EndTime(RepetitionTester &tester)
{
    ++tester.close_block_count_;
    tester.time_accumulated_on_this_test_ += ReadCPUTimer();
}

void CountBytes(RepetitionTester &tester, uint64_t byte_count)
{
    tester.bytes_accumulated_on_this_test_ += byte_count;
}

bool IsTesting(RepetitionTester &tester)
{
    if(tester.mode_ == TestMode::TESTING)
    {
        uint64_t current_time = ReadCPUTimer();
        
        if(tester.open_block_count_) // NOTE(casey): We don't count tests that had no timing blocks - we assume they took some other path
        {
            if(tester.open_block_count_ != tester.close_block_count_)
            {
                Error(tester, "Unbalanced BeginTime/EndTime");
            }
            
            if(tester.bytes_accumulated_on_this_test_ != tester.target_processed_byte_count_)
            {
                Error(tester, "Processed byte count mismatch");
            }
    
            if(tester.mode_ == TestMode::TESTING)
            {
                RepetitionTestResults &results = tester.results_;
                uint64_t elapsed_time = tester.time_accumulated_on_this_test_;
                results.test_count_ += 1;
                results.total_time_ += elapsed_time;
                if(results.max_time_ < elapsed_time)
                {
                    results.max_time_ = elapsed_time;
                }
                
                if(results.min_time_ > elapsed_time)
                {
                    results.min_time_ = elapsed_time;
                    
                    // NOTE(casey): Whenever we get a new minimum time, we reset the clock to the full trial time
                    tester.tests_started_at_ = current_time;
                    
                    if(tester.print_new_minimums_)
                    {
                        PrintTime("Min", results.min_time_, tester.cpu_timer_freq_, tester.bytes_accumulated_on_this_test_);
                        printf("               \r");
                    }
                }
                
                tester.open_block_count_ = 0;
                tester.close_block_count_ = 0;
                tester.time_accumulated_on_this_test_ = 0;
                tester.bytes_accumulated_on_this_test_ = 0;
            }
        }
        
        if((current_time - tester.tests_started_at_) > tester.try_for_time_)
        {
            tester.mode_ = TestMode::COMPLETED;
            
            printf("                                                          \r");
            PrintResults(tester.results_, tester.cpu_timer_freq_, tester.target_processed_byte_count_);
        }
    }
    
    bool result = (tester.mode_ == TestMode::TESTING);
    return result;
}