/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the Qt Charts module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:GPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 or (at your option) any later version
** approved by the KDE Free Qt Foundation. The licenses are as published by
** the Free Software Foundation and appearing in the file LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef CHARTVIEW_H
#define CHARTVIEW_H
#include <QChartView>
#include <QtCharts/QChartGlobal>
#include <QtWidgets/QGraphicsView>

QT_BEGIN_NAMESPACE
class QGraphicsScene;
class QMouseEvent;
class QResizeEvent;
QT_END_NAMESPACE

QT_CHARTS_BEGIN_NAMESPACE
class QChart;
QT_CHARTS_END_NAMESPACE

class Callout;

QT_CHARTS_USE_NAMESPACE

class ChartView : public QChartView {
    Q_OBJECT

public:
    ChartView(QWidget* parent = 0);
    void setData0(const QPolygonF& data);
    void setData1(const QPolygonF& data);
    void setDeltaData(const QPolygonF& data);
    void setPrec(int prec);

protected:
    void resizeEvent(QResizeEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

    //    void mousePressEvent(QMouseEvent* event) override;
    //    void mouseReleaseEvent(QMouseEvent* event) override;

public slots:
    void keepCallout();
    void tooltip(QPointF point, bool state);
    void init(const QStringList& names);

private:
    QGraphicsSimpleTextItem* m_coordX;
    QGraphicsSimpleTextItem* m_coordY;
    Callout* m_tooltip;
    QList<Callout*> m_callouts;
    QPointF m_pos;
    struct MinMax {
        void reset() {
            min = +std::numeric_limits<double>::max();
            max = -std::numeric_limits<double>::max();
        }
        double min = +std::numeric_limits<double>::max();
        double max = -std::numeric_limits<double>::max();
        auto operator+=(MinMax r) {
            min = std::min(min, r.min);
            max = std::max(max, r.max);
            return *this;
        }
    } mmx[2], mmy[2];
};

#endif
