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

#pragma once

#include "src/settings/SettingsManager.h"
#include <QObject>
#include <QString>
#include <parser/model.h>

namespace netsimulyzer {

class PlaybackController : public QObject {
  Q_OBJECT

  Q_PROPERTY(bool playing READ playing NOTIFY playingChanged)
  Q_PROPERTY(bool atEnd READ atEnd NOTIFY atEndChanged)
  Q_PROPERTY(bool controlsEnabled READ controlsEnabled NOTIFY controlsEnabledChanged)
  Q_PROPERTY(qreal sliderPosition READ sliderPosition NOTIFY sliderPositionChanged)
  Q_PROPERTY(QString timeLabel READ timeLabel NOTIFY timeLabelChanged)
  Q_PROPERTY(QString speedLabel READ speedLabel NOTIFY speedLabelChanged)

public:
  explicit PlaybackController(QObject *parent = nullptr);

  [[nodiscard]] bool playing() const;
  [[nodiscard]] bool atEnd() const;
  [[nodiscard]] bool controlsEnabled() const;
  [[nodiscard]] qreal sliderPosition() const;
  [[nodiscard]] QString timeLabel() const;
  [[nodiscard]] QString speedLabel() const;

  Q_INVOKABLE void togglePlayPause();

  Q_INVOKABLE void sliderMoved(qreal pos);

  Q_INVOKABLE void openJumpDialog();
  Q_INVOKABLE void openTimeStepDialog();

  void setMaxTime(parser::nanoseconds value);
  void setTime(parser::nanoseconds simulationTime);
  void setTimeStep(parser::nanoseconds value, SettingsManager::TimeUnit unit);
  void setGranularity(SettingsManager::TimeUnit unit);
  void reset();
  void enableControls();
  void setPlaying();
  void setPaused();

  [[nodiscard]] bool isPlaying() const {
    return playingState;
  }
  [[nodiscard]] parser::nanoseconds getCurrentTime() const {
    return currentTime;
  }

  void applyJumpedTime(parser::nanoseconds time);
  void applyTimeStep(parser::nanoseconds value, int unit);

signals:
  void play();
  void pause();
  void timeSet(parser::nanoseconds time);
  void timeStepChanged(parser::nanoseconds value, int unit);

  void requestJumpDialog(parser::nanoseconds current, parser::nanoseconds max);
  void requestTimeStepDialog();

  void playingChanged();
  void atEndChanged();
  void controlsEnabledChanged();
  void sliderPositionChanged();
  void timeLabelChanged();
  void speedLabelChanged();

private:
  void updateTimeLabel();
  void updateSpeedLabel();
  void setPlayingInternal(bool value);
  void setAtEndInternal(bool value);

  SettingsManager settings;

  parser::nanoseconds currentTime{0LL};
  parser::nanoseconds maxTime{0LL};
  parser::nanoseconds timeStep{0LL};
  SettingsManager::TimeUnit currentUnit =
      settings.get<SettingsManager::TimeUnit>(SettingsManager::Key::PlaybackTimeStepUnit).value();

  bool playingState{false};
  bool atEndState{true};
  bool controlsEnabledState{false};

  QString timeLabelText{"0.000 / 0.000"};
  QString speedLabelText{"0"};
  QString formattedMaxTime{"0.000"};
};

} // namespace netsimulyzer
