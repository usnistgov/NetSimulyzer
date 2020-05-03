#include "CameraConfigurationDialogue.h"
#include <QObject>
#include <QSlider>
#include <iostream>
#include <ui_CameraConfigurationDialogue.h>

namespace visualization {

CameraConfigurationDialogue::CameraConfigurationDialogue(Camera &camera, QWidget *parent)
    : QDialog(parent), ui(new Ui::CameraConfigurationDialogue), camera(camera) {
  ui->setupUi(this);

  QObject::connect(ui->sliderMoveSpeed, &QSlider::valueChanged, this, &CameraConfigurationDialogue::moveSpeedChanged);
  ui->sliderMoveSpeed->setValue(static_cast<int>(camera.getMoveSpeed() * 100.0f));
  QObject::connect(ui->pushButtonResetMoveSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderMoveSpeed->setValue(defaultMoveSpeed); });
  ui->pushButtonResetMoveSpeed->setEnabled(ui->sliderMoveSpeed->value() != defaultMoveSpeed);

  QObject::connect(ui->sliderKeyboardTurnSpeed, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::keyboardTurnSpeedChanged);
  ui->sliderKeyboardTurnSpeed->setValue(static_cast<int>(camera.getTurnSpeed() * 10.0f));
  QObject::connect(ui->pushButtonResetKeyboardTurnSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderKeyboardTurnSpeed->setValue(defaultKeyboardTurnSpeed); });
  ui->pushButtonResetKeyboardTurnSpeed->setEnabled(ui->sliderKeyboardTurnSpeed->value() != defaultKeyboardTurnSpeed);

  QObject::connect(ui->sliderMouseTurnSpeed, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::mouseTurnSpeedChanged);
  ui->sliderMouseTurnSpeed->setValue(static_cast<int>(camera.getMouseTurnSpeed() * 10.0f));
  QObject::connect(ui->pushButtonResetMouseTurnSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderMouseTurnSpeed->setValue(defaultMouseTurnSpeed); });
  ui->pushButtonResetMouseTurnSpeed->setEnabled(ui->sliderMouseTurnSpeed->value() != defaultMouseTurnSpeed);

  QObject::connect(ui->sliderFieldOfView, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::fieldOfViewChanged);
  ui->sliderFieldOfView->setValue(static_cast<int>(camera.getFieldOfView()));
  QObject::connect(ui->pushButtonResetFieldOfView, &QPushButton::clicked,
                   [this]() { ui->sliderFieldOfView->setValue(defaultFieldOfView); });
  ui->pushButtonResetFieldOfView->setEnabled(ui->sliderFieldOfView->value() != defaultFieldOfView);

  QObject::connect(ui->checkBoxMouseControls, &QCheckBox::stateChanged, this,
                   &CameraConfigurationDialogue::useMouseControlsChanged);

  QObject::connect(ui->pushButtonResetMouseControlsEnabled, &QPushButton::clicked,
                   [this]() { ui->checkBoxMouseControls->setChecked(defaultUseMouseControls); });
  ui->pushButtonResetMouseControlsEnabled->setEnabled(ui->checkBoxMouseControls->isChecked() !=
                                                      defaultUseMouseControls);

  QObject::connect(ui->keySequenceEditForward, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::forwardKeyChanged);
  QObject::connect(ui->keySequenceEditForward, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditForward->keySequence().count() == 0)
      ui->keySequenceEditForward->setKeySequence(defaultForwardKey);
  });

  QObject::connect(ui->keySequenceEditBackward, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::backwardKeyChanged);
  QObject::connect(ui->keySequenceEditBackward, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditBackward->keySequence().count() == 0)
      ui->keySequenceEditBackward->setKeySequence(defaultBackwardKey);
  });

  QObject::connect(ui->keySequenceEditLeft, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::leftKeyChanged);
  QObject::connect(ui->keySequenceEditLeft, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditLeft->keySequence().count() == 0)
      ui->keySequenceEditLeft->setKeySequence(defaultLeftKey);
  });

  QObject::connect(ui->keySequenceEditRight, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::rightKeyChanged);
  QObject::connect(ui->keySequenceEditRight, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditRight->keySequence().count() == 0)
      ui->keySequenceEditRight->setKeySequence(defaultRightKey);
  });

  QObject::connect(ui->keySequenceEditTurnLeft, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::leftTurnKeyChanged);
  QObject::connect(ui->keySequenceEditTurnLeft, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditTurnLeft->keySequence().count() == 0)
      ui->keySequenceEditTurnLeft->setKeySequence(defaultLeftTurnKey);
  });

  QObject::connect(ui->keySequenceEditTurnRight, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::rightTurnKeyChanged);
  QObject::connect(ui->keySequenceEditTurnRight, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditTurnRight->keySequence().count() == 0)
      ui->keySequenceEditTurnRight->setKeySequence(defaultRightTurnKey);
  });

  QObject::connect(ui->keySequenceEditUp, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::upKeyChanged);
  QObject::connect(ui->keySequenceEditUp, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditUp->keySequence().count() == 0)
      ui->keySequenceEditUp->setKeySequence(defaultUpKey);
  });

  QObject::connect(ui->keySequenceEditDown, &QKeySequenceEdit::keySequenceChanged, this,
                   &CameraConfigurationDialogue::downKeyChanged);
  QObject::connect(ui->keySequenceEditDown, &QKeySequenceEdit::editingFinished, [this]() {
    if (ui->keySequenceEditDown->keySequence().count() == 0)
      ui->keySequenceEditDown->setKeySequence(defaultDownKey);
  });

  QObject::connect(ui->pushButtonResetForward, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditForward->setKeySequence(defaultForwardKey); });
  // Only enable the reset button for the control if it is not
  // already the default
  ui->pushButtonResetForward->setEnabled(camera.getKeyForward() != defaultForwardKey);

  QObject::connect(ui->pushButtonResetBackward, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditBackward->setKeySequence(defaultBackwardKey); });
  ui->pushButtonResetBackward->setEnabled(camera.getKeyBackward() != defaultBackwardKey);

  QObject::connect(ui->pushButtonResetLeft, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditLeft->setKeySequence(defaultLeftKey); });
  ui->pushButtonResetLeft->setEnabled(camera.getKeyLeft() != defaultLeftKey);

  QObject::connect(ui->pushButtonResetRight, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditRight->setKeySequence(defaultRightKey); });
  ui->pushButtonResetRight->setEnabled(camera.getKeyRight() != defaultRightKey);

  QObject::connect(ui->pushButtonResetLeftTurn, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditTurnLeft->setKeySequence(defaultLeftTurnKey); });
  ui->pushButtonResetLeftTurn->setEnabled(camera.getKeyTurnLeft() != defaultLeftTurnKey);

  QObject::connect(ui->pushButtonResetRightTurn, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditTurnRight->setKeySequence(defaultRightTurnKey); });
  ui->pushButtonResetRightTurn->setEnabled(camera.getKeyTurnRight() != defaultRightTurnKey);

  QObject::connect(ui->pushButtonResetUp, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditUp->setKeySequence(defaultUpKey); });
  ui->pushButtonResetUp->setEnabled(camera.getKeyUp() != defaultUpKey);

  QObject::connect(ui->pushButtonResetDown, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditDown->setKeySequence(defaultDownKey); });
  ui->pushButtonResetDown->setEnabled(camera.getKeyDown() != defaultDownKey);

  QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
                   &CameraConfigurationDialogue::dialogueButtonClicked);
}

