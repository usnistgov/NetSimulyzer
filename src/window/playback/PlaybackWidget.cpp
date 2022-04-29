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

#include "PlaybackWidget.h"
#include "src/conversion.h"
#include "src/settings/SettingsManager.h"
#include <QFontDatabase>
#include <QMessageBox>
#include <QObject>
#include <QPushButton>
#include <QString>
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

void PlaybackWidget::updateButtonSpeed(parser::nanoseconds step, SettingsManager::TimeUnit unit) {
  switch (unit) {
  case SettingsManager::TimeUnit::Nanoseconds:
    ui.buttonPlaybackSpeed->setText(QStringLiteral("%1%2").arg(step).arg("ns"));
    break;
  case SettingsManager::TimeUnit::Microseconds:
    ui.buttonPlaybackSpeed->setText(QStringLiteral("%1%2").arg(toMicroseconds(step)).arg("µs"));
    break;
  case SettingsManager::TimeUnit::Milliseconds:
    ui.buttonPlaybackSpeed->setText(QStringLiteral("%1%2").arg(toMilliseconds(step)).arg("ms"));
    break;
  }
}

void PlaybackWidget::setGranularity(SettingsManager::TimeUnit unit) {
  currentUnit = unit;
  formattedMaxTime = toDisplayTime(maxTime, currentUnit);

  // Round, so if we move from low to high precisions
  // (e.g. ns to ms), we are not potentially hiding values
  const auto roundedTime = ceilTime(currentTime, unit);
  if (roundedTime != currentTime)
    setTime(roundedTime);

  setTimeLabel(currentTime);
}

void PlaybackWidget::setTimeLabel(parser::nanoseconds time) {
  ui.labelTime->setText(toDisplayTime(time, currentUnit) + " / " + formattedMaxTime);
}

PlaybackWidget::PlaybackWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  ui.buttonPlayPause->setIcon(playIcon);

  const auto playbackSpeed = settings.get<int>(SettingsManager::Key::PlaybackTimeStepPreference).value();
  const auto unit = settings.get<SettingsManager::TimeUnit>(SettingsManager::Key::PlaybackTimeStepUnit).value();

  updateButtonSpeed(playbackSpeed, unit);
  setGranularity(unit);

  timeStepDialog.setValue(playbackSpeed, unit);

  // Pull the system fixed width font and use it for the numeric time
  ui.labelTime->setFont(QFontDatabase::systemFont(QFontDatabase::FixedFont));

  QObject::connect(ui.buttonPlayPause, &QPushButton::pressed, [this]() {
    playing = !playing;
    if (playing) {
      // If we're at the end, restart from the beginning
      if (currentTime == maxTime) {
        setTime(0LL);
        emit timeSet(0LL);
      }
      ui.buttonPlayPause->setIcon(pauseIcon);
      emit play();
    } else {
      setPaused();
      emit pause();
    }
  });

  QObject::connect(ui.timelineSlider, &QSlider::valueChanged, this, &PlaybackWidget::sliderMoved);

  QObject::connect(ui.buttonPlaybackSpeed, &QPushButton::clicked, [this]() {
    timeStepDialog.show();
  });

  QObject::connect(ui.buttonJump, &QPushButton::clicked, [this]() {
    setPaused();
    emit pause();

    jumpDialog.setMaxTime(maxTime);
    jumpDialog.setInputValue(currentTime);
    jumpDialog.exec();
  });

  QObject::connect(&jumpDialog, &PlaybackJumpDialog::timeSelected, [this](parser::nanoseconds time) {
    if (time > maxTime)
      time = maxTime;
    else if (time < 0LL)
      time = 0LL;

    setTime(time);
    emit timeSet(time);
    setPaused();
    emit pause();
  });

  QObject::connect(&timeStepDialog, &PlaybackTimeStepDialog::timeStepChanged,
                   [this](parser::nanoseconds newValue, int unit) {
                     const auto newUnit = SettingsManager::TimeUnitFromInt(unit);
                     updateButtonSpeed(newValue, newUnit);
                     setGranularity(newUnit);
                     emit timeStepChanged(newValue, unit);
                   });
}

void PlaybackWidget::setMaxTime(parser::nanoseconds value) {
  formattedMaxTime = toDisplayTime(value, currentUnit);
  maxTime = value;
  setTimeLabel(0LL);
  jumpDialog.setMaxTime(maxTime);

  // Roughly 2 secs
  if (maxTime <= std::numeric_limits<int>::max()) {
    ui.timelineSlider->setMaximum(static_cast<int>(maxTime));
    timeSliderStep = 1.0;
  } else {
    timeSliderStep = static_cast<double>(maxTime) / std::numeric_limits<int>::max();
    ui.timelineSlider->setMaximum(std::numeric_limits<int>::max());
  }
}

void PlaybackWidget::setTime(parser::nanoseconds simulationTime) {
  ignoreMove = true;

  if (simulationTime > maxTime)
    simulationTime = maxTime;

  currentTime = simulationTime;
  ui.timelineSlider->setValue(static_cast<int>(simulationTime / timeSliderStep));
  setTimeLabel(simulationTime);
  ignoreMove = false;
}

void PlaybackWidget::setTimeStep(parser::nanoseconds value, SettingsManager::TimeUnit unit) {
  updateButtonSpeed(value, unit);
  setGranularity(unit);
  timeStepDialog.setValue(value, unit);
}

void PlaybackWidget::sliderMoved(int value) {
  if (ignoreMove)
    return;
  auto timeValue = static_cast<parser::nanoseconds>(value * timeSliderStep);

  timeValue = ceilTime(timeValue, currentUnit);
  // Since we may have rounded, make sure we don't go over the max
  if (timeValue > maxTime)
    timeValue = maxTime;

  if (timeValue == maxTime && !playing)
    ui.buttonPlayPause->setIcon(resetIcon);
  else
    ui.buttonPlayPause->setIcon(playIcon);

  setTime(timeValue);
  emit timeSet(timeValue);
}

void PlaybackWidget::reset() {
  ui.timelineSlider->setValue(0);
  setMaxTime(0LL);
  currentTime = 0LL;

  ui.buttonPlayPause->setEnabled(false);
  ui.timelineSlider->setEnabled(false);
  ui.buttonJump->setEnabled(false);
}

void PlaybackWidget::enableControls() {
  ui.buttonPlayPause->setEnabled(true);
  ui.timelineSlider->setEnabled(true);
  ui.buttonJump->setEnabled(true);
}

bool PlaybackWidget::isPlaying() const {
  return playing;
}

void PlaybackWidget::setPlaying() {
  ui.buttonPlayPause->setIcon(pauseIcon);
  playing = true;
}

void PlaybackWidget::setPaused() {
  if (currentTime == maxTime)
    ui.buttonPlayPause->setIcon(resetIcon);
  else
    ui.buttonPlayPause->setIcon(playIcon);
  playing = false;
}

} // namespace netsimulyzer
