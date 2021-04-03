#include "timer.h"
#include <QDebug>

TimerCt::TimerCt(bool fl)
    : t1 { chrono::high_resolution_clock::now() }
{
    //        avg = fl ? avg : double {};
    //        ctr = fl ? ctr : int {};
}

TimerCt::~TimerCt()
{
    using chrono::duration;
    using chrono::duration_cast;
    using chrono::high_resolution_clock;
    using chrono::milliseconds;
    duration<double, std::micro> ms_double { chrono::high_resolution_clock::now() - t1 };
    avg += ms_double.count();
    qDebug() << "avg" << (avg / ++ctr) << "us";
}

TimerRt::TimerRt(bool fl)
    : t1 { chrono::high_resolution_clock::now() }
{
    //        avg = fl ? avg : double {};
    //        ctr = fl ? ctr : int {};
}

TimerRt::~TimerRt()
{
    using chrono::duration;
    using chrono::duration_cast;
    using chrono::high_resolution_clock;
    using chrono::milliseconds;
    duration<double, std::micro> ms_double { chrono::high_resolution_clock::now() - t1 };
    avg += ms_double.count();
    qDebug() << "avg" << (avg / ++ctr) << "us";
}
