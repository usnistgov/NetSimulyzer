/*
 * NIST-developed software is provided by NIST as a public service. You may use,
 * copy and distribute copies of the software in any medium, provided that you
 * keep intact this entire notice. You may improve, modify and create derivative
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

#include "PlaybackQuick.h"
#include <QQmlContext>
#include <QQmlEngine>
#include <QUrl>

namespace netsimulyzer {

PlaybackQuick::PlaybackQuick(QWidget *parent) : QQuickWidget(parent), controller{this} {

  setResizeMode(QQuickWidget::SizeRootObjectToView);
  rootContext()->setContextProperty("playbackController", &controller);

  setSource(QUrl{QStringLiteral("qrc:/src/window/playback/PlaybackQuick.qml")});

  setFixedHeight(45);
  setContentsMargins(QMargins{});

  // Signals & Slots
  connect(&controller, &PlaybackController::play, this, &PlaybackQuick::play);
  connect(&controller, &PlaybackController::pause, this, &PlaybackQuick::pause);
  connect(&controller, &PlaybackController::timeSet, this, &PlaybackQuick::timeSet);
  connect(&controller, &PlaybackController::timeStepChanged, this, &PlaybackQuick::timeStepChanged);

  connect(&controller, &PlaybackController::requestJumpDialog, this, &PlaybackQuick::showJumpDialog);
  connect(&controller, &PlaybackController::requestTimeStepDialog, this, &PlaybackQuick::showTimeStepDialog);

  connect(&jumpDialog, &PlaybackJumpDialog::timeSelected, &controller, &PlaybackController::applyJumpedTime);
  connect(&timeStepDialog, &PlaybackTimeStepDialog::timeStepChanged, &controller, &PlaybackController::applyTimeStep);
}

void PlaybackQuick::showJumpDialog(parser::nanoseconds current, parser::nanoseconds max) {
  jumpDialog.setMaxTime(max);
  jumpDialog.setInputValue(current);
  jumpDialog.exec();
}

void PlaybackQuick::showTimeStepDialog() {
  timeStepDialog.show();
}

void PlaybackQuick::setMaxTime(parser::nanoseconds value) {
  controller.setMaxTime(value);
}

void PlaybackQuick::setTime(parser::nanoseconds simulationTime) {
  controller.setTime(simulationTime);
}

void PlaybackQuick::setTimeStep(parser::nanoseconds value, SettingsManager::TimeUnit unit) {
  controller.setTimeStep(value, unit);
  timeStepDialog.setValue(value, unit);
}

void PlaybackQuick::reset() {
  controller.reset();
}

void PlaybackQuick::enableControls() {
  controller.enableControls();
}

bool PlaybackQuick::isPlaying() const {
  return controller.isPlaying();
}

void PlaybackQuick::setPlaying() {
  controller.setPlaying();
}

void PlaybackQuick::setPaused() {
  controller.setPaused();
}

} // namespace netsimulyzer
