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

namespace visualization {

ChartManager::ChartManager(QWidget *parent) : QWidget(parent) {
  ui->setupUi(this);

  ui->comboBoxSeries->addItem("Select Series", 0u);
  ui->comboBoxLeftAxis->addItem("Select Axis", 0u);
  ui->comboBoxBottomAxis->addItem("Select Axis", 0u);

  chart.legend()->setVisible(true);
  chart.legend()->setAlignment(Qt::AlignBottom);

  ui->chartView->setChart(&chart);
  ui->chartView->setRenderHints(QPainter::Antialiasing);

  QObject::connect(ui->comboBoxBottomAxis, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &ChartManager::bottomAxisSelected);
  QObject::connect(ui->comboBoxLeftAxis, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &ChartManager::leftAxisSelected);
  QObject::connect(ui->comboBoxSeries, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &ChartManager::seriesSelected);
}

ChartManager::~ChartManager() {
  delete ui;
}

void ChartManager::bottomAxisSelected(int index) {
  showAxis(ui->comboBoxBottomAxis->itemData(index).toUInt(), Qt::AlignBottom);

  auto leftModel = qobject_cast<QStandardItemModel *>(ui->comboBoxLeftAxis->model());

  // Re-enable the option for the one we're switching away from
  if (previousLeftIndex)
    leftModel->item(previousLeftIndex.value())->setEnabled(true);

  leftModel->item(index)->setEnabled(false);
  previousLeftIndex = index;
}

void ChartManager::leftAxisSelected(int index) {
  showAxis(ui->comboBoxLeftAxis->itemData(index).toUInt(), Qt::AlignLeft);

  auto bottomModel = qobject_cast<QStandardItemModel *>(ui->comboBoxBottomAxis->model());

  // Re-enable the option for the one we're switching away from
  if (previousBottomIndex)
    bottomModel->item(previousBottomIndex.value())->setEnabled(true);

  bottomModel->item(index)->setEnabled(false);
  previousBottomIndex = index;
}

void ChartManager::seriesSelected(int index) {
  auto seriesId = ui->comboBoxSeries->itemData(index).toUInt();

  // TODO: Allow several series to be shown at once
  if (activeSeries != nullptr) {
    for (auto &activeAxis : activeAxes) {
      activeSeries->detachAxis(activeAxis.second);
    }

    chart.removeSeries(activeSeries);
    // Reclaim ownership of the series
    activeSeries->setParent(this);
  }

  // ID 0 is for the placeholder element
  if (seriesId == 0u) {
    activeSeries = nullptr;
    return;
  }

  showSeries(seriesId);
  activeSeries = series[seriesId].qtSeries;
}

void ChartManager::addValueAxis(const ValueAxis &model) {
  ChartManager::ValueAxisTie tie;
  tie.model = model;

  auto axis = new QtCharts::QValueAxis(this);
  axis->setTitleText(QString::fromStdString(model.name));
  axis->setRange(model.min, model.max);
  axis->setTickCount(model.ticks);
  axis->setMinorTickCount(model.minorTicks);
  tie.qtAxis = axis;
  // Alignment handled by the chart itself

  axes.insert({model.id, tie});
  ui->comboBoxBottomAxis->addItem(QString::fromStdString(model.name), model.id);
  ui->comboBoxLeftAxis->addItem(QString::fromStdString(model.name), model.id);
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

  tie.qtSeries->setName(QString::fromStdString(s.name));
  tie.qtSeries->setPointLabelsVisible(true);

  series.insert({s.id, tie});
  ui->comboBoxSeries->addItem(QString::fromStdString(s.name), s.id);
}

void ChartManager::showSeries(uint32_t seriesId) {
  const auto &seriesIterator = series.find(seriesId);
  if (seriesIterator == series.end())
    return;

  auto &seriesValue = seriesIterator->second;

  // Qt wants the series on the chart before the axes
  chart.addSeries(seriesValue.qtSeries);

  for (auto &activeAxis : activeAxes)
    seriesValue.qtSeries->attachAxis(activeAxis.second);
}

void ChartManager::hideSeries(uint32_t seriesId) {
  const auto &seriesIterator = series.find(seriesId);
  if (seriesIterator == series.end())
    return;

  auto &seriesValue = seriesIterator->second;
  for (auto &activeAxis : activeAxes)
    seriesValue.qtSeries->detachAxis(activeAxis.second);

  chart.removeSeries(seriesValue.qtSeries);
  // Reclaim ownership of the series
  seriesValue.qtSeries->setParent(this);
}

void ChartManager::showAxis(uint32_t axisId, Qt::AlignmentFlag align) {
  auto existingAxis = activeAxes.find(align);
  if (existingAxis != activeAxes.end()) {
    chart.removeAxis(existingAxis->second);
    if (activeSeries)
      activeSeries->detachAxis(existingAxis->second);

    // addAxis changes the chart to the owner, removeAxis unsets the chart as the owner
    // so we should retake the axis
    existingAxis->second->setParent(this);

    // Erase in the event we're removing an axis entirely from an alignment
    activeAxes.erase(existingAxis);
  }

  const auto &axisIterator = axes.find(axisId);
  if (axisIterator == axes.end())
    return;
  const auto &axis = axisIterator->second;

  activeAxes.insert_or_assign(align, axis.qtAxis);
  chart.addAxis(axis.qtAxis, align);

  if (activeSeries)
    activeSeries->attachAxis(axis.qtAxis);
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
      series[e.seriesId].qtSeries->append(e.x, e.y);
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
