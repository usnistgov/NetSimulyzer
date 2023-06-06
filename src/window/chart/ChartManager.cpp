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
#include <unordered_set>
#include <utility>

namespace netsimulyzer {

ChartManager::XYSeriesTie ChartManager::makeTie(const parser::XYSeries &model) {
  ChartManager::XYSeriesTie tie;
  tie.model = model;
  switch (model.labelMode) {
  case parser::XYSeries::LabelMode::Hidden:
    //    tie.qtSeries->setPointLabelsVisible(false);
    break;
  case parser::XYSeries::LabelMode::Shown:
    //    tie.qtSeries->setPointLabelsVisible(true);
    break;
  }

  // It seems there's some difficulty with this setting on macOS,
  // so disable it
#ifndef __APPLE__
  // Note, this will only work with line/spline/scatter plots
  // if we add more plot types, this will have to be disabled
  // See: https://doc.qt.io/qt-5/qabstractseries.html#useOpenGL-prop
//  tie.qtSeries->setUseOpenGL(true);
#endif

  tie.pen.setColor(QColor::fromRgb(model.color.red, model.color.green, model.color.blue));

  //  tie.qtSeries->setName(QString::fromStdString(model.legend));

  // X Axis
  tie.XRange.upper = model.xAxis.max;
  tie.XRange.lower = model.xAxis.min;

  //  if (tie.model.xAxis.scale == parser::ValueAxis::Scale::Linear)
  //    tie.xAxis = new QtCharts::QValueAxis(this);
  //  else
  //    tie.xAxis = new QtCharts::QLogValueAxis(this);

  //  tie.xAxis->setTitleText(QString::fromStdString(model.xAxis.name));
  //  tie.xAxis->setRange(model.xAxis.min, model.xAxis.max);

  // Y Axis
  tie.YRange.upper = model.yAxis.max;
  tie.YRange.lower = model.yAxis.min;
  //  if (tie.model.yAxis.scale == parser::ValueAxis::Scale::Linear)
  //    tie.yAxis = new QtCharts::QValueAxis(this);
  //  else
  //    tie.yAxis = new QtCharts::QLogValueAxis(this);
  //  tie.yAxis = new QtCharts::QValueAxis(this);
  //  tie.yAxis->setTitleText(QString::fromStdString(model.yAxis.name));
  //  tie.yAxis->setRange(model.yAxis.min, model.yAxis.max);

  switch (model.connection) {
  case parser::XYSeries::Connection::None: {
    tie.scatterStyle.setShape(QCPScatterStyle::ssDisc);
    tie.scatterStyle.setPen(tie.pen);

    // TODO: user setting for scatter shape size
  } break;
  case parser::XYSeries::Connection::Line:
    tie.scatterStyle.setShape(QCPScatterStyle::ssNone);
    break;
  case parser::XYSeries::Connection::Spline:
    //    tie.qtSeries = new QtCharts::QSplineSeries(this);
    break;
  }

  return tie;
}

ChartManager::SeriesCollectionTie ChartManager::makeTie(const parser::SeriesCollection &model) {
  ChartManager::SeriesCollectionTie tie;
  tie.model = model;

  // X Axis
  tie.XRange.upper = model.xAxis.max;
  tie.XRange.lower = model.xAxis.min;

  // Y Axis
  tie.YRange.upper = model.yAxis.max;
  tie.YRange.lower = model.yAxis.min;

  return tie;
}

ChartManager::CategoryValueTie ChartManager::makeTie(const parser::CategoryValueSeries &model) {
  CategoryValueTie tie;
  tie.model = model;

  tie.pen.setColor(QColor::fromRgb(model.color.red, model.color.green, model.color.blue));

  // X Axis (value)
  tie.XRange.upper = model.xAxis.max;
  tie.XRange.lower = model.xAxis.min;

  auto minId = tie.model.yAxis.values[0].id;
  auto maxId = tie.model.yAxis.values[0].id;
  // Y axis (categories)
  for (const auto &category : tie.model.yAxis.values) {
    tie.labelTicker->addTick(category.id, QString::fromStdString(category.name));
    minId = std::min(category.id, minId);
    maxId = std::max(category.id, maxId);
  }
  tie.YRange.lower = static_cast<double>(minId);
  tie.YRange.upper = static_cast<double>(maxId);

  return tie;
}

ChartManager::ChartManager(QWidget *parent) : QObject(parent) {
}

void ChartManager::reset() {
  dropdownElements.clear();
  events.clear();
  undoEvents.clear();

  // Clear the child widgets first
  // since they may be holding on to series
  for (auto chartWidget : chartWidgets) {
    chartWidget->reset();
  }

  series.clear();
}

void ChartManager::setChildrenSeries(const std::vector<DropdownValue> &values) {
  for (auto chartWidget : chartWidgets) {
    chartWidget->setSeries(values);
  }
}

std::vector<unsigned int> ChartManager::inCollections(unsigned int id) {
  std::vector<unsigned int> collections;

  for (const auto &[key, value] : series) {
    // We're only concerned about collections
    // so we can skip everything else
    if (!std::holds_alternative<SeriesCollectionTie>(value))
      continue;

    const auto &tieModel = std::get<SeriesCollectionTie>(value).model;
    if (std::find(tieModel.series.begin(), tieModel.series.end(), id) != tieModel.series.end())
      collections.emplace_back(tieModel.id);
  }

  return collections;
}

void ChartManager::clearSeries(const ChartWidget *except, unsigned int id) {
  for (auto widget : chartWidgets) {
    if (widget == except)
      continue;

    if (widget->getCurrentSeries() == id)
      widget->clearSelected();
  }
}

void ChartManager::updateRange(QCPRange &range, double point) {
  range.lower = std::min(range.lower, point);
  range.upper = std::max(range.upper, point);
}

void ChartManager::notifyDataChanged(const XYSeriesTie &tie) {
  for (const auto widget : chartWidgets) {
    if (widget->getCurrentSeries() == tie.model.id)
      widget->dataChanged(tie);
  }
}

void ChartManager::notifyDataChanged(const ChartManager::SeriesCollectionTie &tie) {
  for (const auto widget : chartWidgets) {
    if (widget->getCurrentSeries() == tie.model.id)
      widget->dataChanged(tie);
  }
}
void ChartManager::notifyDataChanged(const ChartManager::CategoryValueTie &tie) {
  for (const auto widget : chartWidgets) {
    if (widget->getCurrentSeries() == tie.model.id)
      widget->dataChanged(tie);
  }
}

void ChartManager::timeAdvanced(parser::nanoseconds time) {
  using BoundMode = parser::ValueAxis::BoundMode;
  std::unordered_set<uint32_t> changedSeries;

  auto handleEvent = [time, &changedSeries, this](auto &&e) {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(e)>;

    if (e.time > time)
      return false;

    if constexpr (std::is_same_v<T, parser::XYSeriesAddValue>) {
      auto &s = std::get<XYSeriesTie>(series[e.seriesId]);
      if (s.model.xAxis.boundMode == BoundMode::HighestValue) {
        updateRange(s.XRange, e.point.x);
      }
      if (s.model.yAxis.boundMode == BoundMode::HighestValue) {
        updateRange(s.YRange, e.point.y);
      }

      updateCollectionRanges(e.seriesId, e.point.x, e.point.y);

      const auto pointIndex = static_cast<double>(s.data->size());
      s.data->add({pointIndex, e.point.x, e.point.y});

      changedSeries.insert(e.seriesId);
      const auto collections = inCollections(e.seriesId);
      changedSeries.insert(collections.begin(), collections.end());

      undoEvents.emplace_back(undo::XYSeriesAddValue{e, pointIndex});
      events.pop_front();
      return true;
    }

    if constexpr (std::is_same_v<T, parser::XYSeriesAddValues>) {
      auto &s = std::get<XYSeriesTie>(series[e.seriesId]);

      std::pair<double, double> range;
      range.first = static_cast<double>(s.data->size());

      for (const auto &point : e.points) {
        if (s.model.xAxis.boundMode == BoundMode::HighestValue) {
          updateRange(s.XRange, point.x);
        }
        if (s.model.yAxis.boundMode == BoundMode::HighestValue) {
          updateRange(s.YRange, point.y);
        }

        updateCollectionRanges(e.seriesId, point.x, point.y);
        s.data->add({static_cast<double>(s.data->size()), point.x, point.y});
      }
      range.second = static_cast<double>(s.data->size());

      changedSeries.insert(e.seriesId);
      const auto collections = inCollections(e.seriesId);
      changedSeries.insert(collections.begin(), collections.end());

      undoEvents.emplace_back(undo::XYSeriesAddValues{e, range});
      events.pop_front();
      return true;
    }

    if constexpr (std::is_same_v<T, parser::XYSeriesClear>) {
      // Not const since we replace the data pointer
      auto &s = std::get<XYSeriesTie>(series[e.seriesId]);
      auto oldData = s.data;
      s.data = QSharedPointer<QCPCurveDataContainer>{new QCPCurveDataContainer{}};

      changedSeries.insert(e.seriesId);
      const auto collections = inCollections(e.seriesId);
      changedSeries.insert(collections.begin(), collections.end());

      undoEvents.emplace_back(undo::XYSeriesClear{e, oldData});
      events.pop_front();
      return true;
    }

    if constexpr (std::is_same_v<T, parser::CategorySeriesAddValue>) {
      // Not const since we change the lastUpdatedTime
      auto &s = std::get<CategoryValueTie>(series[e.seriesId]);
      if (s.model.xAxis.boundMode == BoundMode::HighestValue) {
        updateRange(s.XRange, e.value);
      }

      // Y axis on category charts is a fixed size

      s.lastUpdatedTime = time;
      const auto pointIndex = static_cast<double>(s.data->size());
      s.data->add({pointIndex, e.value, static_cast<double>(e.category)});
      updateCollectionRanges(e.seriesId, e.value, e.category);

      changedSeries.insert(e.seriesId);
      undoEvents.emplace_back(undo::CategorySeriesAddValue{e, pointIndex});
      events.pop_front();
      return true;
    }

    // Unhandled event type
    return false;
  };

  while (!events.empty() && std::visit(handleEvent, events.front())) {
    // Intentionally Blank
  }

  // Add "Fake Events" to keep the category value series moving
  // TODO: Maybe move to parse time
  for (auto &[key, s] : series) {
    // Only CategoryValueSeries may have auto-appended values
    if (!std::holds_alternative<CategoryValueTie>(s))
      continue;

    auto &value = std::get<CategoryValueTie>(s);
    if (!value.model.autoUpdate)
      continue;

    const auto &points = value.data;
    if (points->isEmpty())
      continue;

    if (time - value.lastUpdatedTime < value.model.autoUpdateInterval)
      continue;

    const auto lastValue = *(points->end() - 1);
    parser::CategorySeriesAddValue fakeEvent;
    fakeEvent.time = time;
    fakeEvent.value = lastValue.key + value.model.autoUpdateIncrement;
    fakeEvent.category = static_cast<unsigned int>(lastValue.value);
    fakeEvent.seriesId = key;

    if (value.model.xAxis.boundMode == BoundMode::HighestValue) {
      updateRange(value.XRange, fakeEvent.value);
    }

    // Y axis on category charts is a fixed size

    changedSeries.insert(value.model.id);

    const auto pointIndex = static_cast<double>(points->size());
    points->add({pointIndex, fakeEvent.value, static_cast<double>(fakeEvent.category)});

    updateCollectionRanges(fakeEvent.seriesId, fakeEvent.value, fakeEvent.category);
    undoEvents.emplace_back(undo::CategorySeriesAddValue{fakeEvent, pointIndex});

    value.lastUpdatedTime = time;
  }

  for (const auto changedSeriesId : changedSeries) {
    std::visit(
        [this](auto &&tie) {
          notifyDataChanged(tie);
        },
        series[changedSeriesId]);
  }
}

void ChartManager::timeRewound(parser::nanoseconds time) {
  std::unordered_set<uint32_t> changedSeries;

  auto handleUndoEvent = [time, &changedSeries, this](auto &&e) -> bool {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(e)>;

    // All events have a time
    // Make sure we don't handle one
    // Before it was originally applied
    if (time > e.event.time)
      return false;

    if constexpr (std::is_same_v<T, undo::XYSeriesAddValue>) {
      auto &s = std::get<XYSeriesTie>(series[e.event.seriesId]);

      s.data->remove(e.pointIndex);

      changedSeries.insert(e.event.seriesId);
      const auto collections = inCollections(e.event.seriesId);
      changedSeries.insert(collections.begin(), collections.end());

      events.emplace_front(e.event);
      return true;
    }

    if constexpr (std::is_same_v<T, undo::XYSeriesAddValues>) {
      auto &s = std::get<XYSeriesTie>(series[e.event.seriesId]);
      s.data->remove(e.tIndexRange.first, e.tIndexRange.second);

      changedSeries.insert(e.event.seriesId);
      const auto collections = inCollections(e.event.seriesId);
      changedSeries.insert(collections.begin(), collections.end());

      events.emplace_front(e.event);
      return true;
    }

    if constexpr (std::is_same_v<T, undo::XYSeriesClear>) {
      auto &s = std::get<XYSeriesTie>(series[e.event.seriesId]);
      s.data = e.oldData;

      changedSeries.insert(e.event.seriesId);
      const auto collections = inCollections(e.event.seriesId);
      changedSeries.insert(collections.begin(), collections.end());

      events.emplace_front(e.event);
      return true;
    }

    if constexpr (std::is_same_v<T, undo::CategorySeriesAddValue>) {
      auto &s = std::get<CategoryValueTie>(series[e.event.seriesId]);

      s.data->remove(e.pointIndex);

      changedSeries.insert(e.event.seriesId);
      events.emplace_front(e.event);
      return true;
    }

    return false;
  };

  while (!undoEvents.empty() && std::visit(handleUndoEvent, undoEvents.back())) {
    undoEvents.pop_back();
  }

  for (const auto changedSeriesId : changedSeries) {
    std::visit(
        [this](auto &&tie) {
          notifyDataChanged(tie);
        },
        series[changedSeriesId]);
  }
}

void ChartManager::spawnWidget(QMainWindow *parent) {
  auto newWidget = new ChartWidget{parent, *this, dropdownElements};
  parent->addDockWidget(Qt::RightDockWidgetArea, newWidget);

  chartWidgets.emplace_back(newWidget);
}

void ChartManager::clearWidgets() {
  for (auto widget : chartWidgets) {
    widget->close();
    widget->deleteLater();
  }

  chartWidgets.clear();
}

void ChartManager::widgetClosed(ChartWidget *widget) {
  // Remove the closed widget from our list
  chartWidgets.erase(std::remove(chartWidgets.begin(), chartWidgets.end(), widget), chartWidgets.end());
}

void ChartManager::updateCollectionRanges(unsigned int seriesId, double x, double y) {
  using BoundMode = parser::ValueAxis::BoundMode;

  for (auto &[key, value] : series) {
    // We're only concerned about collections
    // so we can skip everything else
    if (!std::holds_alternative<SeriesCollectionTie>(value))
      continue;

    // Not `const` as we may need to change ranges
    auto &tie = std::get<SeriesCollectionTie>(value);
    const auto &model = tie.model;

    // If the series isn't in the collection, then we can skip it
    if (std::find(model.series.begin(), model.series.end(), seriesId) == model.series.end())
      continue;
    if (model.xAxis.boundMode == BoundMode::HighestValue)
      updateRange(tie.XRange, x);

    if (model.yAxis.boundMode == BoundMode::HighestValue)
      updateRange(tie.YRange, y);
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

void ChartManager::timeChanged(parser::nanoseconds time, parser::nanoseconds increment) {
  if (increment > 0LL)
    timeAdvanced(time);
  else
    timeRewound(time);
}

void ChartManager::enqueueEvents(const std::vector<parser::ChartEvent> &e) {
  events.insert(events.end(), e.begin(), e.end());
}
void ChartManager::addSeries(const std::vector<parser::XYSeries> &xySeries,
                             const std::vector<parser::SeriesCollection> &collections,
                             const std::vector<parser::CategoryValueSeries> &categoryValueSeries) {

  for (const auto &collection : collections) {
    series.emplace(collection.id, makeTie(collection));
    dropdownElements.emplace_back(
        DropdownValue{QString::fromStdString(collection.name), SeriesType::Collection, collection.id});
  }

  for (const auto &xy : xySeries) {
    series.emplace(xy.id, makeTie(xy));

    if (xy.visible) {
      dropdownElements.emplace_back(DropdownValue{QString::fromStdString(xy.name), SeriesType::XY, xy.id});
    }
  }

  for (const auto &category : categoryValueSeries) {
    series.emplace(category.id, makeTie(category));

    if (category.visible) {
      dropdownElements.emplace_back(
          DropdownValue{QString::fromStdString(category.name), SeriesType::CategoryValue, category.id});
    }
  }

  setChildrenSeries(dropdownElements);
}
void ChartManager::setSortOrder(SettingsManager::ChartDropdownSortOrder value) {
  sortOrder = value;
  for (const auto widget : chartWidgets) {
    widget->setSortOrder(sortOrder);
  }
}

} // namespace netsimulyzer
