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
#include <QDebug>
#include <QGraphicsLayout>
#include <QStandardItemModel>
#include <QString>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QScatterSeries>
#include <QtCharts/QSplineSeries>
#include <utility>

namespace netsimulyzer {

void ChartWidget::seriesSelected(int index) {
  clearChart();
  auto selectedSeriesId = ui.comboBoxSeries->itemData(index).toUInt();

  manager.seriesSelected(this, selectedSeriesId);
  currentSeries = selectedSeriesId;

  // ID for the "Select Series" element
  if (selectedSeriesId == ChartManager::PlaceholderId) {
    setWindowTitle("Chart Widget");
    return;
  }

  auto &s = manager.getSeries(selectedSeriesId);

  if (std::holds_alternative<ChartManager::XYSeriesTie>(s))
    showSeries(std::get<ChartManager::XYSeriesTie>(s));
  else if (std::holds_alternative<ChartManager::SeriesCollectionTie>(s))
    showSeries(std::get<ChartManager::SeriesCollectionTie>(s));
  else if (std::holds_alternative<ChartManager::CategoryValueTie>(s))
    showSeries(std::get<ChartManager::CategoryValueTie>(s));
}

void ChartWidget::showSeries(const ChartManager::XYSeriesTie &tie) {
  ui.chartView->clearItems();

  // This is linked to the plot through the X Axis
  // and is deleted by `clearItems()`
  auto curve = new QCPCurve(ui.chartView->xAxis, ui.chartView->yAxis);

  // Color
  curve->setPen(tie.pen);

  const auto name = QString::fromStdString(tie.model.name);

  curve->setData(tie.data);
  curve->setName(name);
  setWindowTitle(name);

  ui.chartView->replot();
}

void ChartWidget::showSeries(const ChartManager::SeriesCollectionTie &tie) {
  // TODO: Implement
  assert(false);
  /*
  const auto name = QString::fromStdString(tie.model.name);
  chart.setTitle(name);
  setWindowTitle(name);

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
   */
}

void ChartWidget::showSeries(const ChartManager::CategoryValueTie &tie) {
  /*
  const auto name = QString::fromStdString(tie.model.name);
  chart.setTitle(name);
  setWindowTitle(name);

  // Qt wants the series on the chart before the axes
  chart.addSeries(tie.qtSeries);

  chart.addAxis(tie.xAxis, Qt::AlignBottom);
  chart.addAxis(tie.yAxis, Qt::AlignLeft);

  // The series may only be attached to an axis _after_ both have
  // been added to the chart...
  tie.qtSeries->attachAxis(tie.xAxis);
  tie.qtSeries->attachAxis(tie.yAxis);
   */
}

void ChartWidget::clearChart() {
  ui.chartView->clearItems();
  setWindowTitle("Chart Widget");
  /*
  // Remove old axes
  auto currentAxes = chart.axes();

  // Remove currently attached series
  for (const auto &activeSeries : chart.series()) {
    // Detach the axes from each series,
    // then they may be removed from the chart
    for (const auto &axis : currentAxes) {
      activeSeries->detachAxis(axis);
    }

    chart.removeSeries(activeSeries);
    // The chart claims ownership of the series when it's attached
    activeSeries->setParent(&manager);
  }

  for (const auto &axis : currentAxes) {
    chart.removeAxis(axis);

    // Reclaim ownership of the axis
    // Since the chart takes it when it is attached
    axis->setParent(&manager);
  }

  chart.setTitle("");
   */
}

void ChartWidget::closeEvent(QCloseEvent *event) {
  clearChart();
  manager.widgetClosed(this);

  QDockWidget::closeEvent(event);
}

ChartWidget::ChartWidget(QWidget *parent, ChartManager &manager, std::vector<ChartManager::DropdownValue> initialSeries)
    : QDockWidget(parent), manager(manager), dropdownValues(std::move(initialSeries)) {
  ui.setupUi(this);
  setWindowTitle("Chart Widget");
  /*

  chart.legend()->setVisible(true);
  chart.legend()->setAlignment(Qt::AlignBottom);

  // Remove padding
  chart.layout()->setContentsMargins(0, 0, 0, 0);
  chart.setBackgroundRoundness(0.0);

//  ui.chartView->setChart(&chart);
   */

  sortDropdown();
  populateDropdown();

  QObject::connect(ui.comboBoxSeries, qOverload<int>(&QComboBox::currentIndexChanged), this,
                   &ChartWidget::seriesSelected);

  setFloating(false);
  setVisible(true);
}

void ChartWidget::addSeries(ChartManager::DropdownValue dropdownValue) {
  dropdownValues.emplace_back(dropdownValue);
}

void ChartWidget::setSeries(std::vector<ChartManager::DropdownValue> values) {
  dropdownValues = std::move(values);
  sortDropdown();
  populateDropdown();
}

void ChartWidget::sortDropdown() {
  using SortOrder = SettingsManager::ChartDropdownSortOrder;
  using DropdownValue = ChartManager::DropdownValue;

  switch (sortOrder) {
  case SortOrder::Alphabetical:
    std::sort(dropdownValues.begin(), dropdownValues.end(), [](const DropdownValue &left, const DropdownValue &right) {
      return QString::localeAwareCompare(left.name, right.name) < 0;
    });
    break;
  case SortOrder::Type:
    // Sort by type
    std::sort(dropdownValues.begin(), dropdownValues.end(), [](const auto &left, const auto &right) -> bool {
      return static_cast<int>(left.type) < static_cast<int>(right.type);
    });

    // Sort Alphabetically within types
    std::sort(dropdownValues.begin(), dropdownValues.end(), [](const auto &left, const auto &right) -> bool {
      // Do not reorder along type boundaries
      // since we know the elements are in type order
      // should hold the boundaries in place
      if (left.type != right.type) {
        return false;
      }
      return QString::localeAwareCompare(left.name, right.name) < 0;
    });

    break;
  case SortOrder::Id:
    std::sort(dropdownValues.begin(), dropdownValues.end(), [](const auto &left, const auto &right) -> bool {
      return left.id < right.id;
    });
    break;
  case SortOrder::None:
    // Intentionally Blank
    break;
  default:
    std::cerr << "Unrecognised SortOrder: " << static_cast<int>(sortOrder) << '\n';
    std::abort();
    break;
  }
}

void ChartWidget::populateDropdown() {
  using SortOrder = SettingsManager::ChartDropdownSortOrder;

  const auto previousSeries = currentSeries;
  ui.comboBoxSeries->clear();
  ui.comboBoxSeries->addItem("Select Series", ChartManager::PlaceholderId);

  // We need the item model to set flags
  auto model = qobject_cast<QStandardItemModel *>(ui.comboBoxSeries->model());

  // Starter for Type sort order labels
  ChartManager::SeriesType lastType;
  if (sortOrder == SortOrder::Type && !dropdownValues.empty()) {
    lastType = dropdownValues.begin()->type;
    switch (lastType) {
    case ChartManager::SeriesType::XY:
      ui.comboBoxSeries->addItem("XYSeries");
      break;
    case ChartManager::SeriesType::CategoryValue:
      ui.comboBoxSeries->addItem("CategoryValueSeries");
      break;
    case ChartManager::SeriesType::Collection:
      ui.comboBoxSeries->addItem("Collections");
      break;
    }

    auto item = model->item(ui.comboBoxSeries->count() - 1);
    item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
  }

  for (const auto &dropDownValue : dropdownValues) {
    // If we're sorting by type, add in labels for each type we encounter
    if (sortOrder == SortOrder::Type && lastType != dropDownValue.type) {
      switch (dropDownValue.type) {
      case ChartManager::SeriesType::XY:
        ui.comboBoxSeries->addItem("XYSeries");
        break;
      case ChartManager::SeriesType::CategoryValue:
        ui.comboBoxSeries->addItem("CategoryValueSeries");
        break;
      case ChartManager::SeriesType::Collection:
        ui.comboBoxSeries->addItem("Collections");
        break;
      }
      auto item = model->item(ui.comboBoxSeries->count() - 1);
      item->setFlags(item->flags() & ~Qt::ItemIsEnabled);
      ui.comboBoxSeries->show();
    }
    lastType = dropDownValue.type;
    ui.comboBoxSeries->addItem(dropDownValue.name, dropDownValue.id);
  }
  ui.comboBoxSeries->setCurrentIndex(ui.comboBoxSeries->findData(previousSeries));
}

void ChartWidget::reset() {
  clearChart();
  dropdownValues.clear();
  populateDropdown();
}

void ChartWidget::setSortOrder(SettingsManager::ChartDropdownSortOrder value) {
  sortOrder = value;
  sortDropdown();
  populateDropdown();
}

void ChartWidget::dataChanged(const ChartManager::XYSeriesTie &tie) const {
  using BoundMode = parser::ValueAxis::BoundMode;

  if (tie.model.xAxis.boundMode == BoundMode::HighestValue && tie.XRange != ui.chartView->xAxis->range()) {
    ui.chartView->xAxis->setRange(tie.XRange);
  }

  if (tie.model.yAxis.boundMode == BoundMode::HighestValue && tie.YRange != ui.chartView->yAxis->range()) {
    ui.chartView->yAxis->setRange(tie.YRange);
  }

  ui.chartView->replot();
}

void ChartWidget::clearSelected() {
  clearChart();
  ui.comboBoxSeries->setCurrentIndex(0);
}

unsigned int ChartWidget::getCurrentSeries() const {
  return currentSeries;
}

} // namespace netsimulyzer
