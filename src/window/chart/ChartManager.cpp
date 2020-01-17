/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * NIST-developed software is provided by NIST as a public service. You may use,
 * copy and distribute copies of the software in any medium, provided that you
 * keep intact this entire notice. You may improve,modify and create derivative
 * works of the software or any portion of the software, and you may copy and
 * distribute such modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and nature of
 * any such change. Please explicitly acknowledge the National Institute of
 * Standards and Technology as the source of the software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES NO
 * WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
 * LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
 * AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE
 * OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT
 * ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY
 * REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF,
 * INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY,
 * OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of using and
 * distributing the software and you assume all risks associated with its use,
 * including but not limited to the risks and costs of program errors,
 * compliance with applicable laws, damage to or loss of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 *
 * Author: Evan Black <evan.black@nist.gov>
 */

#include "ChartManager.h"
#include <QConstOverload>
#include <QString>
#include <stdexcept>
#include <utility>

namespace {

struct AxisRange {
  qreal min;
  qreal max;
};

AxisRange getRange(visualization::ValueAxis::Scale scale, QtCharts::QAbstractAxis *axis) {
  AxisRange range{};

  if (scale == visualization::ValueAxis::Scale::Linear) {
    const auto valueAxis = qobject_cast<QtCharts::QValueAxis *>(axis);
    range.min = valueAxis->min();
    range.max = valueAxis->max();
  } else {
    const QLogValueAxis *logAxis = qobject_cast<QtCharts::QLogValueAxis *>(axis);
    range.min = logAxis->min();
    range.max = logAxis->max();
  }
  return range;
}

} // namespace

namespace visualization {

ChartManager::ChartManager(QWidget *parent) : QWidget(parent) {
  ui->setupUi(this);

  ui->comboBoxSeries->addItem("Select Series", 0u);

  chart.legend()->setVisible(true);
  chart.legend()->setAlignment(Qt::AlignBottom);

  ui->chartView->setChart(&chart);
  ui->chartView->setRenderHints(QPainter::Antialiasing);

  QObject::connect(ui->comboBoxSeries, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &ChartManager::seriesSelected);
}

ChartManager::~ChartManager() {
  delete ui;
}

void ChartManager::seriesSelected(int index) {
  auto seriesId = ui->comboBoxSeries->itemData(index).toUInt();
  showSeries(seriesId);
}

void ChartManager::addSeries(const XYSeries &s) {
  ChartManager::XYSeriesTie tie;
  tie.model = s;
  switch (s.connection) {
  case XYSeries::Connection::None:
    tie.qtSeries = new QtCharts::QScatterSeries(this);
    break;
  case XYSeries::Connection::Line:
    tie.qtSeries = new QtCharts::QLineSeries(this);
    break;
  case XYSeries::Connection::Spline:
    tie.qtSeries = new QtCharts::QSplineSeries(this);
    break;
  }

  tie.qtSeries->setColor(QColor::fromRgb(s.red, s.green, s.blue, s.alpha));
  tie.qtSeries->setName(QString::fromStdString(s.name));
  tie.qtSeries->setPointLabelsVisible(true);

  // X Axis
  if (tie.model.xAxis.scale == ValueAxis::Scale::Linear)
    tie.xAxis = new QtCharts::QValueAxis(this);
  else
    tie.xAxis = new QtCharts::QLogValueAxis(this);

  tie.xAxis->setTitleText(QString::fromStdString(s.xAxis.name));
  tie.xAxis->setRange(s.xAxis.min, s.xAxis.max);

  // Y Axis
  if (tie.model.yAxis.scale == ValueAxis::Scale::Linear)
    tie.yAxis = new QtCharts::QValueAxis(this);
  else
    tie.yAxis = new QtCharts::QLogValueAxis(this);
  tie.yAxis = new QtCharts::QValueAxis(this);
  tie.yAxis->setTitleText(QString::fromStdString(s.yAxis.name));
  tie.yAxis->setRange(s.xAxis.min, s.yAxis.max);

  series.insert({s.id, tie});
  ui->comboBoxSeries->addItem(QString::fromStdString(s.name), s.id);
}

void ChartManager::showSeries(uint32_t seriesId) {
  // Remove old axes
  auto currentAxes = chart.axes();

  // Remove currently attached series
  for (const auto &currentSeries : chart.series()) {
    // Detach the axes from each series,
    // then they may be removed from the chart
    for (const auto &axis : currentAxes) {
      currentSeries->detachAxis(axis);
    }

    chart.removeSeries(currentSeries);
    // The chart claims ownership of the series when it's attached
    currentSeries->setParent(this);
  }

  for (const auto &axis : currentAxes) {
    chart.removeAxis(axis);

    // Reclaim ownership of the axis
    // Since the chart takes it when it is attached
    axis->setParent(this);
  }

  const auto &seriesIterator = series.find(seriesId);
  if (seriesIterator == series.end()) {
    // Clear the old series title,
    // since we don't have a new one to overwrite the old one
    chart.setTitle("");
    return;
  }

  auto &seriesValue = seriesIterator->second;

  chart.setTitle(QString::fromStdString(seriesValue.model.name));

  // Qt wants the series on the chart before the axes
  chart.addSeries(seriesValue.qtSeries);

  chart.addAxis(seriesValue.xAxis, Qt::AlignBottom);
  chart.addAxis(seriesValue.yAxis, Qt::AlignLeft);

  // The series may only be attached to an axis _after_ both have
  // been added to the chart...
  seriesValue.qtSeries->attachAxis(seriesValue.xAxis);
  seriesValue.qtSeries->attachAxis(seriesValue.yAxis);
}

void ChartManager::timeAdvanced(double time) {
  auto handleEvent = [time, this](auto &&e) {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(e)>;

    if (e.time > time)
      return false;

    if constexpr (std::is_same_v<T, XYSeriesAddValue>) {
      const auto &s = series[e.seriesId];
      if (s.model.xAxis.boundMode == ValueAxis::BoundMode::HighestValue) {
        auto range = getRange(s.model.xAxis.scale, s.xAxis);
        if (e.x > range.max)
          s.xAxis->setMax(e.x);
        if (e.x < range.min)
          s.xAxis->setMin(e.x);
      }
      if (s.model.yAxis.boundMode == ValueAxis::BoundMode::HighestValue) {
        auto range = getRange(s.model.yAxis.scale, s.yAxis);
        if (e.x > range.max)
          s.yAxis->setMax(e.x);
        if (e.x < range.min)
          s.yAxis->setMin(e.x);
      }

      s.qtSeries->append(e.x, e.y);
      ui->chartView->repaint();
      events.pop_front();
      return true;
    }

    // Unhandled event type
    return false;
  };

  while (!events.empty() && std::visit(handleEvent, events.front())) {
    // Intentionally Blank
  }
}
void ChartManager::enqueueEvent(const visualization::ChartEvent &e) {
  events.emplace_back(e);
}

} // namespace visualization
