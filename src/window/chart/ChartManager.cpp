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
#include <algorithm>
#include <iostream>
#include <stdexcept>
#include <utility>

namespace {

void updateRange(QtCharts::QAbstractAxis *axis, qreal value) {
  // Amount to scale past the min/max
  // so we don't cut off the actual point
  const auto additionalScale = 0.05;

  qreal min;
  qreal max;
  if (const auto valueAxis = qobject_cast<QtCharts::QValueAxis *>(axis)) {
    min = valueAxis->min();
    max = valueAxis->max();
  } else if (const auto logAxis = qobject_cast<QtCharts::QLogValueAxis *>(axis)) {
    min = logAxis->min();
    max = logAxis->max();
  } else {
    std::cerr << "Error: Unhandled axis type in updateRange()\n";
    return;
  }

  if (value > max)
    axis->setMax(value + value * additionalScale);
  else if (value < min)
    axis->setMin(value - value * additionalScale);
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
void ChartManager::clearChart() {
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

  chart.setTitle("");
}

void ChartManager::seriesSelected(int index) {
  auto seriesId = ui->comboBoxSeries->itemData(index).toUInt();
  showSeries(seriesId);
}

void ChartManager::reset() {
  ui->comboBoxSeries->clear();
  ui->comboBoxSeries->addItem("Select Series", 0u);

  clearChart();
  events.clear();

  for (auto &iterator : series) {
    auto &value = iterator.second;
    if (std::holds_alternative<XYSeriesTie>(value)) {
      auto qtSeries = std::get<XYSeriesTie>(value).qtSeries;
      qtSeries->setParent(nullptr);
      qtSeries->deleteLater();
    }
    // No need to handle SeriesCollection since it has no pointers
  }

  series.clear();
}

void ChartManager::addSeries(const parser::XYSeries &s) {
  ChartManager::XYSeriesTie tie;
  tie.model = s;
  switch (s.connection) {
  case parser::XYSeries::Connection::None:
    tie.qtSeries = new QtCharts::QScatterSeries(this);
    break;
  case parser::XYSeries::Connection::Line:
    tie.qtSeries = new QtCharts::QLineSeries(this);
    break;
  case parser::XYSeries::Connection::Spline:
    tie.qtSeries = new QtCharts::QSplineSeries(this);
    break;
  }

  switch (s.labelMode) {
  case parser::XYSeries::LabelMode::Hidden:
    tie.qtSeries->setPointLabelsVisible(false);
    break;
  case parser::XYSeries::LabelMode::Shown:
    tie.qtSeries->setPointLabelsVisible(true);
    break;
  }

  // Qt Charts OpenGL Does not play nice with OSG
  tie.qtSeries->setUseOpenGL(false);

  tie.qtSeries->setColor(QColor::fromRgb(s.red, s.green, s.blue, s.alpha));
  tie.qtSeries->setName(QString::fromStdString(s.name));

  // X Axis
  if (tie.model.xAxis.scale == parser::ValueAxis::Scale::Linear)
    tie.xAxis = new QtCharts::QValueAxis(this);
  else
    tie.xAxis = new QtCharts::QLogValueAxis(this);

  tie.xAxis->setTitleText(QString::fromStdString(s.xAxis.name));
  tie.xAxis->setRange(s.xAxis.min, s.xAxis.max);

  // Y Axis
  if (tie.model.yAxis.scale == parser::ValueAxis::Scale::Linear)
    tie.yAxis = new QtCharts::QValueAxis(this);
  else
    tie.yAxis = new QtCharts::QLogValueAxis(this);
  tie.yAxis = new QtCharts::QValueAxis(this);
  tie.yAxis->setTitleText(QString::fromStdString(s.yAxis.name));
  tie.yAxis->setRange(s.yAxis.min, s.yAxis.max);

  series.insert({s.id, tie});
  ui->comboBoxSeries->addItem(QString::fromStdString(s.name), s.id);
}

void ChartManager::addSeries(const parser::SeriesCollection &s) {
  ChartManager::SeriesCollectionTie tie;
  tie.model = s;

  // X Axis
  if (tie.model.xAxis.scale == parser::ValueAxis::Scale::Linear)
    tie.xAxis = new QtCharts::QValueAxis(this);
  else
    tie.xAxis = new QtCharts::QLogValueAxis(this);
  tie.xAxis->setTitleText(QString::fromStdString(s.xAxis.name));
  tie.xAxis->setRange(s.xAxis.min, s.xAxis.max);

  // Y Axis
  if (tie.model.yAxis.scale == parser::ValueAxis::Scale::Linear)
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
  clearChart();

  const auto &seriesIterator = series.find(seriesId);
  if (seriesIterator == series.end()) {
    // Clear the old series title,
    // since we don't have a new one to overwrite the old one
    chart.setTitle("");
    return;
  }

  auto &s = seriesIterator->second;
  if (std::holds_alternative<ChartManager::XYSeriesTie>(s))
    showSeries(std::get<ChartManager::XYSeriesTie>(s));
  else if (std::holds_alternative<ChartManager::SeriesCollectionTie>(s))
    showSeries(std::get<ChartManager::SeriesCollectionTie>(s));
}

void ChartManager::showSeries(const XYSeriesTie &tie) {
  chart.setTitle(QString::fromStdString(tie.model.name));

  // Qt wants the series on the chart before the axes
  chart.addSeries(tie.qtSeries);

  chart.addAxis(tie.xAxis, Qt::AlignBottom);
  chart.addAxis(tie.yAxis, Qt::AlignLeft);

  // The series may only be attached to an axis _after_ both have
  // been added to the chart...
  tie.qtSeries->attachAxis(tie.xAxis);
  tie.qtSeries->attachAxis(tie.yAxis);
}

void ChartManager::showSeries(const ChartManager::SeriesCollectionTie &tie) {
  chart.setTitle(QString::fromStdString(tie.model.name));

  for (auto seriesId : tie.model.series) {
    const auto &seriesVariant = series[seriesId];

    if (std::holds_alternative<ChartManager::XYSeriesTie>(seriesVariant)) {
      const auto &xySeries = std::get<ChartManager::XYSeriesTie>(seriesVariant);
      chart.addSeries(xySeries.qtSeries);
    }
  }

  chart.addAxis(tie.xAxis, Qt::AlignBottom);
  chart.addAxis(tie.yAxis, Qt::AlignLeft);

  for (auto chartSeries : chart.series()) {
    chartSeries->attachAxis(tie.xAxis);
    chartSeries->attachAxis(tie.yAxis);
  }
}

void ChartManager::updateCollectionRanges(uint32_t seriesId, double x, double y) {
  for (auto &iterator : series) {
    // Only update collections
    if (!std::holds_alternative<ChartManager::SeriesCollectionTie>(iterator.second))
      continue;

    auto &collection = std::get<ChartManager::SeriesCollectionTie>(iterator.second);

    // Only update the collection's ranges if it actually contains the series
    if (std::find(collection.model.series.begin(), collection.model.series.end(), seriesId) !=
        collection.model.series.end()) {
      if (collection.model.xAxis.boundMode == parser::ValueAxis::BoundMode::HighestValue)
        updateRange(collection.xAxis, x);
      if (collection.model.yAxis.boundMode == parser::ValueAxis::BoundMode::HighestValue)
        updateRange(collection.yAxis, y);
    }
  }
}

void ChartManager::timeAdvanced(double time) {
  auto handleEvent = [time, this](auto &&e) {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(e)>;

    if (e.time > time)
      return false;

    if constexpr (std::is_same_v<T, parser::XYSeriesAddValue>) {
      const auto &s = std::get<XYSeriesTie>(series[e.seriesId]);
      if (s.model.xAxis.boundMode == parser::ValueAxis::BoundMode::HighestValue) {
        updateRange(s.xAxis, e.x);
      }
      if (s.model.yAxis.boundMode == parser::ValueAxis::BoundMode::HighestValue) {
        updateRange(s.yAxis, e.y);
      }
      updateCollectionRanges(e.seriesId, e.x, e.y);
      s.qtSeries->append(e.x, e.y);
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
void ChartManager::enqueueEvents(const std::vector<parser::ChartEvent> &e) {
  events.insert(events.end(), e.begin(), e.end());
}

} // namespace visualization