#include "CameraConfigurationDialogue.h"
#include "../controls/SingleKeySequenceEdit/SingleKeySequenceEdit.h"
#include <QObject>
#include <QSlider>
#include <iostream>
#include <ui_CameraConfigurationDialogue.h>

namespace visualization {

void CameraConfigurationDialogue::loadSettings() {
  using Key = SettingsManager::Key;
  const auto allowDefault = SettingsManager::RetrieveMode::AllowDefault;

  // Will pull the saved value or the default
  camera.setMoveSpeed(*settings.get<float>(Key::MoveSpeed, allowDefault));
  camera.setTurnSpeed(*settings.get<float>(Key::KeyboardTurnSpeed, allowDefault));
  camera.setMouseTurnSpeed(*settings.get<float>(Key::MouseTurnSpeed, allowDefault));
  camera.setFieldOfView(*settings.get<float>(Key::FieldOfView, allowDefault));
  camera.useMouseControls(*settings.get<bool>(Key::CameraMouseControls, allowDefault));
  camera.setKeyForward(*settings.get<int>(Key::CameraKeyForward, allowDefault));
  camera.setKeyBackward(*settings.get<int>(Key::CameraKeyBackwards, allowDefault));
  camera.setKeyLeft(*settings.get<int>(Key::CameraKeyLeft, allowDefault));
  camera.setKeyRight(*settings.get<int>(Key::CameraKeyRight, allowDefault));
  camera.setKeyTurnLeft(*settings.get<int>(Key::CameraKeyLeftTurn, allowDefault));
  camera.setKeyTurnRight(*settings.get<int>(Key::CameraKeyRightTurn, allowDefault));
  camera.setKeyUp(*settings.get<int>(Key::CameraKeyUp, allowDefault));
  camera.setKeyDown(*settings.get<int>(Key::CameraKeyDown, allowDefault));

  ui->sliderMoveSpeed->setValue(static_cast<int>(camera.getMoveSpeed() * moveSpeedScale));
  ui->sliderKeyboardTurnSpeed->setValue(static_cast<int>(camera.getTurnSpeed() * turnSpeedScale));
  ui->sliderMouseTurnSpeed->setValue(static_cast<int>(camera.getMouseTurnSpeed() * turnSpeedScale));
  ui->sliderFieldOfView->setValue(static_cast<int>(camera.getFieldOfView()));
  ui->checkBoxMouseControls->setChecked(camera.mouseControlsEnabled());
  ui->keyForward->setKeySequence(camera.getKeyForward());
  ui->keyBackward->setKeySequence(camera.getKeyBackward());
  ui->keyLeft->setKeySequence(camera.getKeyLeft());
  ui->keyRight->setKeySequence(camera.getKeyRight());
  ui->keyTurnLeft->setKeySequence(camera.getKeyTurnLeft());
  ui->keyTurnRight->setKeySequence(camera.getKeyTurnRight());
  ui->keyUp->setKeySequence(camera.getKeyUp());
  ui->keyDown->setKeySequence(camera.getKeyDown());
}

CameraConfigurationDialogue::CameraConfigurationDialogue(Camera &camera, QWidget *parent)
    : QDialog(parent), ui(new Ui::CameraConfigurationDialogue), camera(camera) {
  ui->setupUi(this);

  QObject::connect(ui->sliderMoveSpeed, &QSlider::valueChanged, this, &CameraConfigurationDialogue::moveSpeedChanged);
  QObject::connect(ui->pushButtonResetMoveSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderMoveSpeed->setValue(defaultMoveSpeed); });

  QObject::connect(ui->sliderKeyboardTurnSpeed, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::keyboardTurnSpeedChanged);

