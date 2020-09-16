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

#include "MainWindow.h"
#include "LoadWorker.h"
#include "about/AboutDialog.h"
#include <QAction>
#include <QDebug>
#include <QDockWidget>
#include <QFileDialog>
#include <QObject>
#include <cstdlib>
#include <iostream>
#include <parser/file-parser.h>
#include <parser/model.h>
#include <project.h>

namespace visualization {

MainWindow::MainWindow() : QMainWindow() {
  ui.setupUi(this);
  setWindowTitle(VISUALIZER_APPLICATION_NAME);
  setCentralWidget(&render);

  ui.nodesDock->setWidget(&nodeWidget);
  ui.logDock->setWidget(&logWidget);

  auto state = settings.get<QByteArray>(SettingsManager::Key::MainWindowState);
  if (state)
    restoreState(*state, stateVersion);

  loadWorker.moveToThread(&loadThread);
  QObject::connect(this, &MainWindow::startLoading, &loadWorker, &LoadWorker::load);
  QObject::connect(&loadWorker, &LoadWorker::fileLoaded, this, &MainWindow::finishLoading);
  loadThread.start();

  ui.menuWindow->addAction(ui.nodesDock->toggleViewAction());
  ui.menuWindow->addAction(ui.logDock->toggleViewAction());

  // For somewhat permanent messages (a message with no timeout)
  // We need to use a widget in the status bar.
  // Note: This message can still be temporarily overwritten,
  // should we choose to do so
  ui.statusbar->insertWidget(0, &statusLabel);

  QObject::connect(&render, &SceneWidget::timeAdvanced, &charts, &ChartManager::timeAdvanced);
  QObject::connect(&render, &SceneWidget::timeRewound, &charts, &ChartManager::timeRewound);

  QObject::connect(&render, &SceneWidget::timeAdvanced, &logWidget, &ScenarioLogWidget::timeAdvanced);
  QObject::connect(&render, &SceneWidget::timeRewound, &logWidget, &ScenarioLogWidget::timeRewound);

  QObject::connect(&render, &SceneWidget::timeAdvanced, this, &MainWindow::timeChanged);
  QObject::connect(&render, &SceneWidget::timeRewound, this, &MainWindow::timeChanged);

  QObject::connect(&render, &SceneWidget::pauseToggled, [this](bool paused) {
    // clears any temporary messages when playback is started/resumed
    // so the time is visible
    if (!paused)
      ui.statusbar->clearMessage();
  });

  QObject::connect(&nodeWidget, &NodeWidget::nodeSelected, &render, &SceneWidget::focusNode);

  QObject::connect(ui.actionLoad, &QAction::triggered, this, &MainWindow::load);

  QObject::connect(ui.actionSettings, &QAction::triggered, [this]() {
    settingsDialog.show();
  });

  auto &camera = render.getCamera();
  QObject::connect(&settingsDialog, &SettingsDialog::moveSpeedChanged, [&camera](float value) {
    camera.setMoveSpeed(value);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::keyboardTurnSpeedChanged, [&camera](float value) {
    camera.setTurnSpeed(value);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::mouseTurnSpeedChanged, [&camera](float value) {
    camera.setMouseTurnSpeed(value);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::fieldOfViewChanged, [this, &camera](float value) {
    camera.setFieldOfView(value);
    render.updatePerspective();
  });

  QObject::connect(&settingsDialog, &SettingsDialog::forwardKeyChanged, [&camera](int key) {
    camera.setKeyForward(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::backwardKeyChanged, [&camera](int key) {
    camera.setKeyBackward(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::leftKeyChanged, [&camera](int key) {
    camera.setKeyLeft(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::rightKeyChanged, [&camera](int key) {
    camera.setKeyRight(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::turnLeftKeyChanged, [&camera](int key) {
    camera.setKeyTurnLeft(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::turnRightKeyChanged, [&camera](int key) {
    camera.setKeyTurnRight(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::upKeyChanged, [&camera](int key) {
    camera.setKeyUp(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::downKeyChanged, [&camera](int key) {
    camera.setKeyDown(key);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::playKeyChanged, &render, &SceneWidget::setPlayKey);

  QObject::connect(&settingsDialog, &SettingsDialog::rewindKeyChanged, &render, &SceneWidget::setRewindKey);

  QObject::connect(&settingsDialog, &SettingsDialog::resourcePathChanged, &render, &SceneWidget::setResourcePath);

  QObject::connect(ui.actionResetCameraPosition, &QAction::triggered, &render, &SceneWidget::resetCamera);

  QObject::connect(ui.actionAbout, &QAction::triggered, [this]() {
    AboutDialog dialog{this};
    dialog.exec();
  });

  QObject::connect(ui.actionAddChart, &QAction::triggered, [this]() {
    charts.spawnWidget(this);
  });

  QObject::connect(ui.actionRemovCharts, &QAction::triggered, &charts, &ChartManager::clearWidgets);
}

MainWindow::~MainWindow() {
  loadThread.quit();
  // Make sure the thread has time to close before trying to destroy it
  loadThread.wait();
}
void MainWindow::timeChanged(double time) {
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
    ui.statusbar->showMessage("Already loading scenario!", 10000);
  }
  loading = true;
  ui.actionLoad->setEnabled(false);
  statusLabel.setText("Loading scenario: " + fileName);
  render.reset();
  nodeWidget.reset();
  emit startLoading(fileName);
}

void MainWindow::finishLoading(const QString &fileName, unsigned long long milliseconds) {
  auto parser = loadWorker.getParser();
  render.setConfiguration(parser.getConfiguration());

  // Nodes, Buildings, Decorations
  const auto &nodes = parser.getNodes();
  render.add(parser.getAreas(), parser.getBuildings(), parser.getDecorations(), parser.getNodes());

  for (const auto &node : nodes) {
    nodeWidget.addNode(node);
  }

  // Charts
  charts.reset();
  charts.addSeries(parser.getXYSeries(), parser.getSeriesCollections(), parser.getCategoryValueSeries());

  // Log Streams
  logWidget.reset();
  const auto &logStreams = parser.getLogStreams();
  for (const auto &logStream : logStreams) {
    logWidget.addStream(logStream);
  }

  // Events
  const auto &sceneEvents = parser.getSceneEvents();
  render.enqueueEvents(sceneEvents);

  const auto &chartEvents = parser.getChartsEvents();
  charts.enqueueEvents(chartEvents);

  const auto &logEvents = parser.getLogEvents();
  logWidget.enqueueEvents(logEvents);

  std::clog << "Scenario loaded in " << milliseconds << "ms\n";
  ui.statusbar->showMessage("Successfully loaded scenario: " + fileName + " in " + QString::number(milliseconds) + "ms",
                            10000);
  statusLabel.setText("Ready");
  loading = false;
  ui.actionLoad->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings.set(SettingsManager::Key::MainWindowState, saveState(stateVersion));
  QMainWindow::closeEvent(event);
}

} // namespace visualization
