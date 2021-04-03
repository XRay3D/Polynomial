#pragma once

#include <QDebug>
#include <QPolygonF>
#include <QSettings>
#include <array>
#include <chrono>
#include <cmath>
#include <utility>
#include <vector>

using PointF = QPointF;
//using PolygonF = std::vector<PointF>;
using PolygonF = QPolygonF;

struct Timer {
    //    using high_resolution_clock = std::chrono::high_resolution_clock;
    //    using duration_cast = std::chrono::duration_cast;
    //    using duration = std::chrono::duration;
    //    using milliseconds = std::chrono::milliseconds;
    std::chrono::time_point<std::chrono::steady_clock> t1;
    static inline double avg {};
    static inline int ctr {};

    Timer()
        : t1 { std::chrono::high_resolution_clock::now() }
    {
    }
    ~Timer()
    {
        using std::chrono::high_resolution_clock;
        using std::chrono::duration_cast;
        using std::chrono::duration;
        using std::chrono::milliseconds;
        duration<double, std::micro> ms_double { std::chrono::high_resolution_clock::now() - t1 };
        avg += ms_double.count();
        qDebug() << "avg" << (avg / ++ctr) << "us";
    }
};

template <size_t N>
struct It {
};

template <size_t D, class ld = long double>
class Polynomial {
    static constexpr size_t degree = D + 1; //степень полинома

    template <size_t size = degree>
    using MakeSeq = std::make_index_sequence<size>;

    template <size_t... is>
    using Seq = std::index_sequence<is...>;

    template <class T, size_t Size>
    using array = std::array<T, Size>;

    array<ld, degree> coeff {}; //результат
    PolygonF data; //массив входных данных

public:
    Polynomial() { }

    void addData(double x, double y) { data.push_back({ x, y }); }
    void addData(const PointF& xy) { data.push_back(xy); }
    void setData(const PolygonF& xy) { data = xy; }

