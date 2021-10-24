#pragma once

#include "types.h"

//template <class ld = long double, index_t MaxDdegree = 32>
using ld = long double;

class Polynomial : public QObject {
    Q_OBJECT

    std::vector<ld> degrees_; //результат
    std::vector<ld> degreesPrec_; //результат
    Data data_; //массив входных данных
    Data delta_;
    int precision_ { 6 };
    bool usePrecision_ {};
    inline ld calcPoly(ld x, std::vector<ld> &degrees, index_t size = {});

signals:
    void degreesChanged(const Degrees&);
    void dataChanged(const Data&);
    void deltaChanged(const Data&);

public:
    Polynomial();

    void addData(double x, double y);
    void addData(const DataPoint& xy);
    void setData(const Data& xy);
    void clear();

    void calcDegrees(index_t D);

    Data calcDataAndDeltaErr(Data in);

    Degrees degrees() const;
    void setDegrees(const Degrees& degrees);
    int precision() const;
    void setPrecision(int newPrecision);
    bool usePrecision() const;
    void setUsePrecision(bool newUsePrecision);
};
