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

#pragma once

#include "ChartManager.h"
#include "ui_ChartWidget.h"
#include <QDockWidget>
#include <QString>
#include <QWidget>
#include <src/settings/SettingsManager.h>
#include <vector>

namespace netsimulyzer {

class ChartWidget : public QDockWidget {
  Q_OBJECT

  ChartManager &manager;
  SettingsManager settings{};
  Ui::ChartWidget ui{};
  unsigned int currentSeries{ChartManager::PlaceholderId};
  std::vector<ChartManager::DropdownValue> dropdownValues;
  SettingsManager::ChartDropdownSortOrder sortOrder =
      settings.get<SettingsManager::ChartDropdownSortOrder>(SettingsManager::Key::ChartDropdownSortOrder).value();

  void seriesSelected(int index);
  void showSeries(const ChartManager::XYSeriesTie &tie);
  void showSeries(const ChartManager::SeriesCollectionTie &tie);
  void showSeries(const ChartManager::CategoryValueTie &tie);

  /**
   * Remove all axes & series from the chart
   */
  void clearChart();

protected:
  void closeEvent(QCloseEvent *event) override;

public:
  ChartWidget(QWidget *parent, ChartManager &manager, std::vector<ChartManager::DropdownValue> initialSeries);
  void addSeries(ChartManager::DropdownValue dropdownValue);
  void setSeries(std::vector<ChartManager::DropdownValue> values);
  void sortDropdown();
  void populateDropdown();
  void reset();
  void setSortOrder(SettingsManager::ChartDropdownSortOrder value);

  void dataChanged(const ChartManager::XYSeriesTie &tie) const;

  /**
   * Unselects the current series
   * & resets the chart
   */
  void clearSelected();

  /**
   * Gets the ID of the currently selected series.
   * 0u is the ID of the placeholder item
   *
   * @return
   * The ID of the currently selected series,
   * or 0u in no series is selected
   */
  [[nodiscard]] unsigned int getCurrentSeries() const;
};

} // namespace netsimulyzer
