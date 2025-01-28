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

ProfileBlock::ProfileBlock(char const* label, uint32_t anchor_index)
{
	parent_index_ = g_profiler_parent;
	anchor_index_ = anchor_index;
	label_ = label;
	
	ProfileAnchor* anchor = g_profile_anchors + anchor_index_;
	old_tsc_elapsed_inclusive_ = anchor->tsc_elapsed_inclusive_;
	
	g_profiler_parent = anchor_index_;
	start_tsc_ = ReadCPUTimer();
}

ProfileBlock::~ProfileBlock()
{
	uint64_t elapsed = ReadCPUTimer() - start_tsc_;
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

void PrintTimeElapsed(uint64_t total_tsc_elapsed, ProfileAnchor* anchor)
{
	double percent = 100.0 * ((double)anchor->tsc_elapsed_exclusive_ / (double)total_tsc_elapsed);
	printf("  %s[%llu]: %llu (%.2f%%", anchor->label_, anchor->hit_count_, anchor->tsc_elapsed_exclusive_, percent);

	if (anchor->tsc_elapsed_inclusive_ != anchor->tsc_elapsed_exclusive_)
	{
		double percent_with_children = 100.0 * ((double)anchor->tsc_elapsed_inclusive_ / (double)total_tsc_elapsed);
		printf(", %.2f%% w/children", percent_with_children);
	}
	printf(")\n");
}
void PrintAnchorData(uint64_t total_cpu_elapsed)
{
	for (uint32_t anchor_index = 0; anchor_index < ArrayCount(g_profile_anchors); ++anchor_index)
	{
		ProfileAnchor* anchor = g_profile_anchors + anchor_index;
		if (anchor->tsc_elapsed_inclusive_)
		{
			PrintTimeElapsed(total_cpu_elapsed, anchor);
		}
	}
}
#endif

void BeginProfile()
{
	g_profiler.start_tsc_ = ReadCPUTimer();
}

void EndAndPrintProfile()
{
	g_profiler.end_tsc_ = ReadCPUTimer();
	uint64_t cpu_freq = GetCPUFreqEstimate();

	uint64_t total_cpu_elapsed = g_profiler.end_tsc_ - g_profiler.start_tsc_;

	if (cpu_freq)
	{
		printf("\nTotal time: %0.4fms (CPU freq %llu)\n", 1000.0 * (double)total_cpu_elapsed / (double)cpu_freq, cpu_freq);
	}

	PrintAnchorData(total_cpu_elapsed);
}