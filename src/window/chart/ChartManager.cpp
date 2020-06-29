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
#include "ChartWidget.h"
#include <QDockWidget>
#include <QGraphicsLayout>
#include <QMainWindow>
#include <QMessageBox>
#include <QString>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <algorithm>
#include <cstdlib>
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

ChartManager::ChartManager(QWidget *parent) : QObject(parent) {
}

void ChartManager::reset() {
  seriesInCollections.clear();
  events.clear();

  // Clear the child widgets first
  // since they may be holding on to series
  for (auto chartWidget : chartWidgets) {
    chartWidget->reset();
  }

  for (auto &iterator : series) {
    auto &value = iterator.second;
    if (std::holds_alternative<XYSeriesTie>(value)) {
      auto qtSeries = std::get<XYSeriesTie>(value).qtSeries;
      qtSeries->setParent(nullptr);
      qtSeries->deleteLater();
    } else if (std::holds_alternative<CategoryValueTie>(value)) {
      auto qtSeries = std::get<CategoryValueTie>(value).qtSeries;
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
  case parser::XYSeries::Connection::None: {
    auto scatterSeries = new QtCharts::QScatterSeries(this);

    // Hide the borders of points, as they cover up other points
    scatterSeries->setBorderColor(QColor(Qt::transparent));

    // Cut this down, as the default size (15 at time of writing) is quite large
    scatterSeries->setMarkerSize(5.0);
    tie.qtSeries = scatterSeries;
  } break;
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

  // Only add the series to the combobox if it is not part of a collection
  // TODO: Replace this behavior with an ns-3 attribute
  if (std::find(seriesInCollections.begin(), seriesInCollections.end(), s.id) == seriesInCollections.end()) {
    addSeriesToChildren(s.name, s.id);
  }
}

void ChartManager::addSeries(const parser::SeriesCollection &s) {
  ChartManager::SeriesCollectionTie tie;
  tie.model = s;
  seriesInCollections.insert(seriesInCollections.end(), s.series.begin(), s.series.end());

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
  addSeriesToChildren(s.name, s.id);
}

void ChartManager::addSeries(const parser::CategoryValueSeries &s) {
  CategoryValueTie tie;
  tie.model = s;
  tie.qtSeries = new QtCharts::QLineSeries(this);

  tie.qtSeries->setColor(QColor::fromRgb(s.red, s.green, s.blue, s.alpha));
  tie.qtSeries->setName(QString::fromStdString(s.name));

  // X Axis (values)
  if (tie.model.xAxis.scale == parser::ValueAxis::Scale::Linear)
    tie.xAxis = new QtCharts::QValueAxis(this);
  else
    tie.xAxis = new QtCharts::QLogValueAxis(this);
  tie.xAxis->setTitleText(QString::fromStdString(s.xAxis.name));
  tie.xAxis->setRange(s.xAxis.min, s.xAxis.max);

  // Y axis (categories)
  auto yAxis = new QtCharts::QCategoryAxis(this);
  const auto &categories = tie.model.yAxis.values;

  yAxis->setTitleText(QString::fromStdString(s.yAxis.name));
  // Just to be safe
  if (!categories.empty()) {
    // Give slight padding before/after the min/max values
    yAxis->setMin(static_cast<double>(categories.front().id) - 0.1);
    yAxis->setMax(static_cast<double>(categories.back().id) + 0.1);
  }
  for (const auto &category : categories)
    yAxis->append(QString::fromStdString(category.name), category.id);

  // Center the label within the range
  // since we use the ID as the end
  // of the range, rather than the center
  yAxis->setLabelsPosition(QtCharts::QCategoryAxis::AxisLabelsPositionOnValue);

  tie.yAxis = yAxis;

  series.insert({s.id, tie});
  addSeriesToChildren(s.name, s.id);
}

void ChartManager::addSeriesToChildren(const std::string &name, unsigned int id) {
  for (auto chartWidget : chartWidgets) {
    chartWidget->addSeries(name, id);
  }
}

void ChartManager::spawnWidget(QMainWindow *parent) {
  auto newWidget = new ChartWidget{parent, *this};
  parent->addDockWidget(Qt::RightDockWidgetArea, newWidget);

  // Add existing series to the dropdown
  for (const auto &[key, value] : series) {
    std::visit(
        [this, newWidget](auto &&tie) {
          // XYSeries is the only one which may be in a collection
          // so we conditionally add it
          if constexpr (std::is_same_v<std::decay_t<decltype(tie)>, XYSeriesTie>) {
            if (std::find(seriesInCollections.begin(), seriesInCollections.end(), tie.model.id) ==
                seriesInCollections.end()) {
              newWidget->addSeries(tie.model.name, tie.model.id);
            }
            return;
          }

          newWidget->addSeries(tie.model.name, tie.model.id);
        },
        value);
  }

  chartWidgets.emplace_back(newWidget);
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

ChartManager::TieVariant &ChartManager::getSeries(uint32_t seriesId) {
  const auto &seriesIterator = series.find(seriesId);
  if (seriesIterator == series.end()) {
    QMessageBox::critical(qobject_cast<QMainWindow *>(parent()), "Series not found",
                          "The selected series was not found");
    std::abort();
  }

  return seriesIterator->second;
}

void ChartManager::disableSeries(unsigned int id) {
  // Do nothing for the placeholder
  if (id == 0u)
    return;

  for (auto chartWidget : chartWidgets) {
    chartWidget->disableSeries(id);
  }
}

void ChartManager::enableSeries(unsigned int id) {
  // Do nothing for the placeholder
  if (id == 0u)
    return;

  for (auto chartWidget : chartWidgets) {
    chartWidget->enableSeries(id);
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

    if constexpr (std::is_same_v<T, parser::CategorySeriesAddValue>) {
      const auto &s = std::get<CategoryValueTie>(series[e.seriesId]);
      if (s.model.xAxis.boundMode == parser::ValueAxis::BoundMode::HighestValue) {
        updateRange(s.xAxis, e.value);
      }

      // Y axis on category charts is a fixed size

      s.qtSeries->append(e.value, e.category);
      updateCollectionRanges(e.seriesId, e.value, e.category);
      events.pop_front();
      return true;
    }

    // Unhandled event type
    return false;
  };

  while (!events.empty() && std::visit(handleEvent, events.front())) {
    // Intentionally Blank
  }

  if (events.empty())
    emit eventsComplete();
}
void ChartManager::enqueueEvents(const std::vector<parser::ChartEvent> &e) {
  events.insert(events.end(), e.begin(), e.end());
}

} // namespace visualization
