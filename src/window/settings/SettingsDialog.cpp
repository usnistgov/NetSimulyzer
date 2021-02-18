#include "SettingsDialog.h"
#include "src/settings/SettingsManager.h"
#include "src/window/util/file-operations.h"
#include "ui_SettingsDialog.h"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QtWidgets/QDialogButtonBox>

namespace netsimulyzer {

void SettingsDialog::loadSettings() {
  using Key = SettingsManager::Key;

  ui.sliderMoveSpeed->setValue(static_cast<int>(*settings.get<float>(Key::MoveSpeed) * moveSpeedScale));
  ui.sliderKeyboardTurnSpeed->setValue(static_cast<int>(*settings.get<float>(Key::KeyboardTurnSpeed) * turnSpeedScale));
  ui.sliderMouseTurnSpeed->setValue(static_cast<int>(*settings.get<float>(Key::MouseTurnSpeed) * turnSpeedScale));

  ui.sliderFieldOfView->setValue(static_cast<int>(*settings.get<float>(Key::FieldOfView)));

  ui.keyForward->setKeySequence(*settings.get<int>(Key::CameraKeyForward));
  ui.keyBackward->setKeySequence(*settings.get<int>(Key::CameraKeyBackwards));
  ui.keyLeft->setKeySequence(*settings.get<int>(Key::CameraKeyLeft));
  ui.keyRight->setKeySequence(*settings.get<int>(Key::CameraKeyRight));
  ui.keyTurnLeft->setKeySequence(*settings.get<int>(Key::CameraKeyLeftTurn));
  ui.keyTurnRight->setKeySequence(*settings.get<int>(Key::CameraKeyRightTurn));
  ui.keyUp->setKeySequence(*settings.get<int>(Key::CameraKeyUp));
  ui.keyDown->setKeySequence(*settings.get<int>(Key::CameraKeyDown));

  const auto samples = *settings.get<int>(Key::NumberSamples);
  ui.comboSamples->setCurrentIndex(ui.comboSamples->findData(samples));

  ui.checkBoxSkybox->setChecked(settings.get<bool>(Key::RenderSkybox).value());

  const auto buildingMode = settings.get<SettingsManager::BuildingRenderMode>(Key::RenderBuildingMode).value();
  ui.comboBuildingRender->setCurrentIndex(ui.comboBuildingRender->findData(static_cast<int>(buildingMode)));

  ui.keyPlay->setKeySequence(*settings.get<int>(Key::SceneKeyPlay));

  // Time Step is session based (so no setting to load)

  ui.lineEditResource->setText(resourcePath);
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);
  ui.comboSamples->addItem("0 (Off)", 0);
  ui.comboSamples->addItem("2", 2);
  ui.comboSamples->addItem("4", 4);
  ui.comboSamples->addItem("8", 8);
  ui.comboSamples->addItem("16", 16);

  ui.comboBuildingRender->addItem("Transparent", static_cast<int>(SettingsManager::BuildingRenderMode::Transparent));
  ui.comboBuildingRender->addItem("Opaque", static_cast<int>(SettingsManager::BuildingRenderMode::Opaque));

  using Key = SettingsManager::Key;
  ui.keyForward->setDefaultKey(settings.getDefault<int>(Key::CameraKeyForward));
  ui.keyBackward->setDefaultKey(settings.getDefault<int>(Key::CameraKeyBackwards));
  ui.keyLeft->setDefaultKey(settings.getDefault<int>(Key::CameraKeyLeft));
  ui.keyRight->setDefaultKey(settings.getDefault<int>(Key::CameraKeyRight));
  ui.keyTurnLeft->setDefaultKey(settings.getDefault<int>(Key::CameraKeyLeftTurn));
  ui.keyTurnRight->setDefaultKey(settings.getDefault<int>(Key::CameraKeyRightTurn));
  ui.keyUp->setDefaultKey(settings.getDefault<int>(Key::CameraKeyUp));
  ui.keyDown->setDefaultKey(settings.getDefault<int>(Key::CameraKeyDown));
  ui.keyPlay->setDefaultKey(settings.getDefault<int>(Key::SceneKeyPlay));

  loadSettings();

  QObject::connect(ui.buttonResetMoveSpeed, &QPushButton::clicked, this, &SettingsDialog::defaultMoveSpeed);
  QObject::connect(ui.buttonResetKeyboardTurnSpeed, &QPushButton::clicked, this,
                   &SettingsDialog::defaultKeyboardTurnSpeed);
  QObject::connect(ui.buttonResetMouseTurnSpeed, &QPushButton::clicked, this, &SettingsDialog::defaultMouseTurnSpeed);
  QObject::connect(ui.buttonResetFieldOfView, &QPushButton::clicked, this, &SettingsDialog::defaultFieldOfView);

  QObject::connect(ui.buttonResetForward, &QPushButton::clicked, ui.keyForward, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetBackward, &QPushButton::clicked, ui.keyBackward, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetLeft, &QPushButton::clicked, ui.keyLeft, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetRight, &QPushButton::clicked, ui.keyRight, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetLeftTurn, &QPushButton::clicked, ui.keyTurnLeft, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetRightTurn, &QPushButton::clicked, ui.keyTurnRight, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetUp, &QPushButton::clicked, ui.keyUp, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetDown, &QPushButton::clicked, ui.keyDown, &SingleKeySequenceEdit::setDefault);

