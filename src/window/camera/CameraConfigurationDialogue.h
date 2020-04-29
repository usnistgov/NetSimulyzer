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
  const int defaultTurnSpeed = 5;
  const int defaultFieldOfView = 45;
  const bool defaultAllowCameraEvents = true;
  const int defaultForwardKey = Qt::Key_W;
  const int defaultBackwardKey = Qt::Key_S;
  const int defaultLeftKey = Qt::Key_A;
  const int defaultRightKey = Qt::Key_D;

  Ui::CameraConfigurationDialogue *ui;
  Camera &camera;
  void moveSpeedChanged(int value);
  void turnSpeedChanged(int value);
  void fieldOfViewChanged(int value);

  void forwardKeyChanged(const QKeySequence &value);
  void backwardKeyChanged(const QKeySequence &value);
  void leftKeyChanged(const QKeySequence &value);
  void rightKeyChanged(const QKeySequence &value);

  void dialogueButtonClicked(QAbstractButton *button);
};
} // namespace visualization
