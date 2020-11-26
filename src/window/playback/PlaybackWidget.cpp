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
#include <QObject>
#include <QPushButton>
#include <QString>

namespace visualization {

PlaybackWidget::PlaybackWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
  ui.buttonPlayPause->setIcon(playIcon);

  QObject::connect(ui.buttonPlayPause, &QPushButton::pressed, [this]() {
    playing = !playing;
    if (playing) {
      ui.buttonPlayPause->setIcon(pauseIcon);
      emit play();
    } else {
      ui.buttonPlayPause->setIcon(playIcon);
      emit pause();
    }
  });

  QObject::connect(ui.timelineSlider, &QSlider::valueChanged, this, &PlaybackWidget::sliderMoved);
}

void PlaybackWidget::setMaxTime(double value) {
  formattedMaxTime = toDisplayTime(value);
  maxTime = value;
  ui.timelineSlider->setMaximum(static_cast<int>(value));
  ui.labelTime->setText(QString{"0.000 / "} + formattedMaxTime);
}

void PlaybackWidget::setTime(double simulationTime) {
  ui.timelineSlider->setValue(static_cast<int>(simulationTime));
  ui.labelTime->setText(toDisplayTime(simulationTime) + " / " + formattedMaxTime);
}

void PlaybackWidget::sliderMoved(int value) {
  setTime(static_cast<double>(value));
  emit timeSet(static_cast<double>(value));
}

void PlaybackWidget::reset() {
  ui.timelineSlider->setValue(0);
  setMaxTime(0.0);

  ui.buttonPlayPause->setEnabled(false);
  ui.timelineSlider->setEnabled(false);
}

void PlaybackWidget::enableControls() {
  ui.buttonPlayPause->setEnabled(true);
  ui.timelineSlider->setEnabled(true);
}

bool PlaybackWidget::isPlaying() const {
  return playing;
}

void PlaybackWidget::setPlaying() {
  ui.buttonPlayPause->setIcon(pauseIcon);
  playing = true;
}

void PlaybackWidget::setPaused() {
  ui.buttonPlayPause->setIcon(playIcon);
  playing = false;
}

} // namespace visualization
