#pragma once

#include "chart/ChartManager.h"
#include "osgWidget.h"
#include "ui_mainWindow.h"
#include <QDockWidget>
#include <QLabel>
#include <QMainWindow>
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
  explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow() override;

private:
  ChartManager *charts;
  Ui::MainWindow *ui;
  /**
   * Label inside the Status Bar. Used for 'Normal' Messages
   *
   * @see: https://doc.qt.io/qt-5/qstatusbar.html
   */
  QLabel statusLabel{"0ms", this};
  OSGWidget osg{this};

  void timeAdvanced(double time);
  void load();
  void toggleCharts();
};
} // namespace visualization
