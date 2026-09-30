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

#include "PlaybackController.h"
#include "src/conversion.h"
#include <QStringLiteral>
#include <limits>

namespace {

parser::nanoseconds ceilTime(parser::nanoseconds time, netsimulyzer::SettingsManager::TimeUnit unit) {
  using TimeUnit = netsimulyzer::SettingsManager::TimeUnit;

  if (unit == TimeUnit::Microseconds && time % 1'000LL > 0LL)
    time += (1'000LL - time % 1'000LL);
  else if (unit == TimeUnit::Milliseconds && time % 1'000'000LL > 0LL)
    time += (1'000'000LL - time % 1'000'000LL);

  return time;
}

} // namespace

namespace netsimulyzer {

PlaybackController::PlaybackController(QObject *parent) : QObject(parent) {
  const auto playbackSpeed = settings.get<int>(SettingsManager::Key::PlaybackTimeStepPreference).value();
  const auto unit = settings.get<SettingsManager::TimeUnit>(SettingsManager::Key::PlaybackTimeStepUnit).value();

  timeStep = playbackSpeed;
  currentUnit = unit;
  updateSpeedLabel();
  setGranularity(unit);
}

bool PlaybackController::playing() const {
  return playingState;
}

bool PlaybackController::atEnd() const {
  return atEndState;
}

bool PlaybackController::controlsEnabled() const {
  return controlsEnabledState;
}

qreal PlaybackController::sliderPosition() const {
  // the QML slider is 0 - 1.0,
  // so we normalize
  if (maxTime <= 0LL)
    return 0.0;
  return static_cast<qreal>(currentTime) / static_cast<qreal>(maxTime);
}

QString PlaybackController::timeLabel() const {
  return timeLabelText;
}

QString PlaybackController::speedLabel() const {
  return speedLabelText;
}

void PlaybackController::togglePlayPause() {
  setPlayingInternal(!playingState);

  if (playingState) {
    // If we're at the end, restart from the beginning
    if (currentTime == maxTime) {
      setTime(0LL);
      emit timeSet(0LL);
    }
    emit play();
  } else {
    setPaused();
    emit pause();
  }
}

void PlaybackController::sliderMoved(qreal pos) {
  if (pos < 0.0)
    pos = 0.0;
  else if (pos > 1.0)
    pos = 1.0;

  auto timeValue = static_cast<parser::nanoseconds>(pos * static_cast<qreal>(maxTime));

  timeValue = ceilTime(timeValue, currentUnit);
  // Since we may have rounded, make sure we don't go over the max
  if (timeValue > maxTime)
    timeValue = maxTime;

  setTime(timeValue);
  emit timeSet(timeValue);
}

void PlaybackController::openJumpDialog() {
  setPaused();
  emit pause();
  emit requestJumpDialog(currentTime, maxTime);
}

void PlaybackController::openTimeStepDialog() {
  emit requestTimeStepDialog();
}

void PlaybackController::setMaxTime(parser::nanoseconds value) {
  maxTime = value;
  formattedMaxTime = toDisplayTime(value, currentUnit);
  updateTimeLabel();

  // maxTime changed, so the at-end state may have too
  setAtEndInternal(currentTime == maxTime);
  emit sliderPositionChanged();
}

void PlaybackController::setTime(parser::nanoseconds simulationTime) {
  if (simulationTime > maxTime)
    simulationTime = maxTime;

  currentTime = simulationTime;
  updateTimeLabel();
  setAtEndInternal(currentTime == maxTime);
  emit sliderPositionChanged();
}

void PlaybackController::setTimeStep(parser::nanoseconds value, SettingsManager::TimeUnit unit) {
  timeStep = value;
  updateSpeedLabel();
  setGranularity(unit);
}

void PlaybackController::setGranularity(SettingsManager::TimeUnit unit) {
  currentUnit = unit;
  formattedMaxTime = toDisplayTime(maxTime, currentUnit);

  // Round, so moving from low to high precision (e.g. ns to ms)
  // doesn't potentially hide values.
  const auto roundedTime = ceilTime(currentTime, unit);
  if (roundedTime != currentTime)
    setTime(roundedTime);

  updateTimeLabel();
  updateSpeedLabel();
}

void PlaybackController::reset() {
  maxTime = 0LL;
  currentTime = 0LL;
  formattedMaxTime = toDisplayTime(0LL, currentUnit);
  updateTimeLabel();

  setPlayingInternal(false);
  setAtEndInternal(true);

  controlsEnabledState = false;
  emit controlsEnabledChanged();
  emit sliderPositionChanged();
}

void PlaybackController::enableControls() {
  controlsEnabledState = true;
  emit controlsEnabledChanged();
}

void PlaybackController::setPlaying() {
  setPlayingInternal(true);
}

void PlaybackController::setPaused() {
  // Just making sure the icon is right
  setAtEndInternal(currentTime == maxTime);
  setPlayingInternal(false);
}

void PlaybackController::applyJumpedTime(parser::nanoseconds time) {
  if (time > maxTime)
    time = maxTime;
  else if (time < 0LL)
    time = 0LL;

  setTime(time);
  emit timeSet(time);
  setPaused();
  emit pause();
}

void PlaybackController::applyTimeStep(const parser::nanoseconds value, const int unit) {
  const auto newUnit = SettingsManager::TimeUnitFromInt(unit);
  timeStep = value;
  updateSpeedLabel();
  setGranularity(newUnit);
  emit timeStepChanged(value, unit);
}

void PlaybackController::updateTimeLabel() {
  if (const auto next = toDisplayTime(currentTime, currentUnit) + " / " + formattedMaxTime; next != timeLabelText) {
    timeLabelText = next;
    emit timeLabelChanged();
  }
}

void PlaybackController::updateSpeedLabel() {
  QString next;
  switch (currentUnit) {
  case SettingsManager::TimeUnit::Nanoseconds:
    next = QStringLiteral("%1%2").arg(timeStep).arg("ns");
    break;
  case SettingsManager::TimeUnit::Microseconds:
    next = QStringLiteral("%1%2").arg(toMicroseconds(timeStep)).arg("µs");
    break;
  case SettingsManager::TimeUnit::Milliseconds:
    next = QStringLiteral("%1%2").arg(toMilliseconds(timeStep)).arg("ms");
    break;
  }

  if (next != speedLabelText) {
    speedLabelText = next;
    emit speedLabelChanged();
  }
}

void PlaybackController::setPlayingInternal(const bool value) {
  if (playingState != value) {
    playingState = value;
    emit playingChanged();
  }
}

void PlaybackController::setAtEndInternal(const bool value) {
  if (atEndState != value) {
    atEndState = value;
    emit atEndChanged();
  }
}

} // namespace netsimulyzer
