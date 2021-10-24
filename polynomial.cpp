#include "polynomial.h"

#include <QDebug>
#include <QPolygonF>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

template <index_t Size, class T = ld>
using array = std::array<T, Size>;

template <index_t N = 0>
struct It {
};

struct calcDegrees_i {
    virtual std::vector<ld> calcDegrees(const Data&) = 0;
};

//#define __CT__ // Скорее это бенчмарк крмпилятора,а не алгоритма.
#ifdef __CT__
template <index_t D>
struct calcDegreesCt final : calcDegrees_i {
    std::vector<ld> calcDegrees(const Data& data) override {
        constexpr index_t degree = D + 1;
        Timer { __FUNCTION__ };
        array<degree, array<degree>> matrix {};
        array<degree> y {};
        array<degree> c {}; //результат

        constexpr auto I = [](index_t I) { return D - I; };
        constexpr auto J = [](index_t J) { return D - J; };
        constexpr auto K = [](index_t K) { return D - K; };

        { //построение исходной матрицы
            // Timer t { 1 };
            auto forMatrix = [&]<index_t J, index_t... Is>(ld x, It<J>, Seq<Is...>) {
                ((matrix[Is][J] += pow(x, J + Is)), ...);
            };
            auto forY = [&]<index_t J>(It<J>) {
                for (auto&& var : data) {
                    y[J] += pow(var.x(), J) * var.y();
                    forMatrix(var.x(), It<J> {}, MakeSeq<degree> {});
                }
            };
            [&]<index_t... Js>(Seq<Js...>) { (forY(It<Js> {}), ...); }
            (MakeSeq<degree> {});
        }

        { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
            // Timer t { 2 };
            auto for3 = [&]<index_t I, index_t K, index_t... Js>(It<I>, It<K>, ld && Koef, Seq<Js...>) {
                ((matrix[Js][K] = (Js == I) ? ld {} : matrix[Js][K] * Koef - matrix[Js][I]), ...);
                y[K] = y[K] * Koef - y[I];
            };
            auto for2 = [&]<index_t I, index_t... Ks>(It<I>, Seq<Ks...>) {
                ((matrix[I][K(Ks)] != 0 ? for3(It<I> {}, It<K(Ks)> {}, matrix[I][I] / matrix[I][K(Ks)], MakeSeq<degree> {}) : void(Ks)), ...);
            };
            [&]<index_t... I>(Seq<I...>) { (for2(It<I> {}, MakeSeq<D - I> {}), ...); }
            (MakeSeq<degree> {});
        }

        { //поиск коэффициэнтов C
            // Timer t { 3 };
            auto for2 = [&]<index_t I, index_t... Js>(It<I>, Seq<Js...>) {
                ((c[I] -= matrix[J(Js)][I] * c[J(Js)] / matrix[I][I]), ...);
            };
            [&]<index_t... Is>(Seq<Is...>) {
                ((c[I(Is)] = y[I(Is)] / matrix[I(Is)][I(Is)], for2(It<I(Is)> {}, MakeSeq<D - I(Is)> {})), ...);
            }
            (MakeSeq<degree> {});
            std::vector<ld> coeff(degree);
            for (int i = 0; i < degree; ++i)
                coeff[i] = c[i];
            return coeff;
        }
    }
};

template <index_t... Is>
auto arrayOfCalcCt(Seq<Is...>) {
    static constexpr auto size = 8; // sizeof(calcDegreesCt<0>);
    alignas(alignof(calcDegrees_i*)) static char placeHolder[size * sizeof...(Is)] = {};
    static calcDegrees_i* funcs[] = { new (placeHolder + Is * size) calcDegreesCt<Is>... };
    return ((funcs));
};
#else

