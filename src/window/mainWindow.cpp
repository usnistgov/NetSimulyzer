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

#include "mainWindow.h"
#include "../group/building/Building.h"
#include "../group/decoration/Decoration.h"
#include "../group/node/Node.h"
#include "LoadWorker.h"
#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QObject>
#include <cstdlib>
#include <deque>
#include <file-parser.h>
#include <unordered_map>
#include <variant>

namespace visualization {

MainWindow::MainWindow(QWidget *parent, Qt::WindowFlags flags) : QMainWindow(parent, flags), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  setCentralWidget(&render);

  charts = new ChartManager{ui->chartDock};
  ui->chartDock->setWidget(charts);

  nodeWidget = new NodeWidget{ui->nodesDock};
  ui->nodesDock->setWidget(nodeWidget);

  logWidget = new ScenarioLogWidget{ui->logDock};
  ui->logDock->setWidget(logWidget);

  auto state = settings.get<QByteArray>(SettingsManager::Key::MainWindowState);
  if (state)
    restoreState(*state, stateVersion);

  loadWorker.moveToThread(&loadThread);
  QObject::connect(this, &MainWindow::startLoading, &loadWorker, &LoadWorker::load);
  QObject::connect(&loadWorker, &LoadWorker::fileLoaded, this, &MainWindow::finishLoading);
  loadThread.start();

  ui->menuWidget->addAction(ui->nodesDock->toggleViewAction());
  ui->menuWidget->addAction(ui->chartDock->toggleViewAction());
  ui->menuWidget->addAction(ui->logDock->toggleViewAction());

  // For somewhat permanent messages (a message with no timeout)
  // We need to use a widget in the status bar.
  // Note: This message can still be temporarily overwritten,
  // should we choose to do so
  ui->statusbar->insertWidget(0, &statusLabel);

  QObject::connect(&render, &RenderWidget::timeAdvanced, charts, &ChartManager::timeAdvanced);
  QObject::connect(&render, &RenderWidget::timeAdvanced, logWidget, &ScenarioLogWidget::timeAdvanced);
  QObject::connect(&render, &RenderWidget::timeAdvanced, this, &MainWindow::timeAdvanced);
  QObject::connect(&render, &RenderWidget::pauseToggled, [this](bool paused) {
    // clears any temporary messages when playback is started/resumed
    // so the time is visible
    if (!paused)
      ui->statusbar->clearMessage();
  });

  QObject::connect(&render, &RenderWidget::eventsComplete, [this]() {
    renderEventsComplete = true;
    checkPause();
  });

  QObject::connect(logWidget, &ScenarioLogWidget::eventsComplete, [this]() {
    logEventsComplete = true;
    checkPause();
  });

  QObject::connect(charts, &ChartManager::eventsComplete, [this]() {
    chartEventsComplete = true;
    checkPause();
  });

  QObject::connect(nodeWidget, &NodeWidget::nodeSelected, &render, &RenderWidget::focusNode);

  QObject::connect(ui->actionLoad, &QAction::triggered, this, &MainWindow::load);

  QObject::connect(ui->actionCameraSettings, &QAction::triggered,
                   [this]() { render.showCameraConfigurationDialogue(); });

  QObject::connect(ui->actionResetCameraPosition, &QAction::triggered, &render, &RenderWidget::resetCamera);
}

