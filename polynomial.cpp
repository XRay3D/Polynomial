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

template <index_t D, index_t degree = D + 1>
struct calcCt final : calc_i {
    array<degree, array<degree>> matrix {};
    array<degree> y {};
    array<degree> c {}; //результат
    //построение исходной матрицы

    std::vector<ld> calc(const PolygonF& data) override
    {
        qDebug(__FUNCTION__);
        TimerCt t;

        matrix = array<degree, array<degree>> {};
        y = array<degree> {};
        c = array<degree> {};

        { //построение исходной матрицы
            TimerCt t;
            auto forMatrix = [this]<index_t J, index_t... I>(ld x, It<J>, Seq<I...>)
            {
                ((matrix[I][J] += pow(x, J + I)), ...);
            };
            auto forY = [forMatrix, &data, this]<index_t J>(It<J>) {
                for (auto&& var : data) {
                    y[J] += pow(var.x(), J) * var.y();
                    forMatrix(var.x(), It<J> {}, MakeSeq<degree> {});
                }
            };
            [forY]<index_t... j>(Seq<j...>)
            {
                (forY(It<j> {}), ...);
            }
            (MakeSeq<degree> {});
        }

        { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
            TimerCt t;
            auto for3 = [this]<index_t I, index_t K, index_t... J>(It<I>, It<K>, ld && Koef, Seq<J...>)
            {
                ((matrix[J][K] = (J == I) ? ld {} : matrix[J][K] * Koef - matrix[J][I]), ...);
                y[K] = y[K] * Koef - y[I];
            };
            auto for2 = [ for3, this ]<index_t I, index_t... K>(It<I>, Seq<K...>)
            {
                constexpr auto k = [](index_t K_) { return D - K_; };
                ((matrix[I][k(K)] != 0 ? for3(It<I> {}, It<k(K)> {}, matrix[I][I] / matrix[I][k(K)], MakeSeq<degree> {}) : void(K)), ...);
            };
            [for2]<index_t... I>(Seq<I...>)
            {
                (for2(It<I> {}, MakeSeq<D - I> {}), ...);
            }
            (MakeSeq<degree> {});
        }

        { //поиск коэффициэнтов C
            TimerCt t;
            auto for2 = [this]<index_t I, index_t... J>(It<I>, Seq<J...>)
            {
                static constexpr auto j = [](index_t J_) { return D - J_; };
                ((c[I] -= matrix[j(J)][I] * c[j(J)] / matrix[I][I]), ...);
            };
            [ for2, this ]<index_t... I>(Seq<I...>)
            {
                constexpr auto i = [](index_t I_) { return D - I_; };
                ((c[i(I)] = y[i(I)] / matrix[i(I)][i(I)], for2(It<i(I)> {}, MakeSeq<D - i(I)> {})), ...);
            }
            (MakeSeq<degree> {});
            std::vector<ld> coeff(degree);
            for (int i = 0; i < degree; ++i)
                coeff[i] = c[i];
            return coeff;
        }
    }
};
template <index_t D, index_t degree = D + 1>
struct calcRt final : calc_i {
    std::vector<ld> calc(const PolygonF& data) override
    {
        qDebug(__FUNCTION__);
        TimerRt t;

        ld matrix[degree][degree] {};
        ld y[degree] {};

        { //построение исходной матрицы
            TimerRt t;
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
            TimerRt t;
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
            TimerRt t;
            ld c[degree] {}; //результат
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
auto arrayOfCalcCt(Seq<Is...>)
{
    static constexpr auto size = sizeof(calcCt<0>);
    static char placeHolder[size * sizeof...(Is)] = {};
    static calc_i* funcs[] = { new (placeHolder + Is * size) calcCt<Is>... };
    return ((funcs));
};

template <index_t... Is>
auto arrayOfCalcRt(Seq<Is...>)
{
    static constexpr auto size = sizeof(calcRt<0>);
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

void Polynomial::calcCoefCt(index_t D)
{
    if (D >= MaxDegree + 1 || D < 1)
        return;
    coeff = arrayOfCalcCt(MakeSeq<MaxDegree + 1> {})[D]->calc(data);
}

void Polynomial::calcCoefRt(index_t D)
{
    if (D >= MaxDegree + 1 || D < 1)
        return;
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