  QObject::connect(ui->pushButtonResetKeyboardTurnSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderKeyboardTurnSpeed->setValue(defaultKeyboardTurnSpeed); });

  QObject::connect(ui->sliderMouseTurnSpeed, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::mouseTurnSpeedChanged);
  QObject::connect(ui->pushButtonResetMouseTurnSpeed, &QPushButton::clicked,
                   [this]() { ui->sliderMouseTurnSpeed->setValue(defaultMouseTurnSpeed); });

  QObject::connect(ui->sliderFieldOfView, &QSlider::valueChanged, this,
                   &CameraConfigurationDialogue::fieldOfViewChanged);
  QObject::connect(ui->pushButtonResetFieldOfView, &QPushButton::clicked,
                   [this]() { ui->sliderFieldOfView->setValue(defaultFieldOfView); });

  QObject::connect(ui->checkBoxMouseControls, &QCheckBox::stateChanged, this,
                   &CameraConfigurationDialogue::useMouseControlsChanged);

  QObject::connect(ui->pushButtonResetMouseControlsEnabled, &QPushButton::clicked,
                   [this]() { ui->checkBoxMouseControls->setChecked(defaultUseMouseControls); });

  QObject::connect(ui->keyForward, &QKeySequenceEdit::editingFinished, this,
                   &CameraConfigurationDialogue::forwardKeyChanged);
  ui->keyForward->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyForward));

  QObject::connect(ui->keyBackward, &QKeySequenceEdit::editingFinished, this,
                   &CameraConfigurationDialogue::backwardKeyChanged);
  ui->keyBackward->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyBackwards));

  QObject::connect(ui->keyLeft, &QKeySequenceEdit::editingFinished, this, &CameraConfigurationDialogue::leftKeyChanged);
  ui->keyLeft->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyLeft));

  QObject::connect(ui->keyRight, &QKeySequenceEdit::editingFinished, this,
                   &CameraConfigurationDialogue::rightKeyChanged);
  ui->keyRight->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyRight));

  QObject::connect(ui->keyTurnLeft, &QKeySequenceEdit::editingFinished, this,
                   &CameraConfigurationDialogue::leftTurnKeyChanged);
  ui->keyTurnLeft->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyLeftTurn));

  QObject::connect(ui->keyTurnRight, &QKeySequenceEdit::editingFinished, this,
                   &CameraConfigurationDialogue::rightTurnKeyChanged);
  ui->keyTurnRight->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyRightTurn));

  QObject::connect(ui->keyUp, &QKeySequenceEdit::editingFinished, this, &CameraConfigurationDialogue::upKeyChanged);
  ui->keyUp->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyUp));

  QObject::connect(ui->keyDown, &QKeySequenceEdit::editingFinished, this, &CameraConfigurationDialogue::downKeyChanged);
  ui->keyDown->setDefaultKey(settings.getDefault<int>(SettingsManager::Key::CameraKeyDown));

  QObject::connect(ui->pushButtonResetForward, &QPushButton::clicked, ui->keyForward,
                   &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetBackward, &QPushButton::clicked, ui->keyBackward,
                   &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetLeft, &QPushButton::clicked, ui->keyLeft, &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetRight, &QPushButton::clicked, ui->keyRight, &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetLeftTurn, &QPushButton::clicked, ui->keyTurnLeft,
                   &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetRightTurn, &QPushButton::clicked, ui->keyTurnRight,
                   &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetUp, &QPushButton::clicked, ui->keyUp, &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->pushButtonResetDown, &QPushButton::clicked, ui->keyDown, &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui->buttonBox, &QDialogButtonBox::clicked, this,
                   &CameraConfigurationDialogue::dialogueButtonClicked);

  loadSettings();

  ui->pushButtonResetMoveSpeed->setEnabled(ui->sliderMoveSpeed->value() != defaultMoveSpeed);
  ui->pushButtonResetKeyboardTurnSpeed->setEnabled(ui->sliderKeyboardTurnSpeed->value() != defaultKeyboardTurnSpeed);
  ui->pushButtonResetMouseTurnSpeed->setEnabled(ui->sliderMouseTurnSpeed->value() != defaultMouseTurnSpeed);
  ui->pushButtonResetFieldOfView->setEnabled(ui->sliderFieldOfView->value() != defaultFieldOfView);
  ui->pushButtonResetMouseControlsEnabled->setEnabled(ui->checkBoxMouseControls->isChecked() !=
                                                      defaultUseMouseControls);
  ui->pushButtonResetForward->setEnabled(camera.getKeyForward() != *ui->keyForward->getDefaultKey());
  ui->pushButtonResetBackward->setEnabled(camera.getKeyBackward() != *ui->keyBackward->getDefaultKey());
  ui->pushButtonResetLeft->setEnabled(camera.getKeyLeft() != *ui->keyLeft->getDefaultKey());
  ui->pushButtonResetRight->setEnabled(camera.getKeyRight() != *ui->keyRight->getDefaultKey());
  ui->pushButtonResetLeftTurn->setEnabled(camera.getKeyTurnLeft() != *ui->keyTurnLeft->getDefaultKey());
  ui->pushButtonResetRightTurn->setEnabled(camera.getKeyTurnRight() != *ui->keyTurnRight->getDefaultKey());
  ui->pushButtonResetUp->setEnabled(camera.getKeyUp() != *ui->keyUp->getDefaultKey());
  ui->pushButtonResetDown->setEnabled(camera.getKeyDown() != *ui->keyDown->getDefaultKey());
}

