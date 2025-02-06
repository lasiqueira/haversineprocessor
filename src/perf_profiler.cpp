#include <iostream>
#include "perf_profiler.hpp"
#include "platform_metrics.hpp"

#if PROFILER

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

ProfileBlock::ProfileBlock(char const* label, uint32_t anchor_index, uint64_t byte_count)
{
	parent_index_ = g_profiler_parent;
	anchor_index_ = anchor_index;
	label_ = label;
	
	ProfileAnchor* anchor = g_profile_anchors + anchor_index_;
	old_tsc_elapsed_inclusive_ = anchor->tsc_elapsed_inclusive_;
	anchor->processed_byte_count_ += byte_count;
	g_profiler_parent = anchor_index_;
	start_tsc_ = READ_BLOCK_TIMER();
}

ProfileBlock::~ProfileBlock()
{
	uint64_t elapsed = READ_BLOCK_TIMER() - start_tsc_;
	g_profiler_parent = parent_index_;

	ProfileAnchor* parent = g_profile_anchors + parent_index_;
	ProfileAnchor* anchor = g_profile_anchors + anchor_index_;
	
	parent->tsc_elapsed_exclusive_ -= elapsed;
	anchor->tsc_elapsed_exclusive_ += elapsed;
	anchor->tsc_elapsed_inclusive_ = old_tsc_elapsed_inclusive_ + elapsed;
	++anchor->hit_count_;

	/* NOTE(casey): This write happens every time solely because there is no
	   straightforward way in C++ to have the same ease-of-use. In a better programming
	   language, it would be simple to have the anchor points gathered and labeled at compile
	   time, and this repetative write would be eliminated. */
	anchor->label_ = label_;
}

void PrintTimeElapsed(uint64_t total_tsc_elapsed, uint64_t timer_freq,  ProfileAnchor* anchor)
{
	double percent = 100.0 * ((double)anchor->tsc_elapsed_exclusive_ / (double)total_tsc_elapsed);
	printf("  %s[%llu]: %llu (%.2f%%", anchor->label_, anchor->hit_count_, anchor->tsc_elapsed_exclusive_, percent);

	if (anchor->tsc_elapsed_inclusive_ != anchor->tsc_elapsed_exclusive_)
	{
		double percent_with_children = 100.0 * ((double)anchor->tsc_elapsed_inclusive_ / (double)total_tsc_elapsed);
		printf(", %.2f%% w/children", percent_with_children);
	}
	printf(")");
	if (anchor->processed_byte_count_)
	{
		double megabyte = 1024.0 * 1024.0;
		double gigabyte = 1024.0 * megabyte;

		double seconds = (double)anchor->tsc_elapsed_inclusive_ / (double)timer_freq;
		double bytes_per_second = (double)anchor->processed_byte_count_ / seconds;
		double megabytes = (double)anchor->processed_byte_count_ / (double)megabyte;
		double gigabytes_per_second = bytes_per_second / gigabyte;

		printf("  %.3fmb at %.2fgb/s", megabytes, gigabytes_per_second);
	}
	printf("\n");
}
void PrintAnchorData(uint64_t total_cpu_elapsed, uint64_t timer_freq)
{
	for (uint32_t anchor_index = 0; anchor_index < ArrayCount(g_profile_anchors); ++anchor_index)
	{
		ProfileAnchor* anchor = g_profile_anchors + anchor_index;
		if (anchor->tsc_elapsed_inclusive_)
		{
			PrintTimeElapsed(total_cpu_elapsed, timer_freq, anchor);
		}
	}
}
#endif
uint64_t EstimateBlockTimerFreq()
{
	(void)&EstimateBlockTimerFreq; // NOTE(casey): This has to be voided here to prevent compilers from warning us that it is not used

	uint64_t milliseconds_to_wait = 100;
	uint64_t os_freq = GetOSTimerFreq();

	uint64_t block_start = READ_BLOCK_TIMER();
	uint64_t os_start = ReadOSTimer();
	uint64_t os_end = 0;
	uint64_t os_elapsed = 0;
	uint64_t os_wait_time = os_freq * milliseconds_to_wait / 1000;
	while (os_elapsed < os_wait_time)
	{
		os_end = ReadOSTimer();
		os_elapsed = os_end - os_start;
	}

	uint64_t block_end = READ_BLOCK_TIMER();
	uint64_t block_elapsed = block_end - block_start;

	uint64_t block_freq = 0;
	if (os_elapsed)
	{
		block_freq = os_freq * block_elapsed / os_elapsed;
	}

	return block_freq;
}
void BeginProfile()
{
	g_profiler.start_tsc_ = READ_BLOCK_TIMER();
}

void EndAndPrintProfile()
{
	g_profiler.end_tsc_ = READ_BLOCK_TIMER();
	uint64_t timer_freq = EstimateBlockTimerFreq();

	uint64_t total_cpu_elapsed = g_profiler.end_tsc_ - g_profiler.start_tsc_;

	if (timer_freq)
	{
		printf("\nTotal time: %0.4fms (timer freq %llu)\n", 1000.0 * (double)total_cpu_elapsed / (double)timer_freq, timer_freq);
	}

	PrintAnchorData(total_cpu_elapsed, timer_freq);
}