  QObject::connect(ui.buttonResetSkybox, &QPushButton::clicked, this, &SettingsDialog::defaultEnableSkybox);
  QObject::connect(ui.buttonResetSamples, &QPushButton::clicked, this, &SettingsDialog::defaultSamples);
  QObject::connect(ui.buttonResetBuildingRender, &QPushButton::clicked, this, &SettingsDialog::defaultBuildingEffect);

  QObject::connect(ui.buttonResetPlay, &QPushButton::clicked, ui.keyPlay, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetTimeStep, &QPushButton::clicked, this, &SettingsDialog::defaultTimeStep);

  QObject::connect(ui.buttonResource, &QPushButton::clicked, this, &SettingsDialog::selectResourcePath);

  QObject::connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::dialogueButtonClicked);
}

void SettingsDialog::setTimeStep(double value) {
  ui.spinTimeStep->setValue(static_cast<int>(value));

  passedTimeStep = value;
}

void SettingsDialog::dialogueButtonClicked(QAbstractButton *button) {
  switch (ui.buttonBox->standardButton(button)) {
  case QDialogButtonBox::RestoreDefaults:
    ui.buttonResetMoveSpeed->click();
    ui.buttonResetKeyboardTurnSpeed->click();
    ui.buttonResetMouseTurnSpeed->click();
    ui.buttonResetFieldOfView->click();

    ui.buttonResetForward->click();
    ui.buttonResetBackward->click();
    ui.buttonResetLeft->click();
    ui.buttonResetRight->click();
    ui.buttonResetLeftTurn->click();
    ui.buttonResetRightTurn->click();
    ui.buttonResetUp->click();
    ui.buttonResetDown->click();

    ui.buttonResetSkybox->click();
    ui.buttonResetSamples->click();
    ui.buttonResetBuildingRender->click();

    ui.buttonResetPlay->click();
    break;
  case QDialogButtonBox::Save: {
    bool requiresRestart = false;
    using Key = SettingsManager::Key;

    // Camera
    const auto moveSpeed = static_cast<float>(ui.sliderMoveSpeed->value()) / moveSpeedScale;
    settings.set(Key::MoveSpeed, moveSpeed);
    emit moveSpeedChanged(moveSpeed);

    const auto keyboardTurnSpeed = static_cast<float>(ui.sliderKeyboardTurnSpeed->value()) / turnSpeedScale;
    settings.set(Key::KeyboardTurnSpeed, keyboardTurnSpeed);
    emit keyboardTurnSpeedChanged(keyboardTurnSpeed);

    const auto mouseTurnSpeed = static_cast<float>(ui.sliderMouseTurnSpeed->value()) / turnSpeedScale;
    settings.set(Key::MouseTurnSpeed, mouseTurnSpeed);
    emit mouseTurnSpeedChanged(mouseTurnSpeed);

    const auto fov = static_cast<float>(ui.sliderFieldOfView->value());
    settings.set(Key::FieldOfView, fov);
    emit fieldOfViewChanged(fov);

    const auto forwardKey = ui.keyForward->keySequence()[0];
    if (forwardKey != settings.get<int>(Key::CameraKeyForward).value()) {
      settings.set(Key::CameraKeyForward, forwardKey);
      emit forwardKeyChanged(forwardKey);
    }

    const auto backwardKey = ui.keyBackward->keySequence()[0];
    if (backwardKey != settings.get<int>(Key::CameraKeyBackwards).value()) {
      settings.set(Key::CameraKeyBackwards, backwardKey);
      emit backwardKeyChanged(backwardKey);
    }

    const auto leftKey = ui.keyLeft->keySequence()[0];
    if (leftKey != settings.get<int>(Key::CameraKeyLeft).value()) {
      settings.set(Key::CameraKeyLeft, leftKey);
      emit leftKeyChanged(leftKey);
    }

    const auto rightKey = ui.keyRight->keySequence()[0];
    if (rightKey != settings.get<int>(Key::CameraKeyRight).value()) {
      settings.set(Key::CameraKeyRight, rightKey);
      emit rightKeyChanged(rightKey);
    }

    const auto turnLeftKey = ui.keyTurnLeft->keySequence()[0];
    if (turnLeftKey != settings.get<int>(Key::CameraKeyLeftTurn).value()) {
      settings.set(Key::CameraKeyLeftTurn, turnLeftKey);
      emit turnLeftKeyChanged(turnLeftKey);
    }

    const auto turnRightKey = ui.keyTurnRight->keySequence()[0];
    if (turnRightKey != settings.get<int>(Key::CameraKeyRightTurn).value()) {
      settings.set(Key::CameraKeyRightTurn, turnRightKey);
      emit turnRightKeyChanged(turnRightKey);
    }

    const auto upKey = ui.keyUp->keySequence()[0];
    if (upKey != settings.get<int>(Key::CameraKeyUp).value()) {
      settings.set(Key::CameraKeyUp, upKey);
      emit upKeyChanged(upKey);
    }

    const auto downKey = ui.keyDown->keySequence()[0];
    if (downKey != settings.get<int>(Key::CameraKeyDown).value()) {
      settings.set(Key::CameraKeyDown, downKey);
      emit downKeyChanged(downKey);
    }

    // Graphics

    const auto samples = ui.comboSamples->currentData().toInt();
    if (samples != settings.get<int>(Key::NumberSamples)) {
      settings.set(Key::NumberSamples, samples);
      requiresRestart = true;
    }

    auto enableSkybox = ui.checkBoxSkybox->isChecked();
    if (enableSkybox != settings.get<bool>(Key::RenderSkybox)) {
      settings.set(Key::RenderSkybox, enableSkybox);
      emit renderSkyboxChanged(enableSkybox);
    }

    auto buildingRenderMode = SettingsManager::BuildingRenderModeFromInt(ui.comboBuildingRender->currentData().toInt());
    if (buildingRenderMode != settings.get<SettingsManager::BuildingRenderMode>(Key::RenderBuildingMode).value()) {
      settings.set(Key::RenderBuildingMode, buildingRenderMode);
      emit buildingRenderModeChanged(static_cast<int>(buildingRenderMode));
    }

    // Playback

    const auto playKey = ui.keyPlay->keySequence()[0];
    if (playKey != settings.get<int>(Key::SceneKeyPlay).value()) {
      settings.set(Key::SceneKeyPlay, playKey);
      emit playKeyChanged(playKey);
    }

    auto oldResourcePath = settings.get<QString>(Key::ResourcePath);
    if (resourcePath != oldResourcePath) {
      settings.set(Key::ResourcePath, resourcePath);
      emit resourcePathChanged(resourcePath);
    }

    // Since we don't have a setting for this, just assume it's always different
    emit timeStepSet(static_cast<double>(ui.spinTimeStep->value()));

    settings.sync();

    if (requiresRestart)
      QMessageBox::warning(this, "Settings Require Restart",
                           "A restart is required for some of the changed settings to take effect.");

    accept();
  } break;
  case QDialogButtonBox::Discard:
    loadSettings();
    reject();
    break;
  default:
    break;
  }
}