CameraConfigurationDialogue::~CameraConfigurationDialogue() {
  delete ui;
}

void CameraConfigurationDialogue::moveSpeedChanged(int value) {
  camera.setMoveSpeed(static_cast<float>(value) / moveSpeedScale);
  ui->pushButtonResetMoveSpeed->setEnabled(value != defaultMoveSpeed);
}

void CameraConfigurationDialogue::keyboardTurnSpeedChanged(int value) {
  camera.setTurnSpeed(static_cast<float>(value) / turnSpeedScale);
  ui->pushButtonResetKeyboardTurnSpeed->setEnabled(value != defaultKeyboardTurnSpeed);
}

void CameraConfigurationDialogue::mouseTurnSpeedChanged(int value) {
  camera.setMouseTurnSpeed(static_cast<float>(value) / turnSpeedScale);
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

void CameraConfigurationDialogue::forwardKeyChanged() {
  auto value = ui->keyForward->keySequence();

  // Only enable the reset button for the control if it is not
  // being set to the default
  ui->pushButtonResetForward->setEnabled(value[0] != *ui->keyForward->getDefaultKey());
  camera.setKeyForward(value[0]);
}

void CameraConfigurationDialogue::backwardKeyChanged() {
  auto value = ui->keyBackward->keySequence();

  ui->pushButtonResetBackward->setEnabled(value[0] != *ui->keyBackward->getDefaultKey());
  camera.setKeyBackward(value[0]);
}

void CameraConfigurationDialogue::leftKeyChanged() {
  auto value = ui->keyLeft->keySequence();

  ui->pushButtonResetLeft->setEnabled(value[0] != *ui->keyLeft->getDefaultKey());
  camera.setKeyLeft(value[0]);
}

void CameraConfigurationDialogue::rightKeyChanged() {
  auto value = ui->keyRight->keySequence();

  ui->pushButtonResetRight->setEnabled(value[0] != *ui->keyRight->getDefaultKey());
  camera.setKeyRight(value[0]);
}

void CameraConfigurationDialogue::leftTurnKeyChanged() {
  auto value = ui->keyTurnLeft->keySequence();

  ui->pushButtonResetLeftTurn->setEnabled(value[0] != *ui->keyTurnLeft->getDefaultKey());
  camera.setKeyTurnLeft(value[0]);
}

void CameraConfigurationDialogue::rightTurnKeyChanged() {
  auto value = ui->keyTurnRight->keySequence();

  ui->pushButtonResetRightTurn->setEnabled(value[0] != *ui->keyTurnRight->getDefaultKey());
  camera.setKeyTurnRight(value[0]);
}

void CameraConfigurationDialogue::upKeyChanged() {
  auto value = ui->keyUp->keySequence();

  ui->pushButtonResetUp->setEnabled(value[0] != *ui->keyUp->getDefaultKey());
  camera.setKeyUp(value[0]);
}

void CameraConfigurationDialogue::downKeyChanged() {
  auto value = ui->keyDown->keySequence();

  ui->pushButtonResetDown->setEnabled(value[0] != *ui->keyDown->getDefaultKey());
  camera.setKeyDown(value[0]);
}

void CameraConfigurationDialogue::dialogueButtonClicked(QAbstractButton *button) {
  using Key = SettingsManager::Key;

  switch (ui->buttonBox->standardButton(button)) {
  case QDialogButtonBox::RestoreDefaults:
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
    break;
  case QDialogButtonBox::Save:
    settings.set(Key::MoveSpeed, camera.getMoveSpeed());
    settings.set(Key::KeyboardTurnSpeed, camera.getTurnSpeed());
    settings.set(Key::MouseTurnSpeed, camera.getMouseTurnSpeed());
    settings.set(Key::FieldOfView, camera.getFieldOfView());
    settings.set(Key::CameraMouseControls, camera.mouseControlsEnabled());
    settings.set(Key::CameraKeyForward, camera.getKeyForward());
    settings.set(Key::CameraKeyBackwards, camera.getKeyBackward());
    settings.set(Key::CameraKeyLeft, camera.getKeyLeft());
    settings.set(Key::CameraKeyRight, camera.getKeyRight());
    settings.set(Key::CameraKeyLeftTurn, camera.getKeyTurnLeft());
    settings.set(Key::CameraKeyRightTurn, camera.getKeyTurnRight());
    settings.set(Key::CameraKeyUp, camera.getKeyUp());
    settings.set(Key::CameraKeyDown, camera.getKeyDown());
    accept();
    break;
  case QDialogButtonBox::Discard:
    loadSettings();
    reject();
    break;
  default:
    break;
  }
}

} // namespace visualization
