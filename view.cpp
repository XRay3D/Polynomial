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

#include "view.h"
#include "callout.h"
#include <QDebug>
#include <QtCharts/QChart>
#include <QtCharts/QLineSeries>
#include <QtCharts/QSplineSeries>
#include <QtGui/QMouseEvent>
#include <QtGui/QResizeEvent>
#include <QtWidgets/QGraphicsScene>
#include <QtWidgets/QGraphicsTextItem>
#include <ranges>

namespace rng = std::ranges;

ChartView::ChartView(QWidget* parent)
    : QChartView(parent)
    , m_coordX(0)
    , m_coordY(0)
    , m_tooltip(0)
{
    setDragMode(QChartView::NoDrag);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setRubberBand(RectangleRubberBand);
    setRenderHint(QPainter::Antialiasing);
    setMouseTracking(true);
    setInteractive(true);

    // chart
    auto chart = new QChart;
    //    chart->setMinimumSize(640, 480);
    //    chart->setTitle("Hover the line to show callout. Click the line to make it stay");
    //    chart->legend()->hide();
    chart->setMargins({ 6, 6, 6, 6 });

    QXYSeries* series[] { new QLineSeries, new QLineSeries };
    for (auto s : series) {
        chart->addSeries(s);
    }
    series[0]->setName("Данные");
    series[1]->setName("Перерасчет");
    series[0]->setPen(QPen(Qt::black, 2));
    series[1]->setPen(QPen(Qt::red, 2));

    chart->createDefaultAxes();
    chart->setAcceptHoverEvents(true);
    chart->setBackgroundRoundness(0);

    setChart(chart);

    m_coordX = new QGraphicsSimpleTextItem(chart);
    m_coordX->setPos(chart->size().width() / 2 - 50, chart->size().height());
    m_coordX->setText("X: ");

    m_coordY = new QGraphicsSimpleTextItem(chart);
    m_coordY->setPos(chart->size().width() / 2 + 50, chart->size().height());
    m_coordY->setText("Y: ");

    for (auto s : series) {
        connect(s, &QLineSeries::clicked, this, &ChartView::keepCallout);
        connect(s, &QLineSeries::hovered, this, &ChartView::tooltip);
    }
}

void ChartView::setData(const QPolygonF& data, int s)
{
    if (data.empty())
        return;
    auto lineSeries = static_cast<QLineSeries*>(chart()->series()[s]);
    lineSeries->replace(data);
    auto [min, max] = rng::minmax(data, {}, [](const QPointF& p) { return p.y(); });
    if (!s) {
        mmx1 = { data.front().x(), data.back().x() };
        mmy1 = { min.y(), max.y() };
    } else {
        mmx1 |= { data.front().x(), data.back().x() };
        mmy1 |= { min.y(), max.y() };
    }
    chart()->axes(Qt::Horizontal, lineSeries).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, lineSeries).front()->setRange(mmy1.min, mmy1.max);
}

void ChartView::setDeltaData(const QPolygonF& data)
{
    auto lineSeries = static_cast<QLineSeries*>(chart()->series()[1]);
    lineSeries->replace(data);
    auto [min, max] = rng::minmax(data, {}, [](const QPointF& p) { return p.y(); });
    auto maxY = std::max(abs(min.y()), abs(max.y()));

    mmx1 = { data.front().x(), data.back().x() };
    mmy1 = { -maxY, +maxY };

    chart()->axes(Qt::Horizontal, lineSeries).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, lineSeries).front()->setRange(mmy1.min, mmy1.max);
}

void ChartView::setPrec(int /*prec*/)
{
    chart()->axes(Qt::Horizontal, nullptr);
}

void ChartView::resizeEvent(QResizeEvent* event)
{
    QChartView::resizeEvent(event);
    if (scene()) {
        m_coordX->setPos(chart()->size().width() / 2 - 50, chart()->size().height() - 20);
        m_coordY->setPos(chart()->size().width() / 2 + 50, chart()->size().height() - 20);
        const auto callouts = m_callouts;
        for (Callout* callout : callouts)
            callout->updateGeometry();
    }
}

void ChartView::mouseMoveEvent(QMouseEvent* event)
{
    auto pos = mapToScene(event->pos());
    m_coordX->setText(QString("X: %1").arg(pos.x()));
    m_coordY->setText(QString("Y: %1").arg(pos.y()));
    QChartView::mouseMoveEvent(event);
}

void ChartView::wheelEvent(QWheelEvent* event)
{
    event->angleDelta().y() > 0 ? chart()->zoomIn() : chart()->zoomOut();
    QChartView::wheelEvent(event);
}

void ChartView::mouseDoubleClickEvent(QMouseEvent* event)
{
    QChartView::mouseDoubleClickEvent(event);
    chart()->zoomReset();
    chart()->axes(Qt::Horizontal, nullptr).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, nullptr).front()->setRange(mmy1.min, mmy1.max);
}

//void ChartView::mousePressEvent(QMouseEvent* event)
//{
//    QChartView::mousePressEvent(event);
//    //    m_pos = event->pos(); // chart()->mapToPosition(event->pos());
//}
//void ChartView::mouseReleaseEvent(QMouseEvent* event)
//{
//    if (event->button() & Qt::RightButton)
//        return;
//    QChartView::mouseReleaseEvent(event);
//}

void ChartView::keepCallout()
{
    m_callouts.append(m_tooltip);
    m_tooltip = new Callout(chart());
}

void ChartView::tooltip(QPointF point, bool state)
{
    if (m_tooltip == 0)
        m_tooltip = new Callout(chart());

    if (state) {
        m_tooltip->setText(QString("X: %1 \nY: %2 ").arg(point.x()).arg(point.y()));
        m_tooltip->setAnchor(point);
        m_tooltip->setZValue(11);
        m_tooltip->updateGeometry();
        m_tooltip->show();
    } else {
        m_tooltip->hide();
    }
}
