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

View::View(QWidget* parent)
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
    chart->setMinimumSize(640, 480);
    chart->setTitle("Hover the line to show callout. Click the line to make it stay");
    chart->legend()->hide();
    QLineSeries* series = new QLineSeries;
    chart->addSeries(series);

    QSplineSeries* series2 = new QSplineSeries;
    chart->addSeries(series2);

    chart->createDefaultAxes();
    chart->setAcceptHoverEvents(true);

    setChart(chart);
    //scene()->addItem(chart());

    m_coordX = new QGraphicsSimpleTextItem(chart);
    m_coordX->setPos(chart->size().width() / 2 - 50, chart->size().height());
    m_coordX->setText("X: ");
    m_coordY = new QGraphicsSimpleTextItem(chart);
    m_coordY->setPos(chart->size().width() / 2 + 50, chart->size().height());
    m_coordY->setText("Y: ");

    connect(series, &QLineSeries::clicked, this, &View::keepCallout);
    connect(series, &QLineSeries::hovered, this, &View::tooltip);

    connect(series2, &QSplineSeries::clicked, this, &View::keepCallout);
    connect(series2, &QSplineSeries::hovered, this, &View::tooltip);
}

void View::setData(const QPolygonF& data)
{
    auto lineSeries = static_cast<QLineSeries*>(chart()->series().front());
    lineSeries->replace(data);
    auto [min, max] = rng::minmax(data, {}, [](const QPointF& p) { return p.y(); });
    qDebug() << __FUNCTION__ << min << max;
    mmx1 |= { data.front().x(), data.back().x() };
    mmy1 |= { min.y(), max.y() };
    chart()->axes(Qt::Horizontal, lineSeries).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, lineSeries).front()->setRange(mmy1.min, mmy1.max);
}

void View::setData3(const QPolygonF& data)
{
    auto lineSeries = static_cast<QLineSeries*>(chart()->series().back());
    lineSeries->replace(data);
    auto [min, max] = rng::minmax(data, {}, [](const QPointF& p) { return p.y(); });
    qDebug() << __FUNCTION__ << min << max;
    mmx1 |= { data.front().x(), data.back().x() };
    mmy1 |= { min.y(), max.y() };
    chart()->axes(Qt::Horizontal, lineSeries).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, lineSeries).front()->setRange(mmy1.min, mmy1.max);
}

void View::setData2(const QPolygonF& data)
{
    auto lineSeries = static_cast<QLineSeries*>(chart()->series().front());
    lineSeries->replace(data);
    auto [min, max] = rng::minmax(data, {}, [](const QPointF& p) { return p.y(); });
    auto maxY = std::max(abs(min.y()), abs(max.y()));
    mmx1 |= { data.front().x(), data.back().x() };
    mmy1 |= { -maxY, maxY };
    chart()->axes(Qt::Horizontal, lineSeries).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, lineSeries).front()->setRange(mmy1.min, mmy1.max);
}

void View::resizeEvent(QResizeEvent* event)
{
    QChartView::resizeEvent(event);
    if (scene()) {
        //        scene()->setSceneRect(QRect(QPoint(0, 0), event->size()));
        //        chart()->resize(event->size());
        m_coordX->setPos(chart()->size().width() / 2 - 50, chart()->size().height() - 20);
        m_coordY->setPos(chart()->size().width() / 2 + 50, chart()->size().height() - 20);
        const auto callouts = m_callouts;
        for (Callout* callout : callouts)
            callout->updateGeometry();
    }
}

void View::mouseMoveEvent(QMouseEvent* event)
{
    auto pos = chart()->mapToValue(event->pos());
    if (event->buttons() & Qt::RightButton) {
        qDebug() << event->buttons();
        auto d = m_pos - pos;
        chart()->scroll(d.x(), d.y());
    }
    m_coordX->setText(QString("X: %1").arg(pos.x()));
    m_coordY->setText(QString("Y: %1").arg(pos.y()));
    QChartView::mouseMoveEvent(event);
}

void View::wheelEvent(QWheelEvent* event)
{
    event->angleDelta().y() > 0 ? chart()->zoomIn() : chart()->zoomOut();
    QChartView::wheelEvent(event);
}

void View::mouseDoubleClickEvent(QMouseEvent* event)
{
    QChartView::mouseDoubleClickEvent(event);
    chart()->zoomReset();
    chart()->axes(Qt::Horizontal, nullptr).front()->setRange(mmx1.min, mmx1.max);
    chart()->axes(Qt::Vertical, nullptr).front()->setRange(mmy1.min, mmy1.max);
}
void View::mousePressEvent(QMouseEvent* event)
{
    QChartView::mousePressEvent(event);
    m_pos = chart()->mapToValue(event->pos());
}
void View::mouseReleaseEvent(QMouseEvent* event)
{
    if (event->button() & Qt::RightButton)
        return;
    QChartView::mouseReleaseEvent(event);
}

void View::keepCallout()
{
    m_callouts.append(m_tooltip);
    m_tooltip = new Callout(chart());
}

void View::tooltip(QPointF point, bool state)
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
