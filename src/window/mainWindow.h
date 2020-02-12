#pragma once

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
  explicit MainWindow(QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
  ~MainWindow() override;

private:
  ChartManager charts{this};
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
};
} // namespace visualization
