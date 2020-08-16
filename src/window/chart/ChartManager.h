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
#include <QString>
#include <QtCharts/QAbstractAxis>
#include <QtCharts/QCategoryAxis>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cstdint>
#include <deque>
#include <model.h>
#include <optional>
#include <unordered_map>
#include <variant>

namespace visualization {

class ChartWidget;

class ChartManager : public QObject {
  Q_OBJECT

public:
  enum class SortOrder { Alphabetical, Type, Id, None };
  enum class SeriesType : int { XY, CategoryValue, Collection };

  struct SeriesCollectionTie {
    parser::SeriesCollection model;
    QtCharts::QAbstractAxis *xAxis;
    QtCharts::QAbstractAxis *yAxis;
  };

  struct XYSeriesTie {
    parser::XYSeries model;
    QtCharts::QXYSeries *qtSeries;
    QtCharts::QAbstractAxis *xAxis;
    QtCharts::QAbstractAxis *yAxis;
  };

  struct CategoryValueTie {
    parser::CategoryValueSeries model;
    QtCharts::QXYSeries *qtSeries;
    QtCharts::QAbstractAxis *xAxis;
    QtCharts::QCategoryAxis *yAxis;
  };

  struct DropdownValue {
    QString name;
    SeriesType type;
    unsigned int id;
  };

  using TieVariant = std::variant<SeriesCollectionTie, XYSeriesTie, CategoryValueTie>;

private:
  std::deque<parser::ChartEvent> events;
  std::deque<undo::ChartUndoEvent> undoEvents;

  std::unordered_map<uint32_t, TieVariant> series;
  std::vector<unsigned int> seriesInCollections;
  SortOrder sortOrder{SortOrder::Type};
  std::vector<DropdownValue> dropdownElements;
  std::vector<ChartWidget *> chartWidgets;

  XYSeriesTie makeTie(const parser::XYSeries &model);
  SeriesCollectionTie makeTie(const parser::SeriesCollection &model);
  CategoryValueTie makeTie(const parser::CategoryValueSeries &model);
  void updateCollectionRanges(uint32_t seriesId, double x, double y);
  void addSeriesToChildren(const DropdownValue &value);

public:
  explicit ChartManager(QWidget *parent);

  /**
   * Clear all series and events
   */
  void reset();
  void spawnWidget(QMainWindow *parent);
  void addSeries(const std::vector<parser::XYSeries> &xySeries,
                 const std::vector<parser::SeriesCollection> &collections,
                 const std::vector<parser::CategoryValueSeries> &categoryValueSeries);
  TieVariant &getSeries(uint32_t seriesId);
  void disableSeries(unsigned int id);
  void enableSeries(unsigned int id);
  void timeAdvanced(double time);
  void timeRewound(double time);
  void enqueueEvents(const std::vector<parser::ChartEvent> &e);
};

} // namespace visualization
