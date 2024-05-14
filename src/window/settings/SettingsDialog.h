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
#include "src/window/scene/SceneWidget.h"
#include "ui_SettingsDialog.h"
#include <QColor>
#include <QDialog>
#include <QString>

namespace netsimulyzer {

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
  const float moveSpeedScale = 10000.0f;

  /**
   * Scale to divide the value of the turn speed sliders
   * to produce the actual turn speeds
   */
  const float turnSpeedScale = 10.0f;

  /**
   * Scale to divide the value of the label scale slider
   * to produce the actual value used for the setting
   */
  const float labelScaleScale = 100.0f;

  QString resourcePath = *settings.get<QString>(SettingsManager::Key::ResourcePath);

  /**
   * Default vale to use for the time step.
   *
   * Also set by `setTimeStep`
   */
  double passedTimeStep = 10.0;

  /**
   * The user specified clear color.
   * Used when the skybox is off
   */
  QColor customBackgroundColor = settings.get<QColor>(SettingsManager::Key::RenderBackgroundColorCustom).value();

  /**
   * Sets the suffix in the time step preference SpinBox
   *
   * @param unit
   * The use to use for the suffix
   */
  void setStepSpinSuffix(SettingsManager::TimeUnit unit);

  /**
   * Callback for when a button at the bottom of the dialog is clicked
   *
   * @param button
   * A reference to the clicked button
   */
  void dialogueButtonClicked(QAbstractButton *button);

  /**
   * Set the Window Theme combobox to the default value
   */
  void defaultWindowTheme();

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
   * Set the Chart Sort Order input to the default value
   */
  void defaultChartSortOrder();

  /**
   * Set the Samples input to the default value
   */
  void defaultSamples();

  /**
   * Set the background color combo box to it's default value
   */
  void defaultBackgroundColor();

  /**
   * Set the Skybox checkbox to the default value
   */
  void defaultEnableSkybox();

  /**
   * Set the Camera Type combobox to the default value
   */
  void defaultCameraType();

  /**
   * Set the Floor checkbox to the default value
   */
  void defaultEnableFloor();

  /**
   * Sets the building render mode to the default value
   */
  void defaultBuildingEffect();

  /**
   * Sets the "Render Building Outlines" checkbox
   * to its default
   */
  void defaultBuildingOutlines();

  /**
   * Set the default value for the time step spinner.
   *
   * Sets the value passed to `setTimeStep` if one was provided
   */
  void defaultTimeStep();

  /**
   * Set the grid checkbox to the default value
   */
  void defaultShowGrid();

  /**
   * Set the checkbox that enables motion
   * trails to the default state
   */
  void defaultShowTrails();

  /**
   * Sets the Motion Trail Length
   * spinner to the default value
   */
  void defaultTrailsLength();

  /**
   * Sets the Show Labels
   * checkbox to the default value
   */
  void defaultShowLabels();

  /**
   * Sets the Label Scale
   * slider to the default value
   */
  void defaultLabelScale();

  /**
   * Sets the grid step size spinner to its default value
   */
  void defaultGridStep();

  /**
   * Brings up a file select dialog for choosing a resource directory.
   * Validates and stores result in `resourcePath` and the `LineEdit`
   */
  void selectResourcePath();

public:
  explicit SettingsDialog(QWidget *parent = nullptr);

  /**
   * Load the saved settings into each input
   */
  void loadSettings();

  /**
   * Sets the time step spinner to `value`.
   * Does not trigger the `timeStepSet` signal
   *
   * @param value
   * The value to set the spinner to.
   * Should be a whole number
   */
  void setTimeStep(double value);

signals:

  // Ignore unimplemented function warning
  // for Qt Signals, since those are generated
  // For us elsewhere
#pragma clang diagnostic push
#pragma ide diagnostic ignored "NotImplementedFunctions"

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
   * Signal emitted when the user changes the Chart sort order.
   *
   * @param value
   * A value from `SettingsManager::ChartDropdownSortOrder` converted to
   * an `int` for messaging.
   *
   * @see SettingsManager::ChartDropdownSortOrderFromInt
   */
  void chartSortOrderChanged(int value);

  /**
   * Signal emitted when the user changes the
   * Skybox render state
   *
   * @param enable
   * Indicator for the skybox to be rendered or not.
   */
  void renderSkyboxChanged(bool enable);

  /**
   * Signal emitted when the user changes the camera type.
   *
   * @param value
   * A value from `SettingsManager::CameraType` converted to
   * an `int` for messaging.
   *
   * @see SettingsManager::CameraTypeFromInt
   */
  void cameraTypeChanged(int value);

  /**
   * Signal emitted when the user changes the
   * Floor render state
   *
   * @param enable
   * Indicator for the floor to be rendered or not.
   */
  void renderFloorChanged(bool enable);

  /**
   * Signal emitted when the user changes the background color.
   *
   * @param value
   * The new value to use for the clear color
   */
  void backgroundColorChanged(QColor value);

  /**
   * Signal emitted when the user changes the
   * Building render mode.
   *
   * @param value
   * Enum value from SettingsManager::BuildingRenderMode
   */
  void buildingRenderModeChanged(int value);

  /**
   * Signal emitted when the user changes the
   * Building outline mode.
   *
   * @param value
   * Indicator for building outlines to be rendered or not.
   */
  void buildingRenderOutlinesChanged(bool enable);

  /**
   * Signal emitted when the user changes the
   * Grid render state
   *
   * @param enable
   * Indicator for the grid to be rendered or not.
   */
  void renderGridChanged(bool enable);

  /**
   * Signal emitted when the user changes the
   * Grid step size
   *
   * @param stepSize
   * The new size for grid squares
   */
  void gridStepSizeChanged(int stepSize);

  /**
   * Signal emitted when the user changes the
   * Motion Trail render mode.
   *
   * @param value
   * Enum value from SettingsManager::MotionTrailRenderMode
   */
  void renderTrailsChanged(int value);

  /**
   * Signal emitted when the user changes the
   * Label render mode.
   *
   * @param value
   * Enum value from SettingsManager::LabelRenderMode
   */
  void renderLabelsChanged(int value);

  /**
   * Signal emitted when the user changes the
   * Label scale.
   *
   * @param value
   * The scale to use for the Label Scale
   */
  void labelScaleChanged(float value);

  /**
   * Signal emitted when the user saves a new Play/Pause Key
   *
   * @param key
   * The Qt keycode, From `Qt::Key`
   */
  void playKeyChanged(int key);

  /**
   * Signal emitted when the user saves a new Resource Directory.
   *
   * @param dir
   * The absolute path to the resource directory,
   * with a trailing slash.
   */
  void resourcePathChanged(const QString &dir);

#pragma clang diagnostic pop
};

} // namespace netsimulyzer
