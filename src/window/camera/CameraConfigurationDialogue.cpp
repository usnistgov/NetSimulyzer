#include "CameraConfigurationDialogue.h"
#include "ui_CameraConfigurationDialogue.h"
#include <QObject>
#include <QSlider>
#include <iostream>

namespace visualization {

CameraConfigurationDialogue::CameraConfigurationDialogue(Camera &camera, QWidget *parent)
    : QDialog(parent), ui(new Ui::CameraConfigurationDialogue), camera(camera) {
  ui->setupUi(this);

  QObject::connect(ui->sliderMoveSpeed, &QSlider::valueChanged, this, &CameraConfigurationDialogue::moveSpeedChanged);
  ui->sliderMoveSpeed->setValue(static_cast<int>(camera.getMoveSpeed() * 100.0f));
  ui->labelMoveSpeedValue->setNum(camera.getMoveSpeed());
  QObject::connect(ui->pushButtonResetMoveSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderMoveSpeed->setValue(defaultMoveSpeed); });
  ui->pushButtonResetMoveSpeed->setEnabled(ui->sliderMoveSpeed->value() != defaultMoveSpeed);

  QObject::connect(ui->sliderTurnSpeed, &QSlider::valueChanged, this, &CameraConfigurationDialogue::turnSpeedChanged);
  ui->sliderTurnSpeed->setValue(static_cast<int>(camera.getTurnSpeed() * 10.0f));
  ui->labelTurnSpeedValue->setNum(camera.getTurnSpeed());
  QObject::connect(ui->pushButtonResetTurnSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderTurnSpeed->setValue(defaultTurnSpeed); });
  ui->pushButtonResetTurnSpeed->setEnabled(ui->sliderTurnSpeed->value() != defaultTurnSpeed);

  QObject::connect(ui->sliderFieldOfView, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::fieldOfViewChanged);
  ui->sliderFieldOfView->setValue(static_cast<int>(camera.getFieldOfView()));
  ui->labelFieldOfViewValue->setNum(camera.getFieldOfView());
  QObject::connect(ui->pushButtonResetFieldOfView, &QPushButton::clicked,
                   [this]() { ui->sliderFieldOfView->setValue(defaultFieldOfView); });
  ui->pushButtonResetFieldOfView->setEnabled(ui->sliderFieldOfView->value() != defaultFieldOfView);

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

  QObject::connect(ui->pushButtonResetForward, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditForward->setKeySequence(defaultForwardKey); });
  // Only enable the reset button for the control if it is not
  // already the default
  ui->pushButtonResetForward->setEnabled(camera.getKeyForward() != defaultForwardKey);

  QObject::connect(ui->pushButtonResetBackward, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditBackward->setKeySequence(defaultBackwardKey); });
  // Only enable the reset button for the control if it is not
  // already the default
  ui->pushButtonResetBackward->setEnabled(camera.getKeyBackward() != defaultBackwardKey);

  QObject::connect(ui->pushButtonResetLeft, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditLeft->setKeySequence(defaultLeftKey); });
  // Only enable the reset button for the control if it is not
  // already the default
  ui->pushButtonResetLeft->setEnabled(camera.getKeyLeft() != defaultLeftKey);

  QObject::connect(ui->pushButtonResetRight, &QPushButton::clicked,
                   [this]() { ui->keySequenceEditRight->setKeySequence(defaultRightKey); });
  // Only enable the reset button for the control if it is not
  // already the default
  ui->pushButtonResetRight->setEnabled(camera.getKeyRight() != defaultRightKey);

  QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
                   &CameraConfigurationDialogue::dialogueButtonClicked);
}

CameraConfigurationDialogue::~CameraConfigurationDialogue() {
  delete ui;
}

void CameraConfigurationDialogue::moveSpeedChanged(int value) {
  auto scaledValue = static_cast<float>(value) * 0.01f;
  ui->labelMoveSpeedValue->setNum(scaledValue);
  camera.setMoveSpeed(scaledValue);
  ui->pushButtonResetMoveSpeed->setEnabled(value != defaultMoveSpeed);
}

void CameraConfigurationDialogue::turnSpeedChanged(int value) {
  auto scaledValue = static_cast<float>(value) * 0.1f;
  ui->labelTurnSpeedValue->setNum(scaledValue);
  camera.setTurnSpeed(scaledValue);
  ui->pushButtonResetTurnSpeed->setEnabled(value != defaultTurnSpeed);
}

void CameraConfigurationDialogue::fieldOfViewChanged(int value) {
  ui->labelFieldOfViewValue->setNum(value);
  camera.setFieldOfView(static_cast<float>(value));
  ui->pushButtonResetFieldOfView->setEnabled(value != defaultFieldOfView);
  emit perspectiveUpdated();
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

void CameraConfigurationDialogue::dialogueButtonClicked(QAbstractButton *button) {
  auto standardButton = ui->buttonBox->standardButton(button);

  if (standardButton == QDialogButtonBox::RestoreDefaults) {
    ui->pushButtonResetMoveSpeed->click();
    ui->pushButtonResetTurnSpeed->click();
    ui->pushButtonResetFieldOfView->click();
    ui->pushButtonResetForward->click();
    ui->pushButtonResetBackward->click();
    ui->pushButtonResetLeft->click();
    ui->pushButtonResetRight->click();
    ui->checkBoxAllowCameraEvents->setChecked(defaultAllowCameraEvents);
  } else if (standardButton == QDialogButtonBox::Close) {
    accept();
  }
}

} // namespace visualization