CameraConfigurationDialogue::~CameraConfigurationDialogue() {
  delete ui;
}

void CameraConfigurationDialogue::moveSpeedChanged(int value) {
  auto scaledValue = static_cast<float>(value) * 0.01f;
  camera.setMoveSpeed(scaledValue);
  ui->pushButtonResetMoveSpeed->setEnabled(value != defaultMoveSpeed);
}

void CameraConfigurationDialogue::keyboardTurnSpeedChanged(int value) {
  auto scaledValue = static_cast<float>(value) * 0.1f;
  camera.setTurnSpeed(scaledValue);
  ui->pushButtonResetKeyboardTurnSpeed->setEnabled(value != defaultKeyboardTurnSpeed);
}

void CameraConfigurationDialogue::mouseTurnSpeedChanged(int value) {
  auto scaledValue = static_cast<float>(value) * 0.1f;
  camera.setMouseTurnSpeed(scaledValue);
  ui->pushButtonResetMouseTurnSpeed->setEnabled(value != defaultMouseTurnSpeed);
}

void CameraConfigurationDialogue::fieldOfViewChanged(int value) {
  camera.setFieldOfView(static_cast<float>(value));
  ui->pushButtonResetFieldOfView->setEnabled(value != defaultFieldOfView);
  emit perspectiveUpdated();
}

