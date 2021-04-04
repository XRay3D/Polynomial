#pragma once

#include "types.h"

//template <class ld = long double, index_t MaxDdegree = 32>
using ld = long double;
using index_t = uint8_t;

class Polynomial : public QObject {
    Q_OBJECT
    static constexpr index_t MaxDegree = 32;

    std::vector<ld> m_degrees; //результат
    Data m_data; //массив входных данных

    inline ld calcPoly(ld x, index_t size = {});

public:
    Polynomial();

    void addData(double x, double y);
    void addData(const DataPoint& xy);
    void setData(const Data& xy);
    void clear();

    void calcDegrees(index_t D);

    Data calcData(Data in);

    Degrees degrees() const;
    void setDegrees(const Degrees& degrees);
};
