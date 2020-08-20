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

#include "ChartWidget.h"
#include <QConstOverload>
#include <QGraphicsLayout>
#include <QStandardItemModel>
#include <QString>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>

namespace visualization {

void ChartWidget::seriesSelected(int index) {
  clearChart();
  auto selectedSeriesId = ui.comboBoxSeries->itemData(index).toUInt();

  manager.seriesSelected(this, selectedSeriesId);
  currentSeries = selectedSeriesId;

  // ID for the "Select Series" element
  if (selectedSeriesId == ChartManager::PlaceholderId)
    return;

  auto &s = manager.getSeries(selectedSeriesId);

  if (std::holds_alternative<ChartManager::XYSeriesTie>(s))
    showSeries(std::get<ChartManager::XYSeriesTie>(s));
  else if (std::holds_alternative<ChartManager::SeriesCollectionTie>(s))
    showSeries(std::get<ChartManager::SeriesCollectionTie>(s));
  else if (std::holds_alternative<ChartManager::CategoryValueTie>(s))
    showSeries(std::get<ChartManager::CategoryValueTie>(s));
}

void ChartWidget::showSeries(const ChartManager::XYSeriesTie &tie) {
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

void ChartWidget::showSeries(const ChartManager::SeriesCollectionTie &tie) {
  chart.setTitle(QString::fromStdString(tie.model.name));

  for (auto seriesId : tie.model.series) {
    const auto &seriesVariant = manager.getSeries(seriesId);

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

void ChartWidget::showSeries(const ChartManager::CategoryValueTie &tie) {
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

void ChartWidget::clearChart() {
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
    currentSeries->setParent(&manager);
  }

  for (const auto &axis : currentAxes) {
    chart.removeAxis(axis);

    // Reclaim ownership of the axis
    // Since the chart takes it when it is attached
    axis->setParent(&manager);
  }

  chart.setTitle("");
}

void ChartWidget::closeEvent(QCloseEvent *event) {
  clearChart();
  manager.widgetClosed(this);

  QDockWidget::closeEvent(event);
}

ChartWidget::ChartWidget(QWidget *parent, ChartManager &manager,
                         const std::vector<ChartManager::DropdownValue> &initialSeries)
    : QDockWidget(parent), manager(manager) {
  ui.setupUi(this);
  ui.comboBoxSeries->addItem("Select Series", ChartManager::PlaceholderId);

  chart.legend()->setVisible(true);
  chart.legend()->setAlignment(Qt::AlignBottom);

  // Remove padding
  chart.layout()->setContentsMargins(0, 0, 0, 0);
  chart.setBackgroundRoundness(0.0);

  ui.chartView->setChart(&chart);

  // Should already be in proper order
  for (const auto &series : initialSeries) {
    addSeries(series.name, series.id);
  }

  QObject::connect(ui.comboBoxSeries, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &ChartWidget::seriesSelected);

  setFloating(false);
  setVisible(true);
}

void ChartWidget::addSeries(const QString &name, unsigned int id) {
  ui.comboBoxSeries->addItem(name, id);
}

void ChartWidget::reset() {
  clearChart();
  ui.comboBoxSeries->clear();
  ui.comboBoxSeries->addItem("Select Series", 0u);
}

void ChartWidget::clearSelected() {
  clearChart();
  ui.comboBoxSeries->setCurrentIndex(0);
}

unsigned int ChartWidget::getCurrentSeries() const {
  return currentSeries;
}

} // namespace visualization