void SettingsDialog::defaultMoveSpeed() {
  ui.sliderMoveSpeed->setValue(
      static_cast<int>(settings.getDefault<float>(SettingsManager::Key::MoveSpeed) * moveSpeedScale));
}

void SettingsDialog::defaultKeyboardTurnSpeed() {
  ui.sliderKeyboardTurnSpeed->setValue(
      static_cast<int>(settings.getDefault<float>(SettingsManager::Key::KeyboardTurnSpeed) * turnSpeedScale));
}

void SettingsDialog::defaultMouseTurnSpeed() {
  ui.sliderMouseTurnSpeed->setValue(
      static_cast<int>(settings.getDefault<float>(SettingsManager::Key::MouseTurnSpeed) * turnSpeedScale));
}

void SettingsDialog::defaultFieldOfView() {
  ui.sliderFieldOfView->setValue(static_cast<int>(settings.getDefault<float>(SettingsManager::Key::FieldOfView)));
}

void SettingsDialog::defaultSamples() {
  ui.comboSamples->setCurrentIndex(
      ui.comboSamples->findData(settings.getDefault<int>(SettingsManager::Key::NumberSamples)));
}

void SettingsDialog::defaultEnableSkybox() {
  ui.checkBoxSkybox->setChecked(settings.getDefault<bool>(SettingsManager::Key::RenderSkybox));
}

void SettingsDialog::defaultBuildingEffect() {
  const auto defaultBuildingMode =
      settings.getDefault<SettingsManager::BuildingRenderMode>(SettingsManager::Key::RenderBuildingMode);
  ui.comboBuildingRender->setCurrentIndex(ui.comboBuildingRender->findData(static_cast<int>(defaultBuildingMode)));
}

void SettingsDialog::defaultTimeStep() {
  ui.spinTimeStep->setValue(static_cast<int>(passedTimeStep));
}

void SettingsDialog::selectResourcePath() {
  auto selected = netsimulyzer::getExistingDirectory("Select 'resources' Directory");
  if (selected.isEmpty())
    return;

  QFileInfo info{selected};
  if (!info.isDir()) {
    // Should be covered by `getExistingDirectory`, but just in case...
    QMessageBox::critical(this, "Not a Directory", "The selected file: \"" + selected + "\" is not a directory.");
    return;
  } else if (!info.exists()) {
    QMessageBox::critical(this, "Directory Does Not Exist",
                          "The selected directory: \"" + selected + "\" does not exist!");
    return;
  } else if (!info.isReadable()) {
    QMessageBox::critical(this, "Directory Unreadable", "The selected directory: \"" + selected + "\" is unreadable!");
    return;
  }

  resourcePath = info.absoluteFilePath();
  if (!resourcePath.endsWith('/'))
    resourcePath.append('/');

  ui.lineEditResource->setText(resourcePath);
}

} // namespace netsimulyzer