    void calcCoef()
    {
        Timer t;
        if constexpr (1) {

            array<ld, degree> y {};
            array<array<ld, degree>, degree> matrix {};

            { //построение исходной матрицы
                static auto forMatrix = [&matrix]<size_t J, size_t... I>(ld x, It<J>, Seq<I...>)
                {
                    ((matrix[I][J] = matrix[I][J] + pow_(x, MakeSeq<J + I> {})), ...);
                };
                static auto forY = [&y, this]<size_t J>(It<J>) {
                    for (auto&& var : data) {
                        y[J] = y[J] + pow_(var.x(), MakeSeq<J> {}) * var.y();
                        forMatrix(var.x(), It<J> {}, MakeSeq<degree> {});
                    }
                };
                auto ctFor = []<size_t... j>(Seq<j...>) { (forY(It<j> {}), ...); };
                ctFor(MakeSeq<degree> {});
            }

            { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
                if (0) {
                    static auto for3 = [&matrix, &y ]<size_t I, size_t K, size_t... J>(It<I>, It<K>, ld && Koef, Seq<J...>)
                    {
                        ((matrix[J][K] = (J == I) ? ld {} : matrix[J][K] * Koef - matrix[J][I]), ...);
                        y[K] = y[K] * Koef - y[I];
                    };
                    static auto for2 = [&matrix]<size_t I, size_t... K>(It<I>, Seq<K...>)
                    {
                        constexpr auto k = [](size_t K_) { return degree - K_ - 1; };
                        ((matrix[I][k(K)] != 0 ? for3(It<I> {}, It<k(K)> {}, matrix[I][I] / matrix[I][k(K)], MakeSeq<degree> {}) : void(K)), ...);
                    };
                    static auto for1 = [&matrix]<size_t... I>(Seq<I...>)
                    {
                        (for2(It<I> {}, MakeSeq<degree - I - 1> {}), ...);
                    };
                    for1(MakeSeq<degree> {});
                } else {
                    static auto for3 = [&matrix, &y ]<size_t... J>(size_t i, size_t k, ld Koef, Seq<J...>)
                    {
                        ((matrix[J][k] = (J == i) ? ld {} : matrix[J][k] * Koef - matrix[J][i]), ...);
                        y[k] = y[k] * Koef - y[i];
                    };
                    for (size_t i = 0; i < degree; i++) {
                        for (size_t k = i + 1; k < degree; k++) {
                            if (matrix[i][k] != 0) {
                                for3(i, k, matrix[i][i] / matrix[i][k], MakeSeq<degree> {});
                            }
                        }
                    }
                }
            }

            { //поиск коэффициэнтов C
                if constexpr (1) {
                    static auto for2 = [&matrix, this ]<size_t I, size_t... J>(It<I>, Seq<J...>)
                    {
                        static constexpr auto j = [](size_t J_) { return degree - J_ - 1; };
                        ((coeff[I] = coeff[I] - matrix[j(J)][I] * coeff[j(J)] / matrix[I][I]), ...);
                    };
                    [&matrix, &y, this ]<size_t... I>(Seq<I...>)
                    {
                        ((coeff[I] = y[I] / matrix[I][I], for2(It<I> {}, MakeSeq<degree - I - 1> {})), ...);
                    }
                    (MakeSeq<degree> {});
                } else {
                    for (size_t i = degree - 1; i != std::numeric_limits<size_t>::max(); --i) {
                        coeff[i] = y[i] / matrix[i][i];
                        for (size_t j = i + 1; j < degree; ++j)
                            coeff[i] = coeff[i] - matrix[j][i] * coeff[j] / matrix[i][i];
                    }
                }
            }
        } else {
            ld matrix[degree][degree] {};
            ld y[degree] {};
            { //построение исходной матрицы
                for (size_t j = 0; j < degree; j++) {
                    for (auto&& var : data) {
                        y[j] = y[j] + pow(var.x(), j) * var.y();
                        for (size_t i = 0; i < degree; i++) {
                            matrix[i][j] = matrix[i][j] + pow(var.x(), j + i);
                        }
                    }
                }
            }

            { //преобразование матрицы системы уравнений в диагональную а-ля половинчатую матрицу
                for (size_t i = 0; i < degree; i++) {
                    for (size_t k = i + 1; k < degree; k++) {
                        if (matrix[i][k] != 0) {
                            ld Koef = matrix[i][i] / matrix[i][k];
                            for (size_t j = 0; j < degree; j++) {
                                matrix[j][k] = (j == i) ? 0 : matrix[j][k] * Koef - matrix[j][i];
                            }
                            y[k] = y[k] * Koef - y[i];
                        }
                    }
                }
            }

            { //поиск коэффициэнтов C
                for (size_t i = degree - 1; i != std::numeric_limits<size_t>::max(); --i) {
                    coeff[i] = y[i] / matrix[i][i];
                    for (size_t j = i + 1; j < degree; ++j)
                        coeff[i] = coeff[i] - matrix[j][i] * coeff[j] / matrix[i][i];
                }
            }
        }
    }

    PolygonF calc(PolygonF in)
    {
        for (auto&& point : in) {
            //point.ry() = req(point.x(), 0);
            point.ry() = calc(point.x(), MakeSeq<degree> {});
        }
        return in;
    }

    std::vector<double> getCoeff() const
    {
        std::vector<double> copy(degree, 0.0);
        for (int i = 0; i < degree; ++i)
            copy[i] = coeff[i];
        return copy;
    }

private:
    template <size_t... ints>
    constexpr static ld pow_(ld x, Seq<ints...>)
    {
        auto X = [x](size_t) { return x; };
        return (X(ints) * ... * ld { 1 });
    }

    constexpr inline ld calc(ld x, size_t size)
    {
        return size < coeff.size() - 2 ? coeff[size] + x * calc(x, size + 1)
                                       : coeff[size] + x * coeff[size + 1];
    }

    template <size_t... ints>
    constexpr ld calc(ld x, Seq<ints...>)
    {
        return ((ints ? coeff[ints] * pow_(x, MakeSeq<ints> {}) : coeff[0]) + ...);
        return ((ints ? coeff[ints] * pow(x, ints) : coeff[0]) + ...);
    }
};
