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

#pragma once
#include "../../event/model.h"
#include "../../parser/model.h"
#include "ui_chartManager.h"
#include <QComboBox>
#include <QFrame>
#include <QGraphicsItem>
#include <QLayout>
#include <QObject>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <cstdint>
#include <deque>
#include <unordered_map>
#include <optional>

namespace visualization {
class ChartManager : public QWidget {
  Q_OBJECT

  struct XYSeriesTie {
    XYSeries model;
    QtCharts::QXYSeries *qtSeries;
  };

  std::deque<ChartEvent> events;
  std::unordered_map<uint32_t, QtCharts::QAbstractAxis*> axes;
  std::unordered_map<unsigned int, QtCharts::QAbstractAxis*> activeAxes;

  std::unordered_map<uint32_t, XYSeriesTie> series;

  QtCharts::QChart chart;

  Ui::ChartManager *ui = new Ui::ChartManager;

  // Used to re-enable previous axes when they're removed from the graph
  std::optional<int> previousLeftIndex;
  std::optional<int> previousBottomIndex;
  QAbstractSeries *activeSeries = nullptr;

  void bottomAxisSelected(int index);
  void leftAxisSelected(int index);
  void seriesSelected(int index);
public:
  explicit ChartManager(QWidget *parent);
  ~ChartManager() override;
  void addAxis(const ValueAxis &model);
  void addAxis(const LogarithmicAxis &model);
  void addSeries(const XYSeries &s);

  void showSeries(uint32_t seriesId);
  void hideSeries(uint32_t seriesId);
  void showAxis(uint32_t axisId, Qt::AlignmentFlag align);

  void timeAdvanced(double time);
  void enqueueEvent(const ChartEvent &e);
};

} // namespace visualization
