#include "mainWindow.h"
#include "osgWidget.h"
#include <QDebug>
#include <QMdiSubWindow>
#include <QMenuBar>

namespace visualization {

MainWindow::MainWindow(const GlobalConfiguration &config, const std::deque<ChartEvent> &chartEvents,
                       const FileParser &parser, osg::ref_ptr<osg::Group> root, QWidget *parent, Qt::WindowFlags flags)
    : QMainWindow(parent, flags), charts(this), ui(new Ui::MainWindow), osg(config, root, this) {
  ui->setupUi(this);
  ui->horizontalLayout->addWidget(&osg);
  ui->horizontalLayout->addWidget(&charts);

  const auto &xySeries = parser.getXYSeries();
  for (const auto &series : xySeries) {
    charts.addSeries(series);
  }

  const auto &seriesCollections = parser.getSeriesCollections();
  for (const auto &series : seriesCollections) {
    charts.addSeries(series);
  }

  for (const auto &event : chartEvents) {
    charts.enqueueEvent(event);
  }

  QObject::connect(&osg, &OSGWidget::timeAdvanced, &charts, &ChartManager::timeAdvanced);
}

MainWindow::~MainWindow() {
  delete ui;
}

} // namespace visualization
