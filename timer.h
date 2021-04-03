#pragma once

#include <chrono>

namespace chrono = std::chrono;

struct TimerCt {
#ifdef __gnu_linux__
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> t1;
#else
    chrono::time_point<chrono::steady_clock> t1;
#endif
    static inline double avg {};
    static inline int ctr {};

    TimerCt(bool fl = {});
    ~TimerCt();
};

struct TimerRt {

#ifdef __gnu_linux__
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> t1;
#else
    chrono::time_point<chrono::steady_clock> t1;
#endif
    static inline double avg {};
    static inline int ctr {};

    TimerRt(bool fl = {});
    ~TimerRt();
};
