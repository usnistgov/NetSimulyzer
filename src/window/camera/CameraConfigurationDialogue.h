#pragma once
#include "../../render/camera/Camera.h"
#include "../../settings/SettingsManager.h"
#include <QAbstractButton>
#include <QDialog>
#include <QKeySequence>

namespace Ui {
class CameraConfigurationDialogue;
}

namespace visualization {
class CameraConfigurationDialogue : public QDialog {
  Q_OBJECT

  SettingsManager settings;
  const float moveSpeedScale = 1000.0f;
  const int defaultMoveSpeed = settings.getDefault<float>(SettingsManager::Key::MoveSpeed) * moveSpeedScale;

  const float turnSpeedScale = 10.0f;
  const int defaultMouseTurnSpeed =
      static_cast<int>(settings.getDefault<float>(SettingsManager::Key::MouseTurnSpeed) * turnSpeedScale);
  const int defaultKeyboardTurnSpeed =
      static_cast<int>(settings.getDefault<float>(SettingsManager::Key::KeyboardTurnSpeed) * turnSpeedScale);

  const int defaultFieldOfView = settings.getDefault<float>(SettingsManager::Key::FieldOfView);
  const bool defaultAllowCameraEvents = settings.getDefault<bool>(SettingsManager::Key::AllowCameraEvents);
  const bool defaultUseMouseControls = settings.getDefault<bool>(SettingsManager::Key::CameraMouseControls);

  Ui::CameraConfigurationDialogue *ui;
  Camera &camera;
  void loadSettings();
  void moveSpeedChanged(int value);
  void keyboardTurnSpeedChanged(int value);
  void mouseTurnSpeedChanged(int value);
  void fieldOfViewChanged(int value);

  void useMouseControlsChanged(int value);

  void forwardKeyChanged();
  void backwardKeyChanged();
  void leftKeyChanged();
  void rightKeyChanged();
  void leftTurnKeyChanged();
  void rightTurnKeyChanged();
  void upKeyChanged();
  void downKeyChanged();

  void dialogueButtonClicked(QAbstractButton *button);

public:
  CameraConfigurationDialogue(Camera &camera, QWidget *parent = nullptr);
  ~CameraConfigurationDialogue() override;

signals:
  void perspectiveUpdated();
};

} // namespace visualization
