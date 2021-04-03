#include "polynomial.h"

#include <QDebug>
#include <QPolygonF>
#include <array>
#include <cmath>
#include <utility>
#include <vector>

template <index_t size>
using MakeSeq = std::make_integer_sequence<index_t, size>;

template <index_t... is>
using Seq = std::integer_sequence<index_t, is...>;

template <index_t Size, class T = ld>
using array = std::array<T, Size>;

template <index_t N = 0>
struct It {
};

struct calc_i {
    virtual std::vector<ld> calc(const PolygonF&) = 0;
};
#ifdef __CT__
template <index_t D, index_t degree = D + 1>
struct calcCt final : calc_i {
    std::vector<ld> calc(const PolygonF& data) override
    {
        qDebug(__FUNCTION__);
        // Timer t { 0 };
        array<degree, array<degree>> matrix {};
        array<degree> y {};
        array<degree> c {}; //результат

        constexpr auto K = [](index_t K) { return D - K; };
        constexpr auto J = [](index_t J) { return D - J; };
        constexpr auto I = [](index_t I) { return D - I; };

        { //построение исходной матрицы
            // Timer t { 1 };
            auto forMatrix = [&]<index_t J, index_t... Is>(ld x, It<J>, Seq<Is...>)
            {
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
            auto for3 = [&]<index_t I, index_t K, index_t... Js>(It<I>, It<K>, ld && Koef, Seq<Js...>)
            {
                ((matrix[Js][K] = (Js == I) ? ld {} : matrix[Js][K] * Koef - matrix[Js][I]), ...);
                y[K] = y[K] * Koef - y[I];
            };
            auto for2 = [&]<index_t I, index_t... Ks>(It<I>, Seq<Ks...>)
            {
                ((matrix[I][K(Ks)] != 0 ? for3(It<I> {}, It<K(Ks)> {}, matrix[I][I] / matrix[I][K(Ks)], MakeSeq<degree> {}) : void(Ks)), ...);
            };
            [&]<index_t... I>(Seq<I...>) { (for2(It<I> {}, MakeSeq<D - I> {}), ...); }
            (MakeSeq<degree> {});
        }

        { //поиск коэффициэнтов C
            // Timer t { 3 };
            auto for2 = [&]<index_t I, index_t... Js>(It<I>, Seq<Js...>)
            {
                ((c[I] -= matrix[J(Js)][I] * c[J(Js)] / matrix[I][I]), ...);
            };
            [&]<index_t... Is>(Seq<Is...>)
            {
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
auto arrayOfCalcCt(Seq<Is...>)
{
    static constexpr auto size = 8; // sizeof(calcCt<0>);
    static char placeHolder[size * sizeof...(Is)] = {};
    static calc_i* funcs[] = { new (placeHolder + Is * size) calcCt<Is>... };
    return ((funcs));
};

#endif

template <index_t D, index_t degree = D + 1>
struct calcRt final : calc_i {
    std::vector<ld> calc(const PolygonF& data) override
    {
        qDebug(__FUNCTION__);
        // Timer t { 4 };

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
auto arrayOfCalcRt(Seq<Is...>)
{
    static constexpr auto size = 8; // sizeof(calcRt<0>);
    static char placeHolder[size * sizeof...(Is)] = {};
    static calc_i* funcs[] = { new (placeHolder + Is * size) calcRt<Is>... };
    return ((funcs));
};

ld Polynomial::calcPoly(ld x, index_t size)
{
    if (!coeff.size())
        return {};
    return (size < coeff.size() - 2) ? coeff[size] + x * calcPoly(x, size + 1)
                                     : coeff[size] + x * coeff[size + 1];
}

Polynomial::Polynomial() { }

void Polynomial::addData(double x, double y) { data.push_back({ x, y }); }

void Polynomial::addData(const PointF& xy) { data.push_back(xy); }

void Polynomial::setData(const PolygonF& xy) { data = xy; }

void Polynomial::clear()
{
    coeff.clear();
    data.clear();
}

void Polynomial::calcCoef(index_t D)
{
    if (D >= MaxDegree + 1 || D < 1)
        return;
    //    coeff = arrayOfCalcCt(MakeSeq<MaxDegree + 1> {})[D]->calc(data);
    coeff = arrayOfCalcRt(MakeSeq<MaxDegree + 1> {})[D]->calc(data);
}

PolygonF Polynomial::calcData(PolygonF in)
{
    for (auto&& point : in)
        point.ry() = calcPoly(point.x());
    return in;
}

std::vector<double> Polynomial::getCoeff() const
{
    std::vector<double> copy(coeff.size(), 0.0);
    for (size_t i = 0; i < coeff.size(); ++i)
        copy[i] = coeff[i];
    return copy;
}