void CameraConfigurationDialogue::useMouseControlsChanged(int value) {
  auto isChecked = value == Qt::CheckState::Checked;
  camera.useMouseControls(isChecked);

  ui->pushButtonResetMouseControlsEnabled->setEnabled(isChecked != defaultUseMouseControls);

  ui->sliderMouseTurnSpeed->setEnabled(isChecked);
}

void CameraConfigurationDialogue::forwardKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditForward->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetForward->setEnabled(value[0] != defaultForwardKey);
  camera.setKeyForward(value[0]);
}

void CameraConfigurationDialogue::backwardKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditBackward->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetBackward->setEnabled(value[0] != defaultBackwardKey);
  camera.setKeyBackward(value[0]);
}

void CameraConfigurationDialogue::leftKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditLeft->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetLeft->setEnabled(value[0] != defaultLeftKey);
  camera.setKeyLeft(value[0]);
}

void CameraConfigurationDialogue::rightKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditRight->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetRight->setEnabled(value[0] != defaultRightKey);
  camera.setKeyRight(value[0]);
}

void CameraConfigurationDialogue::leftTurnKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditTurnLeft->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetLeftTurn->setEnabled(value[0] != defaultLeftTurnKey);
  camera.setKeyTurnLeft(value[0]);
}

void CameraConfigurationDialogue::rightTurnKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditTurnRight->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetRightTurn->setEnabled(value[0] != defaultRightTurnKey);
  camera.setKeyTurnRight(value[0]);
}

void CameraConfigurationDialogue::upKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditUp->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  ui->pushButtonResetUp->setEnabled(value[0] != defaultUpKey);
  camera.setKeyUp(value[0]);
}

void CameraConfigurationDialogue::downKeyChanged(const QKeySequence &value) {
  if (value.count() > 1)
    ui->keySequenceEditDown->setKeySequence(value[0]);
  else if (value.count() == 0)
    return;

  ui->pushButtonResetDown->setEnabled(value[0] != defaultDownKey);
  camera.setKeyDown(value[0]);
}

void CameraConfigurationDialogue::dialogueButtonClicked(QAbstractButton *button) {
  auto standardButton = ui->buttonBox->standardButton(button);

  if (standardButton == QDialogButtonBox::RestoreDefaults) {
    ui->pushButtonResetMoveSpeed->click();
    ui->pushButtonResetMouseTurnSpeed->click();
    ui->pushButtonResetKeyboardTurnSpeed->click();
    ui->pushButtonResetFieldOfView->click();
    ui->pushButtonResetMouseControlsEnabled->click();
    ui->pushButtonResetForward->click();
    ui->pushButtonResetBackward->click();
    ui->pushButtonResetLeft->click();
    ui->pushButtonResetRight->click();
    ui->pushButtonResetLeftTurn->click();
    ui->pushButtonResetRightTurn->click();
    ui->pushButtonResetUp->click();
    ui->pushButtonResetDown->click();
    ui->checkBoxAllowCameraEvents->setChecked(defaultAllowCameraEvents);
  } else if (standardButton == QDialogButtonBox::Close) {
    accept();
  }
}

} // namespace visualization
