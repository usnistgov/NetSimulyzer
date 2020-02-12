#pragma once

#include "../event/model.h"
#include "../parser/file-parser.h"
#include "../parser/model.h"
#include "chart/ChartManager.h"
#include "osgWidget.h"
#include "ui_mainWindow.h"
#include <QLabel>
#include <QMainWindow>
#include <QMdiArea>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <deque>
#include <osg/Group>
#include <osg/ref_ptr>

namespace visualization {
class MainWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit MainWindow(const GlobalConfiguration &config, const std::deque<ChartEvent> &chartEvents,
                      const FileParser &parser, osg::ref_ptr<osg::Group> root = new osg::Group,
                      QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow() override;

private:
  ChartManager charts;
  Ui::MainWindow *ui;
  /**
   * Label inside the Status Bar. Used for 'Normal' Messages
   *
   * @see: https://doc.qt.io/qt-5/qstatusbar.html
   */
  QLabel statusLabel{"0ms", this};
  OSGWidget osg;

  void timeAdvanced(double time);
};
} // namespace visualization
