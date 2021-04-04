#pragma once

#include <chrono>
#include <vector>

#include <QDebug>
#include <QPolygonF>

//using PolygonF = std::vector<PointF>;
using Degrees = std::vector<double>;
using Data = QPolygonF;
using DataPoint = QPointF;

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
    Timer(size_t idx)
        : t1 { chrono::high_resolution_clock::now() }
        , idx { idx }
    {
        //        avg = fl ? avg : double {};
        //        ctr = fl ? ctr : int {};
    }
    ~Timer()
    {
        using chrono::duration;
        using chrono::high_resolution_clock;
        using chrono::milliseconds;
        duration<double, std::micro> ms_double { chrono::high_resolution_clock::now() - t1 };
        avg[idx] += ms_double.count();
        qDebug() << "time (average)" << (avg[idx] / ++ctr[idx]) << "us";
    }
};
