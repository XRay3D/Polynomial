#pragma once

#include <chrono>

namespace chrono = std::chrono;

struct Timer {
#ifdef __gnu_linux__
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> t1;
#else
    chrono::time_point<chrono::steady_clock> t1;
#endif
    static inline double avg[100] {};
    static inline int ctr[100] {};
    size_t idx;
    Timer(size_t idx);
    ~Timer();
};
