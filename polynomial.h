#pragma once

#include "timer.h"
#include <QPolygonF>

using PointF = QPointF;
//using PolygonF = std::vector<PointF>;
using PolygonF = QPolygonF;

//template <class ld = long double, index_t MaxDdegree = 32>
using ld = long double;
using index_t = uint8_t;

class Polynomial {
    static constexpr index_t MaxDegree = 32;

    std::vector<ld> coeff; //результат
    PolygonF data; //массив входных данных

    inline ld calcPoly(ld x, index_t size = {});

public:
    Polynomial();

    void addData(double x, double y);
    void addData(const PointF& xy);
    void setData(const PolygonF& xy);

    void calcCoefCt(index_t D);
    void calcCoefRt(index_t D);

    PolygonF calcData(PolygonF in);

    std::vector<double> getCoeff() const;
};
