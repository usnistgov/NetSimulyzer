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
#include "src/util/undo-events.h"
#include <QComboBox>
#include <QFrame>
#include <QGraphicsItem>
#include <QLayout>
#include <QMainWindow>
#include <QObject>
#include <QSharedPointer>
#include <QString>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cstdint>
#include <deque>
#include <lib/QCustomPlot/qcustomplot.h>
#include <model.h>
#include <optional>
#include <src/settings/SettingsManager.h>
#include <unordered_map>
#include <variant>

namespace netsimulyzer {

class ChartWidget;

class ChartManager : public QObject {
  Q_OBJECT

public:
  enum class SeriesType : int { XY, CategoryValue, Collection };

  struct SeriesCollectionTie {
    parser::SeriesCollection model;
    QCPRange XRange;
    QCPRange YRange;
  };

  struct XYSeriesTie {
    parser::XYSeries model;
    QPen pen;
    QCPScatterStyle scatterStyle;
    QSharedPointer<QCPCurveDataContainer> data{new QCPCurveDataContainer{}};
    QCPRange XRange;
    QCPRange YRange;
  };

  struct CategoryValueTie {
    parser::CategoryValueSeries model;
    QSharedPointer<QCPAxisTickerText> labelTicker{new QCPAxisTickerText{}};

    QPen pen;
    QSharedPointer<QCPCurveDataContainer> data{new QCPCurveDataContainer{}};
    QCPRange XRange;
    QCPRange YRange; // Fixed range containing the category IDs
    parser::nanoseconds lastUpdatedTime;
  };

  struct DropdownValue {
    QString name;
    SeriesType type;
    unsigned int id;
  };

  using TieVariant = std::variant<SeriesCollectionTie, XYSeriesTie, CategoryValueTie>;

  const static unsigned int PlaceholderId{0u};

private:
  SettingsManager settings;
  std::deque<parser::ChartEvent> events;
  std::deque<undo::ChartUndoEvent> undoEvents;

  std::unordered_map<uint32_t, TieVariant> series;
  SettingsManager::ChartDropdownSortOrder sortOrder{
      settings.get<SettingsManager::ChartDropdownSortOrder>(SettingsManager::Key::ChartDropdownSortOrder).value()};
  std::vector<DropdownValue> dropdownElements;
  std::vector<ChartWidget *> chartWidgets;

  XYSeriesTie makeTie(const parser::XYSeries &model);
  SeriesCollectionTie makeTie(const parser::SeriesCollection &model);
  CategoryValueTie makeTie(const parser::CategoryValueSeries &model);
  void updateCollectionRanges(unsigned int seriesId, double x, double y);
  void setChildrenSeries(const std::vector<DropdownValue> &values);

  /**
   * Finds all the collections the series
   * identified by `id` belongs to
   *
   * @param id
   * The ID of the series to search collections for
   *
   * @return
   * The IDs of all the collections `id` is in
   */
  std::vector<unsigned int> inCollections(unsigned int id);

  /**
   * Clear the series identified by `id` from
   * all widgets except for `except`
   *
   * @param except
   * The widget to ignore when clearing selections
   *
   * @param id
   * The ID of the series to clear selections of
   */
  void clearSeries(const ChartWidget *except, unsigned int id);

  void updateRange(QCPRange &range, double point);

  void notifyDataChanged(const XYSeriesTie &tie);
  void notifyDataChanged(const SeriesCollectionTie &tie);
  void notifyDataChanged(const CategoryValueTie &tie);

  void timeAdvanced(parser::nanoseconds time);
  void timeRewound(parser::nanoseconds time);

public:
  explicit ChartManager(QWidget *parent);

  /**
   * Clear all series and events
   */
  void reset();
  void spawnWidget(QMainWindow *parent);

  /**
   * Remove all child `ChartWidget`s
   */
  void clearWidgets();

  /**
   * Signals a child widget has been closed.
   * Removes all references to `widget`
   *
   * @param widget
   * The child widget that is closing
   */
  void widgetClosed(ChartWidget *widget);
  void addSeries(const std::vector<parser::XYSeries> &xySeries,
                 const std::vector<parser::SeriesCollection> &collections,
                 const std::vector<parser::CategoryValueSeries> &categoryValueSeries);
  TieVariant &getSeries(uint32_t seriesId);

  void timeChanged(parser::nanoseconds time, parser::nanoseconds increment);
  void enqueueEvents(const std::vector<parser::ChartEvent> &e);
  void setSortOrder(SettingsManager::ChartDropdownSortOrder value);
};

} // namespace netsimulyzer