MainWindow::~MainWindow() {
  delete ui;
  loadThread.quit();
  // Make sure the thread has time to close before trying to destroy it
  loadThread.wait();
}
void MainWindow::timeAdvanced(double time) {
  auto convertedTime = static_cast<long>(time);

  // combine / and %
  auto result = std::div(convertedTime, 1000L);
  auto milliseconds = result.rem;

  result = std::div(result.quot, 60L);
  auto seconds = result.rem;

  result = std::div(result.quot, 60L);
  auto minutes = result.rem;

  // Dump the rest in as hours
  auto hours = result.quot;

  QString label;
  if (convertedTime > 3'600'000) /* 1 Hour in ms */ {
    label = QString{"%1:%2:%3.%4"}
                .arg(hours)
                .arg(minutes, 2, 10, QChar{'0'})
                .arg(seconds, 2, 10, QChar{'0'})
                .arg(milliseconds, 3, 10, QChar{'0'});
  } else if (convertedTime > 60'000L) /* 1 minute in ms */ {
    label = QString{"%1:%2.%3"}
                .arg(minutes, 2, 10, QChar{'0'})
                .arg(seconds, 2, 10, QChar{'0'})
                .arg(milliseconds, 3, 10, QChar{'0'});
  } else if (convertedTime > 1000L) {
    label = QString{"%1.%2"}.arg(seconds, 2, 10, QChar{'0'}).arg(milliseconds, 3, 10, QChar{'0'});
  } else {
    label.append('.' + QString::number(milliseconds));
  }

  statusLabel.setText(label);
}

void MainWindow::load() {
  auto fileName =
#ifdef __APPLE__
      QFileDialog::getOpenFileName(this, "Open File", ".", "JSON Files (*.json)");
#else
      QFileDialog::getOpenFileName(this, "Open File", ".", "JSON Files (*.json)", nullptr,
                                   QFileDialog::DontUseNativeDialog);
#endif

  if (fileName.isEmpty())
    return;
  if (loading) {
    ui->statusbar->showMessage("Already loading scenario!", 10000);
  }
  loading = true;
  ui->actionLoad->setEnabled(false);
  statusLabel.setText("Loading scenario: " + fileName);
  render.reset();
  nodeWidget->reset();
  emit startLoading(fileName);
}

void MainWindow::finishLoading(const QString &fileName) {
  auto parser = loadWorker.getParser();
  render.setConfiguration(parser.getConfiguration());

  // Nodes, Buildings, Decorations
  const auto &nodes = parser.getNodes();
  render.add(parser.getAreas(), parser.getBuildings(), parser.getDecorations(), parser.getNodes());

  for (const auto &node : nodes) {
    nodeWidget->addNode(node);
  }

  // Charts
  charts->reset();
  const auto &xySeries = parser.getXYSeries();
  for (const auto &series : xySeries) {
    charts->addSeries(series);
  }

  const auto &seriesCollections = parser.getSeriesCollections();
  for (const auto &series : seriesCollections) {
    charts->addSeries(series);
  }

  const auto &categoryValueSeries = parser.getCategoryValueSeries();
  for (const auto &series : categoryValueSeries) {
    charts->addSeries(series);
  }

  // Log Streams
  logWidget->reset();
  const auto &logStreams = parser.getLogStreams();
  for (const auto &logStream : logStreams) {
    logWidget->addStream(logStream);
  }

  // Events
  const auto &sceneEvents = parser.getSceneEvents();
  renderEventsComplete = sceneEvents.empty();
  render.enqueueEvents(sceneEvents);

  const auto &chartEvents = parser.getChartsEvents();
  chartEventsComplete = chartEvents.empty();
  charts->enqueueEvents(chartEvents);

  const auto &logEvents = parser.getLogEvents();
  logEventsComplete = logEvents.empty();
  logWidget->enqueueEvents(logEvents);
  checkPause();

  ui->statusbar->showMessage("Successfully loaded scenario: " + fileName, 10000);
  statusLabel.setText("Ready");
  loading = false;
  ui->actionLoad->setEnabled(true);
}

void MainWindow::checkPause() {
  if (chartEventsComplete && logEventsComplete && renderEventsComplete) {
    render.pause();
    render.allowPauseToggle(false);
  } else
    render.allowPauseToggle(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings.set(SettingsManager::Key::MainWindowState, saveState(stateVersion));
  QMainWindow::closeEvent(event);
}

} // namespace visualization
