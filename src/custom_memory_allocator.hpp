#pragma once
#ifdef _WIN32
#include <windows.h>
#elif defined(__linux__) || defined(__APPLE__)
#include <sys/mman.h>
#endif
#include <cstddef>
#include <cstdlib>
#include <iostream>
#include <new>

template <typename T>
class CustomMemoryAllocator
{
public:
    using value_type = T;
    CustomMemoryAllocator() = default;
    T* allocate(std::size_t n);
    void deallocate(T* p, std::size_t n);
private:
    static inline bool use_large_pages_ = false;
    void* AllocateLargePages(std::size_t size);
    void DeallocateLargePages(void* p, std::size_t n);
    #ifdef _WIN32
    void* AllocateLargePagesWindows(std::size_t size);
    #elif defined(__linux__)
    void* AllocateLargePagesLinux(std::size_t size);
    #endif
};

template <typename T>
T* CustomMemoryAllocator<T>::allocate(std::size_t n)
{
    {
        std::size_t size = n * sizeof(T);
        void* ptr = AllocateLargePages(size);
        // Fallback to normal allocation if large pages fail
        if (!ptr) {
            #ifdef _WIN32
            ptr = _aligned_malloc(size, alignof(T));
            #elif defined(__linux__) || defined(__APPLE__)
            if (posix_memalign(&ptr, alignof(T), size) != 0) {
                ptr = nullptr;
            }
            #endif
        }

        if (!ptr) {
            throw std::bad_alloc();
        }

        return static_cast<T*>(ptr);
    }
}

template <typename T>
void CustomMemoryAllocator<T>::deallocate(T* p, std::size_t n)
{
    std::size_t size = n * sizeof(T);
    if (use_large_pages_) {
        DeallocateLargePages(p, size);
    }
    else {
        std::free(p);
    }
}

template <typename T>
void* CustomMemoryAllocator<T>::AllocateLargePages(std::size_t size)
{
    #ifdef _WIN32
    return AllocateLargePagesWindows(size);
    #elif defined(__linux__)
    return AllocateLargePagesLinux(size);
    #else
    return nullptr; // macOS falls back to normal allocation
    #endif
}

template <typename T>
void CustomMemoryAllocator<T>::DeallocateLargePages(void* p, std::size_t n)
{
    #ifdef _WIN32
    VirtualFree(p, 0, MEM_RELEASE);
    #elif defined(__linux__)
    munmap(p, n);
    #endif
}

#ifdef _WIN32
template <typename T>
void* CustomMemoryAllocator<T>::AllocateLargePagesWindows(std::size_t size)
{
    static SIZE_T large_page_size = 0;

    if (large_page_size == 0) {
        large_page_size = GetLargePageMinimum();
        if (large_page_size == 0) {
            return nullptr; // Large pages not supported
        }
    }

    // Round up size to a multiple of the large page size
    size = (size + large_page_size - 1) & ~(large_page_size - 1);

    HANDLE token;
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token)) {
        TOKEN_PRIVILEGES tp;
        LookupPrivilegeValue(nullptr, SE_LOCK_MEMORY_NAME, &tp.Privileges[0].Luid);
        tp.PrivilegeCount = 1;
        tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
        AdjustTokenPrivileges(token, FALSE, &tp, 0, nullptr, nullptr);
        CloseHandle(token);
    }

    void* ptr = VirtualAlloc(nullptr, size, MEM_RESERVE | MEM_COMMIT | MEM_LARGE_PAGES, PAGE_READWRITE);
    if (ptr) {
        use_large_pages_ = true;
    }
    return ptr;
}
#elif defined(__linux__)
template <typename T>
void* CustomMemoryAllocator<T>::AllocateLargePagesLinux(std::size_t size)
{
    // Try Huge Pages first
    void* ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
    if (ptr != MAP_FAILED) {
        use_large_pages_ = true;
        return ptr;
    }

    // Fallback to normal mmap (which may still use Transparent Huge Pages)
    ptr = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr != MAP_FAILED) {
        return ptr;
    }

    return nullptr; // mmap failed
}
#endif