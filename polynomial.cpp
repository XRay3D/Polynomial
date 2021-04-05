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
        Timer{__FUNCTION__};
        array<degree, array<degree>> matrix{};
        array<degree> y{};
        array<degree> c{}; //результат

        constexpr auto I = [](index_t I) { return D - I; };
        constexpr auto J = [](index_t J) { return D - J; };
        constexpr auto K = [](index_t K) { return D - K; };

        { //построение исходной матрицы
            // Timer t { 1 };
            auto forMatrix = [&]<index_t J, index_t... Is>(ld x, It<J>, Seq<Is...>) {
                ((matrix[Is][J] += pow(x, J + Is)), ...);
            };
            auto forY = [&]<index_t J>(It<J>) {
                for(auto&& var : data) {
                    y[J] += pow(var.x(), J) * var.y();
                    forMatrix(var.x(), It<J>{}, MakeSeq<degree>{});
                }
            };
            [&]<index_t... Js>(Seq<Js...>) { (forY(It<Js>{}), ...); }
            (MakeSeq<degree>{});
        }

        { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
            // Timer t { 2 };
            auto for3 = [&]<index_t I, index_t K, index_t... Js>(It<I>, It<K>, ld && Koef, Seq<Js...>) {
                ((matrix[Js][K] = (Js == I) ? ld{} : matrix[Js][K] * Koef - matrix[Js][I]), ...);
                y[K] = y[K] * Koef - y[I];
            };
            auto for2 = [&]<index_t I, index_t... Ks>(It<I>, Seq<Ks...>) {
                ((matrix[I][K(Ks)] != 0 ? for3(It<I>{}, It<K(Ks)>{}, matrix[I][I] / matrix[I][K(Ks)], MakeSeq<degree>{}) : void(Ks)), ...);
            };
            [&]<index_t... I>(Seq<I...>) { (for2(It<I>{}, MakeSeq<D - I>{}), ...); }
            (MakeSeq<degree>{});
        }

        { //поиск коэффициэнтов C
            // Timer t { 3 };
            auto for2 = [&]<index_t I, index_t... Js>(It<I>, Seq<Js...>) {
                ((c[I] -= matrix[J(Js)][I] * c[J(Js)] / matrix[I][I]), ...);
            };
            [&]<index_t... Is>(Seq<Is...>) {
                ((c[I(Is)] = y[I(Is)] / matrix[I(Is)][I(Is)], for2(It<I(Is)>{}, MakeSeq<D - I(Is)>{})), ...);
            }
            (MakeSeq<degree>{});
            std::vector<ld> coeff(degree);
            for(int i = 0; i < degree; ++i)
                coeff[i] = c[i];
            return coeff;
        }
    }
};

template <index_t... Is>
auto arrayOfCalcCt(Seq<Is...>) {
    static constexpr auto size = 8; // sizeof(calcDegreesCt<0>);
    static char placeHolder[size * sizeof...(Is)] = {};
    static calcDegrees_i* funcs[] = {new(placeHolder + Is * size) calcDegreesCt<Is>...};
    return ((funcs));
};
#else

template <index_t D>
struct calcDegreesRt final : calcDegrees_i {
    std::vector<ld> calcDegrees(const Data& data) override {
        constexpr index_t degree = D + 1;
        Timer{__FUNCTION__};
        array<degree, array<degree>> matrix{};
        array<degree> y{};
        array<degree> c{}; //результат

        { //построение исходной матрицы
            // Timer t { 5 };

            for(index_t j = 0; j < degree; j++) {
                for(auto&& var : data) {
                    y[j] += pow(var.x(), j) * var.y();
                    for(index_t i = 0; i < degree; i++) {
                        matrix[i][j] += pow(var.x(), j + i);
                    }
                }
            }
        }

        { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
            // Timer t { 6 };
            for(index_t i = 0; i < degree; i++) {
                for(index_t k = i + 1; k < degree; k++) {
                    if(matrix[i][k] != 0) {
                        ld Koef = matrix[i][i] / matrix[i][k];
                        for(index_t j = 0; j < degree; j++) {
                            matrix[j][k] = (j == i) ? ld{} : matrix[j][k] * Koef - matrix[j][i];
                        }
                        y[k] = y[k] * Koef - y[i];
                    }
                }
            }
        }

        { //поиск коэффициэнтов C
            // Timer t { 7 };
            for(index_t i = degree - 1; i != std::numeric_limits<index_t>::max(); --i) {
                c[i] = y[i] / matrix[i][i];
                for(index_t j = i + 1; j < degree; ++j)
                    c[i] -= matrix[j][i] * c[j] / matrix[i][i];
            }
            std::vector<ld> coeff(degree);
            for(int i = 0; i < degree; ++i)
                coeff[i] = c[i];
            return coeff;
        }
    }
};

template <index_t... Is>
auto arrayOfCalcRt(Seq<Is...>) {
    static constexpr auto size = 8; // sizeof(calcDegreesRt<0>);
    static char placeHolder[size * sizeof...(Is)] = {};
    static calcDegrees_i* funcs[] = {new(placeHolder + Is * size) calcDegreesRt<Is>...};
    return ((funcs));
};

#endif

ld Polynomial::calcPoly(ld x, index_t size) {
    if(!m_degrees.size())
        return {};
    return (size < m_degrees.size() - 2) ? m_degrees[size] + x * calcPoly(x, size + 1)
                                         : m_degrees[size] + x * m_degrees[size + 1];
}

Polynomial::Polynomial() { }

void Polynomial::addData(double x, double y) { m_data.push_back({x, y}); }

void Polynomial::addData(const DataPoint& xy) { m_data.push_back(xy); }

void Polynomial::setData(const Data& xy) { m_data = xy; }

void Polynomial::clear() {
    m_degrees.clear();
    m_data.clear();
}

void Polynomial::calcDegrees(index_t D) {
    if(D >= MaxDegree + 1 || D < 1)
        return;
    try {
#ifdef __CT__
        m_degrees = arrayOfCalcCt(MakeSeq<MaxDegree + 1>{})[D]->calcDegrees(m_data);
#else
        m_degrees = arrayOfCalcRt(MakeSeq<MaxDegree + 1>{})[D]->calcDegrees(m_data);
#endif
    } catch(...) {
        return;
    }
    emit degreesChanged(degrees());
    emit dataChanged(calcData(m_data));
    emit deltaChanged(m_delta);
}

Data Polynomial::calcData(Data in) {
    m_delta.clear();
    m_delta.reserve(in.size());

    for(auto&& point : in) {
        m_delta.push_back({point.x(), static_cast<qreal>(calcPoly(point.x()))});
        m_delta.back().ry() -= point.y();
        point.ry() = calcPoly(point.x());
    }
    return in;
}

Degrees Polynomial::degrees() const {
    Degrees copy(m_degrees.size(), 0.0);
    for(size_t i = 0; i < m_degrees.size(); ++i)
        copy[i] = m_degrees[i];
    return copy;
}

void Polynomial::setDegrees(const Degrees& degrees) {
    m_degrees.resize(degrees.size());
    for(size_t i = 0; i < m_degrees.size(); ++i)
        m_degrees[i] = degrees[i];
}