template <index_t D>
struct calcDegreesRt final : calcDegrees_i {
    std::vector<ld> calcDegrees(const Data& data) override {
        constexpr index_t degree = D + 1;
        Timer { __FUNCTION__ };
        array<degree, array<degree>> matrix {};
        array<degree> y {};
        array<degree> c {}; //результат

        { //построение исходной матрицы
            // Timer t { 5 };

            for (index_t j = 0; j < degree; j++) {
                for (auto&& var : data) {
                    y[j] += pow(var.x(), j) * var.y();
                    for (index_t i = 0; i < degree; i++) {
                        matrix[i][j] += pow(var.x(), j + i);
                    }
                }
            }
        }

        { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
            // Timer t { 6 };
            for (index_t i = 0; i < degree; i++) {
                for (index_t k = i + 1; k < degree; k++) {
                    if (matrix[i][k] != 0) {
                        ld Koef = matrix[i][i] / matrix[i][k];
                        for (index_t j = 0; j < degree; j++) {
                            matrix[j][k] = (j == i) ? ld {} : matrix[j][k] * Koef - matrix[j][i];
                        }
                        y[k] = y[k] * Koef - y[i];
                    }
                }
            }
        }

        { //поиск коэффициэнтов C
            // Timer t { 7 };
            for (index_t i = degree - 1; i != std::numeric_limits<index_t>::max(); --i) {
                c[i] = y[i] / matrix[i][i];
                for (index_t j = i + 1; j < degree; ++j)
                    c[i] -= matrix[j][i] * c[j] / matrix[i][i];
            }
            std::vector<ld> coeff(degree);
            for (int i = 0; i < degree; ++i)
                coeff[i] = c[i];
            return coeff;
        }
    }
};

template <index_t... Is>
auto arrayOfCalcRt(Seq<Is...>) {
    static constexpr auto size = 8; // sizeof(calcDegreesRt<0>);
    alignas(alignof(calcDegrees_i*)) static char placeHolder[size * sizeof...(Is)] = {};
    static calcDegrees_i* funcs[] = { new (placeHolder + Is * size) calcDegreesRt<Is>... };
    return ((funcs));
};

#endif

int Polynomial::precision() const { return precision_; }

void Polynomial::setPrecision(int newPrecision) { precision_ = newPrecision; }

bool Polynomial::usePrecision() const { return usePrecision_; }

void Polynomial::setUsePrecision(bool newUsePrecision) { usePrecision_ = newUsePrecision; }

ld Polynomial::calcPoly(ld x, std::vector<ld>& degrees, index_t size) {
    if (!degrees.size())
        return {};
    return (size < degrees.size() - 2) ? degrees[size] + x * calcPoly(x, degrees, size + 1) : degrees[size] + x * degrees[size + 1];
}

Polynomial::Polynomial() { }

void Polynomial::addData(double x, double y) { data_.push_back({ x, y }); }

void Polynomial::addData(const DataPoint& xy) { data_.push_back(xy); }

void Polynomial::setData(const Data& xy) { data_ = xy; }

void Polynomial::clear() {
    degrees_.clear();
    degreesPrec_.clear();
    data_.clear();
}

void Polynomial::calcDegrees(index_t D) {
    if (D >= MaxDegree + 1 || D < 1)
        return;
    try {
        degrees_.clear();
        degreesPrec_.clear();
#ifdef __CT__
        m_degrees = arrayOfCalcCt(MakeSeq<MaxDegree + 1> {})[D]->calcDegrees(m_data);
#else
        degrees_ = arrayOfCalcRt(MakeSeq<MaxDegree + 1> {})[D]->calcDegrees(data_);
        const int prec = pow(10, precision_);
        for (auto&& deg : degrees_) {
            degreesPrec_.push_back(floor(deg * prec) / prec);
        }
#endif
    } catch (...) {
        return;
    }
    emit degreesChanged(degrees());
    calcDataAndDeltaErr(data_);
}

Data Polynomial::calcDataAndDeltaErr(Data in) {
    delta_.clear();
    delta_.reserve(in.size());

    for (auto&& point : in) {
        delta_.push_back({ point.x(), static_cast<qreal>(calcPoly(point.x(), usePrecision_ ? degreesPrec_ : degrees_)) });
        delta_.back().ry() -= point.y();
        point.ry() = calcPoly(point.x(), usePrecision_ ? degreesPrec_ : degrees_);
    }
    emit dataChanged(in);
    emit deltaChanged(delta_);
    return in;
}

Degrees Polynomial::degrees() const {
    Degrees copy(degrees_.size(), 0.0);
    for (size_t i = 0; i < degrees_.size(); ++i)
        copy[i] = degrees_[i];
    return copy;
}

void Polynomial::setDegrees(const Degrees& degrees) {
    degrees_.resize(degrees.size());
    for (size_t i = 0; i < degrees_.size(); ++i)
        degrees_[i] = degrees[i];
}
