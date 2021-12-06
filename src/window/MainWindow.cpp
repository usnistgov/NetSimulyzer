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
#include "src/conversion.h"
#include "src/window/util/file-operations.h"
#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QMessageBox>
#include <QObject>
#include <cstdlib>
#include <iostream>
#include <parser/file-parser.h>
#include <parser/model.h>
#include <project.h>

namespace netsimulyzer {

MainWindow::MainWindow() : QMainWindow() {
  ui.setupUi(this);
  setWindowTitle(NETSIMULYZER_APPLICATION_NAME);
  setCentralWidget(&scene);

  ui.nodesDock->setWidget(&nodeWidget);
  ui.logDock->setWidget(&logWidget);
  ui.playbackDock->setWidget(&playbackWidget);

  auto state = settings.get<QByteArray>(SettingsManager::Key::MainWindowState);
  if (state)
    restoreState(*state, stateVersion);

  loadWorker.moveToThread(&loadThread);
  QObject::connect(this, &MainWindow::startLoading, &loadWorker, &LoadWorker::load);
  QObject::connect(&loadWorker, &LoadWorker::fileLoaded, this, &MainWindow::finishLoading);
  QObject::connect(&loadWorker, &LoadWorker::error, this, &MainWindow::errorLoading);
  loadThread.start();

  ui.menuWindow->addAction(ui.nodesDock->toggleViewAction());
  ui.menuWindow->addAction(ui.logDock->toggleViewAction());
  ui.menuWindow->addAction(ui.playbackDock->toggleViewAction());

  // For somewhat permanent messages (a message with no timeout)
  // We need to use a widget in the status bar.
  // Note: This message can still be temporarily overwritten,
  // should we choose to do so
  ui.statusbar->insertWidget(0, &statusLabel);

  QObject::connect(&scene, &SceneWidget::timeChanged, this, &MainWindow::timeChanged);
  QObject::connect(&scene, &SceneWidget::timeChanged, &charts, &ChartManager::timeChanged);
  QObject::connect(&scene, &SceneWidget::timeChanged, &logWidget, &ScenarioLogWidget::timeChanged);
  QObject::connect(&scene, &SceneWidget::timeChanged, [this](double time, double /* increment */) {
    playbackWidget.setTime(time);
  });

  QObject::connect(ui.actionPlayPause, &QAction::triggered, [this]() {
    if (playbackWidget.isPlaying()) {
      playbackWidget.setPaused();
      scene.pause();
      return;
    }

    playbackWidget.setPlaying();
    scene.play();
  });

  QObject::connect(&playbackWidget, &PlaybackWidget::play, &scene, &SceneWidget::play);
  QObject::connect(&playbackWidget, &PlaybackWidget::pause, &scene, &SceneWidget::pause);
  // Playback widget value is above user preference in priority
  QObject::connect(&playbackWidget, &PlaybackWidget::timeStepChanged, &scene, &SceneWidget::setTimeStep);

  QObject::connect(&playbackWidget, &PlaybackWidget::timeSet, &scene, &SceneWidget::setTime);

  QObject::connect(&scene, &SceneWidget::paused, &playbackWidget, &PlaybackWidget::setPaused);
  QObject::connect(&scene, &SceneWidget::playing, &playbackWidget, &PlaybackWidget::setPlaying);

  QObject::connect(&nodeWidget, &NodeWidget::nodeSelected, &scene, &SceneWidget::focusNode);

  QObject::connect(ui.actionLoad, &QAction::triggered, this, &MainWindow::load);

  QObject::connect(ui.actionSettings, &QAction::triggered, [this]() {
    scene.pause();
    settingsDialog.show();
  });

  auto &camera = scene.getCamera();
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
    scene.updatePerspective();
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

  QObject::connect(&settingsDialog, &SettingsDialog::chartSortOrderChanged, [this](int value) {
    charts.setSortOrder(SettingsManager::ChartDropdownSortOrderFromInt(value));
  });

  QObject::connect(&settingsDialog, &SettingsDialog::renderSkyboxChanged, [this](bool enable) {
    scene.setSkyboxRenderState(enable);
  });

  QObject::connect(&settingsDialog, &SettingsDialog::buildingRenderModeChanged, [this](int mode) {
    scene.setBuildingRenderMode(SettingsManager::BuildingRenderModeFromInt(mode));
  });

  QObject::connect(&settingsDialog, &SettingsDialog::buildingRenderOutlinesChanged, &scene,
                   &SceneWidget::setBuildingRenderOutlines);
  QObject::connect(&settingsDialog, &SettingsDialog::renderGridChanged, &scene, &SceneWidget::setRenderGrid);
  QObject::connect(&settingsDialog, &SettingsDialog::gridStepSizeChanged, &scene, &SceneWidget::changeGridStepSize);

  QObject::connect(&settingsDialog, &SettingsDialog::playKeyChanged, [this](int key) {
    ui.actionPlayPause->setShortcut(QKeySequence{key});
  });

  QObject::connect(&settingsDialog, &SettingsDialog::resourcePathChanged, &scene, &SceneWidget::setResourcePath);

  QObject::connect(ui.actionResetCameraPosition, &QAction::triggered, &scene, &SceneWidget::resetCamera);

  QObject::connect(ui.actionAbout, &QAction::triggered, [this]() {
    scene.pause();
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

void MainWindow::timeChanged(double time, double /* increment */) {
  statusLabel.setText(toDisplayTime(time));
}

void MainWindow::load() {
  auto fileName = getScenarioFile(this);

  if (fileName.isEmpty())
    return;
  if (loading) {
    ui.statusbar->showMessage("Already loading scenario!", 10000);
    return;
  }
  loading = true;
  ui.actionLoad->setEnabled(false);
  statusLabel.setText("Loading scenario: " + fileName);
  scene.reset();
  nodeWidget.reset();
  playbackWidget.reset();
  charts.reset();
  emit startLoading(fileName);
}

void MainWindow::finishLoading(const QString &fileName, unsigned long long milliseconds) {
  auto parser = loadWorker.getParser();
  const auto config = parser.getConfiguration();
  scene.setConfiguration(config);

  playbackWidget.setMaxTime(config.endTime);

  const int timeStepPreference = settings.get<int>(SettingsManager::Key::PlaybackTimeStepPreference).value();
  scene.setTimeStep(config.timeStep.value_or(timeStepPreference));
  playbackWidget.setTimeStep(config.timeStep.value_or(timeStepPreference));

  // Nodes, Buildings, Decorations
  const auto &nodes = parser.getNodes();
  scene.add(parser.getAreas(), parser.getBuildings(), parser.getDecorations(), parser.getLinks(), parser.getNodes());

  for (const auto &node : nodes) {
    nodeWidget.addNode(node);
  }

  // Charts
  charts.addSeries(parser.getXYSeries(), parser.getSeriesCollections(), parser.getCategoryValueSeries());

  // Log Streams
  logWidget.reset();
  const auto &logStreams = parser.getLogStreams();
  for (const auto &logStream : logStreams) {
    logWidget.addStream(logStream);
  }

  // Events
  const auto &sceneEvents = parser.getSceneEvents();
  scene.enqueueEvents(sceneEvents);

  const auto &chartEvents = parser.getChartsEvents();
  charts.enqueueEvents(chartEvents);

  const auto &logEvents = parser.getLogEvents();
  logWidget.enqueueEvents(logEvents);

  std::clog << "Scenario loaded in " << milliseconds << "ms\n";
  ui.statusbar->showMessage("Successfully loaded scenario: " + fileName + " in " + QString::number(milliseconds) + "ms",
                            10000);

  playbackWidget.enableControls();
  statusLabel.setText("Ready");
  loading = false;
  ui.actionLoad->setEnabled(true);
}

void MainWindow::errorLoading(const QString &message, unsigned long long offset) {
  QMessageBox::critical(this, "Parsing Error", message + " at: " + QString::number(offset) + " characters");

  statusLabel.setText("Error loading scenario");
  loading = false;
  ui.actionLoad->setEnabled(true);
}

void MainWindow::closeEvent(QCloseEvent *event) {
  settings.set(SettingsManager::Key::MainWindowState, saveState(stateVersion));
  QMainWindow::closeEvent(event);
}

} // namespace netsimulyzer
