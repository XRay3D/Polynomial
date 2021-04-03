#include "timer.h"
#include <QDebug>

Timer::Timer(size_t idx)
    : t1 { chrono::high_resolution_clock::now() }
    , idx { idx }
{
    //        avg = fl ? avg : double {};
    //        ctr = fl ? ctr : int {};
}

Timer::~Timer()
{
    using chrono::duration;
    using chrono::duration_cast;
    using chrono::high_resolution_clock;
    using chrono::milliseconds;
    duration<double, std::micro> ms_double { chrono::high_resolution_clock::now() - t1 };
    avg[idx] += ms_double.count();
    qDebug() << "avg" << (avg[idx] / ++ctr[idx]) << "us";
}
