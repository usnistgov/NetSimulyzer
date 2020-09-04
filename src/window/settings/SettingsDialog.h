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
#include "ui_SettingsDialog.h"
#include <QDialog>
#include <QString>

namespace visualization {

/**
 * Dialog for configuring application settings
 */
class SettingsDialog : public QDialog {
  Q_OBJECT
  Ui::SettingsDialog ui{};
  SettingsManager settings;

  /**
   * Scale to divide the value of the Move Speed slider
   * to produce the actual move speed
   */
  const float moveSpeedScale = 1000.0f;

  /**
   * Scale to divide the value of the turn speed sliders
   * to produce the actual turn speeds
   */
  const float turnSpeedScale = 10.0f;

  QString resourcePath = *settings.get<QString>(SettingsManager::Key::ResourcePath);

  /**
   * Callback for when a button at the bottom of the dialog is clicked
   *
   * @param button
   * A reference to the clicked button
   */
  void dialogueButtonClicked(QAbstractButton *button);

  /**
   * Set the Move Speed slider to the default value from the Settings Manager
   */
  void defaultMoveSpeed();

  /**
   * Set the Keyboard Turn Speed slider to the default value from the Settings Manager
   */
  void defaultKeyboardTurnSpeed();

  /**
   * Set the Mouse Turn Speed slider to the default value from the Settings Manager
   */
  void defaultMouseTurnSpeed();

  /**
   * Set the Field of View slider to the default value from the Settings Manager
   */
  void defaultFieldOfView();

  /**
   * Set the Samples input to the default value
   */
  void defaultSamples();

  /**
   * Brings up a file select dialog for choosing a resource directory.
   * Validates and stores result in `resourcePath` and the `LineEdit`
   */
  void selectResourcePath();

  /**
   * Load the saved settings into each input
   */
  void loadSettings();

public:
  explicit SettingsDialog(QWidget *parent = nullptr);

signals:
  /**
   * Signal emitted when the user saves a new Move Speed.
   *
   * @param value
   * The value from the slider, scaled for the camera
   */
  void moveSpeedChanged(float value);

  /**
   * Signal emitted when the user saves a new Keyboard Turn Speed.
   *
   * @param value
   * The value from the slider, scaled for the camera
   */
  void keyboardTurnSpeedChanged(float value);

  /**
   * Signal emitted when the user saves a new Mouse Turn Speed.
   *
   * @param value
   * The value from the slider, scaled for the camera
   */
  void mouseTurnSpeedChanged(float value);

  /**
   * Signal emitted when the user saves a new Field of View angle.
   *
   * @param value
   * The value from the slider.
   */
  void fieldOfViewChanged(float value);

  /**
   * Signal emitted when the user saves a new Forward Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void forwardKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Backward Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void backwardKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Left Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void leftKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Right Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void rightKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Turn Left Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void turnLeftKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Turn Right Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void turnRightKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Up Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void upKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Down Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void downKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Playback Speed
   *
   * @param ms
   * The amount of time to pass per frame of the simulation
   * in milliseconds
   */
  void playbackSpeedChanged(int ms);

  /**
   * Signal emitted when the user saves a new Play/Pause Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void playKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Rewind Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void rewindKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Resource Directory.
   *
   * @param dir
   * The absolute path to the resource directory,
   * with a trailing slash.
   */
  void resourcePathChanged(const QString &dir);
};

} // namespace visualization
