#pragma once

#include <chrono>
#include <span>
#include <vector>

#include <QDebug>
#include <QPolygonF>

//using PolygonF = std::vector<PointF>;
using Degrees = std::vector<double>;
using Data = QPolygonF;
using DataPoint = QPointF;

using index_t = uint8_t;

template <index_t size>
using MakeSeq = std::make_integer_sequence<index_t, size>;

template <index_t... is>
using Seq = std::integer_sequence<index_t, is...>;

inline constexpr index_t MaxDegree = 32;

namespace chrono = std::chrono;

struct Clipboard {
    virtual QString copy() const = 0;
    virtual void paste(QString&&) = 0;
};

struct Timer {

#ifdef __gnu_linux__
    chrono::time_point<chrono::system_clock, chrono::nanoseconds> t1;
#else
    chrono::time_point<chrono::steady_clock> t1;
#endif
    static inline std::map<const char*, double> avg{};
    static inline std::map<const char*, size_t> ctr{};
    std::string_view string_view;
    Timer(std::string_view span)
        : t1{chrono::high_resolution_clock::now()}
        , string_view{span} {
        //        avg = fl ? avg : double {};
        //        ctr = fl ? ctr : int {};
    }
    ~Timer() {
        using chrono::duration;
        using chrono::high_resolution_clock;
        using chrono::milliseconds;
        duration<double, std::micro> ms_double{chrono::high_resolution_clock::now() - t1};
        avg[string_view.data()] += ms_double.count();
        qDebug() << "time (" << string_view.data() << ")" << (avg[string_view.data()] / ++ctr[string_view.data()]) << "us";
    }
};
