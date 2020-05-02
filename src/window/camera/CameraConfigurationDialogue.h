#pragma once
#include "../../render/camera/Camera.h"
#include <QAbstractButton>
#include <QDialog>
#include <QKeySequence>

namespace Ui {
class CameraConfigurationDialogue;
}

namespace visualization {
class CameraConfigurationDialogue : public QDialog {
  Q_OBJECT

public:
  CameraConfigurationDialogue(Camera &camera, QWidget *parent = nullptr);
  ~CameraConfigurationDialogue() override;

signals:
  void perspectiveUpdated();

private:
  const int defaultMoveSpeed = 5;
  const int defaultMouseTurnSpeed = 5;
  const int defaultKeyboardTurnSpeed = 1;
  const int defaultFieldOfView = 45;
  const bool defaultAllowCameraEvents = true;
  const bool defaultUseMouseControls = true;
  const int defaultForwardKey = Qt::Key_W;
  const int defaultBackwardKey = Qt::Key_S;
  const int defaultLeftKey = Qt::Key_A;
  const int defaultRightKey = Qt::Key_D;
  const int defaultLeftTurnKey = Qt::Key_Q;
  const int defaultRightTurnKey = Qt::Key_E;
  const int defaultUpKey = Qt::Key_Z;
  const int defaultDownKey = Qt::Key_X;

  Ui::CameraConfigurationDialogue *ui;
  Camera &camera;
  void moveSpeedChanged(int value);
  void keyboardTurnSpeedChanged(int value);
  void mouseTurnSpeedChanged(int value);
  void fieldOfViewChanged(int value);

  void allowCameraEventsChanged(int value);

  void forwardKeyChanged(const QKeySequence &value);
  void backwardKeyChanged(const QKeySequence &value);
  void leftKeyChanged(const QKeySequence &value);
  void rightKeyChanged(const QKeySequence &value);
  void leftTurnKeyChanged(const QKeySequence &value);
  void rightTurnKeyChanged(const QKeySequence &value);
  void upKeyChanged(const QKeySequence &value);
  void downKeyChanged(const QKeySequence &value);

  void dialogueButtonClicked(QAbstractButton *button);
};
} // namespace visualization
