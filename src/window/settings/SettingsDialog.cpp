#include "SettingsDialog.h"
#include "src/conversion.h"
#include "src/settings/SettingsManager.h"
#include "src/util/palette.h"
#include "src/window/util/file-operations.h"
#include "ui_SettingsDialog.h"
#include <QApplication>
#include <QColorDialog>
#include <QComboBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QtWidgets/QDialogButtonBox>

namespace netsimulyzer {

void SettingsDialog::loadSettings() {
  using Key = SettingsManager::Key;

  const auto windowTheme = settings.get<SettingsManager::WindowTheme>(Key::WindowTheme).value();
  ui.comboTheme->setCurrentIndex(ui.comboTheme->findData(static_cast<int>(windowTheme)));

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

  const auto cameraType = settings.get<SettingsManager::CameraType>(Key::RenderCameraType).value();
  ui.comboCameraType->setCurrentIndex(ui.comboCameraType->findData(static_cast<int>(cameraType)));

  ui.checkBoxScaleMoveSpeed->setChecked(settings.get<bool>(Key::AutoScaleMoveSpeed).value());

  ui.checkBoxSkybox->setChecked(settings.get<bool>(Key::RenderSkybox).value());
  ui.checkBoxFloor->setChecked(settings.get<bool>(Key::RenderFloor).value());

  const auto backgroundColor = *settings.get<SettingsManager::BackgroundColor>(Key::RenderBackgroundColor);
  ui.comboBackgroundColor->setCurrentIndex(static_cast<int>(backgroundColor));

  customBackgroundColor = settings.get<QColor>(SettingsManager::Key::RenderBackgroundColorCustom).value();
  ui.comboBackgroundColor->setItemData(2, customBackgroundColor, Qt::DecorationRole);

  const auto buildingMode = settings.get<SettingsManager::BuildingRenderMode>(Key::RenderBuildingMode).value();
  ui.comboBuildingRender->setCurrentIndex(ui.comboBuildingRender->findData(static_cast<int>(buildingMode)));

  const auto chartDropdownSortOrder =
      settings.get<SettingsManager::ChartDropdownSortOrder>(Key::ChartDropdownSortOrder).value();
  ui.comboSortOrder->setCurrentIndex(ui.comboSortOrder->findData(static_cast<int>(chartDropdownSortOrder)));

  ui.checkBoxBuildingOutlines->setChecked(settings.get<bool>(Key::RenderBuildingOutlines).value());

  ui.comboGridSize->setCurrentIndex(ui.comboGridSize->findData(settings.get<int>(Key::RenderGridStep).value()));
  ui.checkBoxShowGrid->setChecked(settings.get<bool>(Key::RenderGrid).value());

  const auto timeStepUnit = settings.get<SettingsManager::TimeUnit>(Key::PlaybackTimeStepUnit).value();
  ui.comboTimeStepUnit->setCurrentIndex(ui.comboTimeStepUnit->findData(static_cast<int>(timeStepUnit)));
  setStepSpinSuffix(timeStepUnit);
  const auto timeStepNs = settings.get<int>(Key::PlaybackTimeStepPreference).value();
  switch (timeStepUnit) {
  case SettingsManager::TimeUnit::Milliseconds:
    ui.spinTimeStep->setValue(toMilliseconds(timeStepNs));
    break;
  case SettingsManager::TimeUnit::Microseconds:
    ui.spinTimeStep->setValue(toMicroseconds(timeStepNs));
    break;
  case SettingsManager::TimeUnit::Nanoseconds:
    ui.spinTimeStep->setValue(timeStepNs);
    break;
  }

  const auto motionTrailMode = settings.get<SettingsManager::MotionTrailRenderMode>(Key::RenderMotionTrails).value();
  ui.comboMotionTrailRender->setCurrentIndex(ui.comboMotionTrailRender->findData(static_cast<int>(motionTrailMode)));
  ui.sliderTrailLength->setValue(settings.get<int>(Key::RenderMotionTrailLength).value());

  const auto labelRenderMode = settings.get<SettingsManager::LabelRenderMode>(Key::RenderLabels).value();
  ui.comboLabelRender->setCurrentIndex(ui.comboLabelRender->findData(static_cast<int>(labelRenderMode)));

  ui.sliderLabelScale->setValue(static_cast<int>(settings.get<float>(Key::RenderLabelScale).value() * labelScaleScale));

  ui.keyPlay->setKeySequence(*settings.get<int>(Key::SceneKeyPlay));

  ui.lineEditResource->setText(resourcePath);
}

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);
  using WindowTheme = SettingsManager::WindowTheme;
  ui.comboTheme->addItem("Dark", static_cast<int>(WindowTheme::Dark));
  ui.comboTheme->addItem("Light", static_cast<int>(WindowTheme::Light));
  ui.comboTheme->addItem("Native", static_cast<int>(WindowTheme::Native));

  ui.comboSamples->addItem("0 (Off)", 0);
  ui.comboSamples->addItem("2", 2);
  ui.comboSamples->addItem("4", 4);
  ui.comboSamples->addItem("8", 8);
  ui.comboSamples->addItem("16", 16);

  ui.comboCameraType->addItem("First Person", static_cast<int>(SettingsManager::CameraType::FirstPerson));
  ui.comboCameraType->addItem("Arc Ball", static_cast<int>(SettingsManager::CameraType::ArcBall));

  ui.comboBackgroundColor->addItem("Black", static_cast<int>(SettingsManager::BackgroundColor::Black));
  ui.comboBackgroundColor->setItemData(0, palette::Black, Qt::DecorationRole);

  ui.comboBackgroundColor->addItem("White", static_cast<int>(SettingsManager::BackgroundColor::White));
  ui.comboBackgroundColor->setItemData(1, palette::White, Qt::DecorationRole);

  ui.comboBackgroundColor->addItem("Custom", static_cast<int>(SettingsManager::BackgroundColor::Custom));
  ui.comboBackgroundColor->setItemData(2, customBackgroundColor, Qt::DecorationRole);

  ui.comboBuildingRender->addItem("Transparent", static_cast<int>(SettingsManager::BuildingRenderMode::Transparent));
  ui.comboBuildingRender->addItem("Opaque", static_cast<int>(SettingsManager::BuildingRenderMode::Opaque));

  QObject::connect(ui.buttonSetCustomBackgroundColor, &QPushButton::clicked, [this]() {
    const auto userColor = QColorDialog::getColor(customBackgroundColor, this, "Select a Background Color");
    if (!userColor.isValid())
      return;

    customBackgroundColor = userColor;
    ui.comboBackgroundColor->setItemData(2, customBackgroundColor, Qt::DecorationRole);
  });

  using SortOrder = SettingsManager::ChartDropdownSortOrder;
  ui.comboSortOrder->addItem("Alphabetical", static_cast<int>(SortOrder::Alphabetical));
  ui.comboSortOrder->addItem("Type", static_cast<int>(SortOrder::Type));
  ui.comboSortOrder->addItem("Id", static_cast<int>(SortOrder::Id));
  ui.comboSortOrder->addItem("None", static_cast<int>(SortOrder::None));

  ui.comboGridSize->addItem("1", 1);
  ui.comboGridSize->addItem("5", 5);
  ui.comboGridSize->addItem("10", 10);

  using MotionTrailRenderMode = SettingsManager::MotionTrailRenderMode;
  ui.comboMotionTrailRender->addItem("Always", static_cast<int>(MotionTrailRenderMode::Always));
  ui.comboMotionTrailRender->addItem("Enabled Only", static_cast<int>(MotionTrailRenderMode::EnabledOnly));
  ui.comboMotionTrailRender->addItem("Never", static_cast<int>(MotionTrailRenderMode::Never));

  using LabelRenderMode = SettingsManager::LabelRenderMode;
  ui.comboLabelRender->addItem("Always", static_cast<int>(LabelRenderMode::Always));
  ui.comboLabelRender->addItem("Enabled Only", static_cast<int>(LabelRenderMode::EnabledOnly));
  ui.comboLabelRender->addItem("Never", static_cast<int>(LabelRenderMode::Never));

  using TimeUnit = SettingsManager::TimeUnit;
  ui.comboTimeStepUnit->addItem("ns", static_cast<int>(TimeUnit::Nanoseconds));
  ui.comboTimeStepUnit->addItem("µs", static_cast<int>(TimeUnit::Microseconds));
  ui.comboTimeStepUnit->addItem("ms", static_cast<int>(TimeUnit::Milliseconds));

  QObject::connect(ui.comboTimeStepUnit, qOverload<int>(&QComboBox::currentIndexChanged), [this](int /* index */) {
    setStepSpinSuffix(SettingsManager::TimeUnitFromInt(ui.comboTimeStepUnit->currentData().toInt()));
  });

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

  ui.sliderMoveSpeed->setEnabled(!ui.checkBoxScaleMoveSpeed->isChecked());
  QObject::connect(ui.checkBoxScaleMoveSpeed, &QCheckBox::stateChanged, this,
                   &SettingsDialog::autoscaleMoveSpeedToggled);

  QObject::connect(ui.buttonResetTheme, &QPushButton::clicked, this, &SettingsDialog::defaultWindowTheme);
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

  QObject::connect(ui.buttonResetSortOrder, &QPushButton::clicked, this, &SettingsDialog::defaultChartSortOrder);

  QObject::connect(ui.buttonResetSkybox, &QPushButton::clicked, this, &SettingsDialog::defaultEnableSkybox);
  QObject::connect(ui.buttonResetFloor, &QPushButton::clicked, this, &SettingsDialog::defaultEnableFloor);
  QObject::connect(ui.buttonResetCameraType, &QPushButton::clicked, this, &SettingsDialog::defaultCameraType);
  QObject::connect(ui.buttonResetMoveSpeedScale, &QPushButton::clicked, this,
                   &SettingsDialog::defaultAutoscaleMoveSpeed);
  QObject::connect(ui.buttonResetBackgroundColor, &QPushButton::clicked, this, &SettingsDialog::defaultBackgroundColor);
  QObject::connect(ui.buttonResetSamples, &QPushButton::clicked, this, &SettingsDialog::defaultSamples);
  QObject::connect(ui.buttonResetBuildingRender, &QPushButton::clicked, this, &SettingsDialog::defaultBuildingEffect);
  QObject::connect(ui.buttonResetBuildingOutlines, &QPushButton::clicked, this,
                   &SettingsDialog::defaultBuildingOutlines);
  QObject::connect(ui.buttonResetShowGrid, &QPushButton::clicked, this, &SettingsDialog::defaultShowGrid);
  QObject::connect(ui.buttonResetGridSize, &QPushButton::clicked, this, &SettingsDialog::defaultGridStep);
  QObject::connect(ui.buttonResetTrails, &QPushButton::clicked, this, &SettingsDialog::defaultShowTrails);
  QObject::connect(ui.buttonResetTrailLength, &QPushButton::clicked, this, &SettingsDialog::defaultTrailsLength);
  QObject::connect(ui.buttonResetShowLabels, &QPushButton::clicked, this, &SettingsDialog::defaultShowLabels);
  QObject::connect(ui.buttonResetLabelScale, &QPushButton::clicked, this, &SettingsDialog::defaultLabelScale);

  QObject::connect(ui.buttonResetPlay, &QPushButton::clicked, ui.keyPlay, &SingleKeySequenceEdit::setDefault);
  QObject::connect(ui.buttonResetTimeStep, &QPushButton::clicked, this, &SettingsDialog::defaultTimeStep);

  QObject::connect(ui.buttonResource, &QPushButton::clicked, this, &SettingsDialog::selectResourcePath);

  QObject::connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &SettingsDialog::dialogueButtonClicked);
}

void SettingsDialog::setTimeStep(double value) {
  ui.spinTimeStep->setValue(static_cast<int>(value));

  passedTimeStep = value;
}

void SettingsDialog::setStepSpinSuffix(SettingsManager::TimeUnit unit) {
  switch (unit) {
  case SettingsManager::TimeUnit::Milliseconds:
    ui.spinTimeStep->setSuffix("ms");
    break;
  case SettingsManager::TimeUnit::Microseconds:
    ui.spinTimeStep->setSuffix("µs");
    break;
  case SettingsManager::TimeUnit::Nanoseconds:
    ui.spinTimeStep->setSuffix("ns");
    break;
  }
}

void SettingsDialog::dialogueButtonClicked(QAbstractButton *button) {
  switch (ui.buttonBox->standardButton(button)) {
  case QDialogButtonBox::RestoreDefaults:
    ui.buttonResetTheme->click();

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

    ui.buttonResetSortOrder->click();

    ui.buttonResetSkybox->click();
    ui.buttonResetFloor->click();
    ui.buttonResetCameraType->click();
    ui.buttonResetMoveSpeedScale->click();
    ui.buttonResetBackgroundColor->click();
    ui.buttonResetSamples->click();
    ui.buttonResetBuildingRender->click();
    ui.buttonResetBuildingOutlines->click();
    ui.buttonResetShowGrid->click();
    ui.buttonResetGridSize->click();
    ui.buttonResetTrails->click();
    ui.buttonResetTrailLength->click();

    ui.buttonResetPlay->click();
    ui.buttonResetTimeStep->click();
    break;
  case QDialogButtonBox::Save: {
    bool requiresRestart = false;
    using Key = SettingsManager::Key;

    // Window
    const auto theme = SettingsManager::WindowThemeFromInt(ui.comboTheme->currentData().toInt());
    if (theme != settings.get<SettingsManager::WindowTheme>(Key::WindowTheme).value()) {
      settings.setTheme(theme);
      settings.set(Key::WindowTheme, theme);
    }

    // Camera
    const auto cameraType = SettingsManager::CameraTypeFromInt(ui.comboCameraType->currentData().toInt());
    if (cameraType != settings.get<SettingsManager::CameraType>(Key::RenderCameraType).value()) {
      settings.set(Key::RenderCameraType, cameraType);
      emit cameraTypeChanged(static_cast<int>(cameraType));
    }

    const auto autoscaleMoveSpeed = settings.get<bool>(Key::AutoScaleMoveSpeed).value();
    if (autoscaleMoveSpeed != ui.checkBoxScaleMoveSpeed->isChecked()) {
      settings.set(Key::AutoScaleMoveSpeed, autoscaleMoveSpeed);
      emit autoscaleMoveSpeedChanged(ui.checkBoxScaleMoveSpeed->isChecked());
    }

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

    const auto forwardKey = ui.keyForward->key();
    if (forwardKey != settings.get<int>(Key::CameraKeyForward).value()) {
      settings.set(Key::CameraKeyForward, forwardKey);
      emit forwardKeyChanged(forwardKey);
    }

    const auto backwardKey = ui.keyBackward->key();
    if (backwardKey != settings.get<int>(Key::CameraKeyBackwards).value()) {
      settings.set(Key::CameraKeyBackwards, backwardKey);
      emit backwardKeyChanged(backwardKey);
    }

    const auto leftKey = ui.keyLeft->key();
    if (leftKey != settings.get<int>(Key::CameraKeyLeft).value()) {
      settings.set(Key::CameraKeyLeft, leftKey);
      emit leftKeyChanged(leftKey);
    }

    const auto rightKey = ui.keyRight->key();
    if (rightKey != settings.get<int>(Key::CameraKeyRight).value()) {
      settings.set(Key::CameraKeyRight, rightKey);
      emit rightKeyChanged(rightKey);
    }

    const auto turnLeftKey = ui.keyTurnLeft->key();
    if (turnLeftKey != settings.get<int>(Key::CameraKeyLeftTurn).value()) {
      settings.set(Key::CameraKeyLeftTurn, turnLeftKey);
      emit turnLeftKeyChanged(turnLeftKey);
    }

    const auto turnRightKey = ui.keyTurnRight->key();
    if (turnRightKey != settings.get<int>(Key::CameraKeyRightTurn).value()) {
      settings.set(Key::CameraKeyRightTurn, turnRightKey);
      emit turnRightKeyChanged(turnRightKey);
    }

    const auto upKey = ui.keyUp->key();
    if (upKey != settings.get<int>(Key::CameraKeyUp).value()) {
      settings.set(Key::CameraKeyUp, upKey);
      emit upKeyChanged(upKey);
    }

    const auto downKey = ui.keyDown->key();
    if (downKey != settings.get<int>(Key::CameraKeyDown).value()) {
      settings.set(Key::CameraKeyDown, downKey);
      emit downKeyChanged(downKey);
    }

    // Charts
    auto chartSortOrder = SettingsManager::ChartDropdownSortOrderFromInt(ui.comboSortOrder->currentData().toInt());
    if (chartSortOrder != settings.get<SettingsManager::ChartDropdownSortOrder>(Key::ChartDropdownSortOrder).value()) {
      settings.set(Key::ChartDropdownSortOrder, chartSortOrder);
      emit chartSortOrderChanged(static_cast<int>(chartSortOrder));
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

    const auto enableFloor = ui.checkBoxFloor->isChecked();
    if (enableFloor != settings.get<bool>(Key::RenderFloor)) {
      settings.set(Key::RenderFloor, enableFloor);
      emit renderFloorChanged(enableFloor);
    }

    using BackgroundColor = SettingsManager::BackgroundColor;
    const auto backgroundColorMode =
        SettingsManager::BackgroundColorFromInt(ui.comboBackgroundColor->currentData().toInt());
    // Handle basic modes
    if (backgroundColorMode != settings.get<BackgroundColor>(Key::RenderBackgroundColor)) {
      settings.set(Key::RenderBackgroundColor, backgroundColorMode);
      switch (backgroundColorMode) {
      case BackgroundColor::Black:
        emit backgroundColorChanged(palette::Black);
        break;
      case BackgroundColor::White:
        emit backgroundColorChanged(palette::White);
        break;
      case BackgroundColor::Custom:
        // Ignored here
        break;
      }
    }

    // Handle custom color
    if (backgroundColorMode == BackgroundColor::Custom &&
        customBackgroundColor != settings.get<QColor>(Key::RenderBackgroundColorCustom).value()) {
      settings.set(Key::RenderBackgroundColorCustom, customBackgroundColor);
      emit backgroundColorChanged(customBackgroundColor);
    }

    auto buildingRenderMode = SettingsManager::BuildingRenderModeFromInt(ui.comboBuildingRender->currentData().toInt());
    if (buildingRenderMode != settings.get<SettingsManager::BuildingRenderMode>(Key::RenderBuildingMode).value()) {
      settings.set(Key::RenderBuildingMode, buildingRenderMode);
      emit buildingRenderModeChanged(static_cast<int>(buildingRenderMode));
    }

    auto renderBuildingOutlines = ui.checkBoxBuildingOutlines->isChecked();
    if (renderBuildingOutlines != settings.get<bool>(Key::RenderBuildingOutlines)) {
      settings.set(Key::RenderBuildingOutlines, renderBuildingOutlines);
      emit buildingRenderOutlinesChanged(renderBuildingOutlines);
    }

    auto enableGrid = ui.checkBoxShowGrid->isChecked();
    if (enableGrid != settings.get<bool>(Key::RenderGrid)) {
      settings.set(Key::RenderGrid, enableGrid);
      emit renderGridChanged(enableGrid);
    }

    const auto gridStepSize = ui.comboGridSize->currentData().toInt();
    if (gridStepSize != settings.get<int>(Key::RenderGridStep)) {
      settings.set(Key::RenderGridStep, gridStepSize);
      emit gridStepSizeChanged(gridStepSize);
    }

    using MotionTrailRenderMode = SettingsManager::MotionTrailRenderMode;
    const auto motionTrailRenderMode =
        SettingsManager::MotionTrailRenderModeFromInt(ui.comboMotionTrailRender->currentData().toInt());
    if (motionTrailRenderMode != settings.get<MotionTrailRenderMode>(Key::RenderMotionTrails).value()) {
      settings.set(Key::RenderMotionTrails, motionTrailRenderMode);
      emit renderTrailsChanged(static_cast<int>(motionTrailRenderMode));
    }

    const auto trailLength = ui.sliderTrailLength->value();
    if (trailLength != settings.get<int>(Key::RenderMotionTrailLength).value()) {
      settings.set(Key::RenderMotionTrailLength, trailLength);
      requiresRestart = true;
    }

    using LabelRenderMode = SettingsManager::LabelRenderMode;
    const auto labelRenderMode = SettingsManager::LabelRenderModeFromInt(ui.comboLabelRender->currentData().toInt());
    if (labelRenderMode != settings.get<LabelRenderMode>(Key::RenderLabels).value()) {
      settings.set(Key::RenderLabels, labelRenderMode);
      emit renderLabelsChanged(static_cast<int>(labelRenderMode));
    }

    const auto labelScale = static_cast<float>(ui.sliderLabelScale->value()) / labelScaleScale;
    if (labelScale != settings.get<float>(Key::RenderLabelScale)) {
      settings.set(Key::RenderLabelScale, labelScale);
      emit labelScaleChanged(labelScale);
    }

    // Playback

    const auto playKey = ui.keyPlay->key();
    if (playKey != settings.get<int>(Key::SceneKeyPlay).value()) {
      settings.set(Key::SceneKeyPlay, playKey);
      emit playKeyChanged(playKey);
    }

    auto oldResourcePath = settings.get<QString>(Key::ResourcePath);
    if (resourcePath != oldResourcePath) {
      settings.set(Key::ResourcePath, resourcePath);
      emit resourcePathChanged(resourcePath);
    }

    using TimeUnit = SettingsManager::TimeUnit;
    const auto oldTimeStepUnit = settings.get<SettingsManager::TimeUnit>(Key::PlaybackTimeStepUnit);
    const auto currentTimeStepUnit = SettingsManager::TimeUnitFromInt(ui.comboTimeStepUnit->currentData().toInt());
    if (currentTimeStepUnit != oldTimeStepUnit) {
      settings.set(Key::PlaybackTimeStepUnit, currentTimeStepUnit);
      // No signal
    }

    const auto oldTimeStep = settings.get<int>(Key::PlaybackTimeStepPreference).value();
    auto currentTimeStep = static_cast<parser::nanoseconds>(ui.spinTimeStep->value());
    if (currentTimeStepUnit == TimeUnit::Microseconds)
      currentTimeStep = fromMicroseconds(currentTimeStep);
    else if (currentTimeStepUnit == TimeUnit::Milliseconds)
      currentTimeStep = fromMilliseconds(currentTimeStep);
    if (currentTimeStep != oldTimeStep) {
      settings.set(Key::PlaybackTimeStepPreference, currentTimeStep);
      // No signal
    }

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

void SettingsDialog::autoscaleMoveSpeedToggled(const int state) {
  ui.sliderMoveSpeed->setEnabled(state != Qt::CheckState::Checked);
  if (state == Qt::CheckState::Checked) {
    ui.sliderMoveSpeed->setValue(
        static_cast<int>(*settings.get<float>(SettingsManager::Key::MoveSpeed) * moveSpeedScale));
  }
}

void SettingsDialog::defaultWindowTheme() {
  const auto defaultTheme = settings.getDefault<SettingsManager::WindowTheme>(SettingsManager::Key::WindowTheme);
  ui.comboTheme->setCurrentIndex(ui.comboTheme->findData(static_cast<int>(defaultTheme)));
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

void SettingsDialog::defaultChartSortOrder() {
  const auto defaultValue = static_cast<int>(
      settings.getDefault<SettingsManager::ChartDropdownSortOrder>(SettingsManager::Key::ChartDropdownSortOrder));
  ui.comboSortOrder->setCurrentIndex(ui.comboSortOrder->findData(defaultValue));
}

void SettingsDialog::defaultBackgroundColor() {
  const auto defaultValue = static_cast<int>(
      settings.getDefault<SettingsManager::BackgroundColor>(SettingsManager::Key::RenderBackgroundColor));
  ui.comboBackgroundColor->setCurrentIndex(ui.comboBackgroundColor->findData(defaultValue));
}

void SettingsDialog::defaultSamples() {
  ui.comboSamples->setCurrentIndex(
      ui.comboSamples->findData(settings.getDefault<int>(SettingsManager::Key::NumberSamples)));
}

void SettingsDialog::defaultEnableSkybox() {
  ui.checkBoxSkybox->setChecked(settings.getDefault<bool>(SettingsManager::Key::RenderSkybox));
}
void SettingsDialog::defaultCameraType() {
  const auto defaultCameraType =
      settings.getDefault<SettingsManager::CameraType>(SettingsManager::Key::RenderCameraType);
  ui.comboCameraType->setCurrentIndex(ui.comboCameraType->findData(static_cast<int>(defaultCameraType)));
}
void SettingsDialog::defaultAutoscaleMoveSpeed() {
  ui.checkBoxScaleMoveSpeed->setChecked(settings.getDefault<bool>(SettingsManager::Key::AutoScaleMoveSpeed));
}

void SettingsDialog::defaultEnableFloor() {
  ui.checkBoxFloor->setChecked(settings.getDefault<bool>(SettingsManager::Key::RenderFloor));
}

void SettingsDialog::defaultBuildingEffect() {
  const auto defaultBuildingMode =
      settings.getDefault<SettingsManager::BuildingRenderMode>(SettingsManager::Key::RenderBuildingMode);
  ui.comboBuildingRender->setCurrentIndex(ui.comboBuildingRender->findData(static_cast<int>(defaultBuildingMode)));
}

void SettingsDialog::defaultBuildingOutlines() {
  ui.checkBoxBuildingOutlines->setChecked(settings.getDefault<bool>(SettingsManager::Key::RenderBuildingOutlines));
}

void SettingsDialog::defaultTimeStep() {
  ui.spinTimeStep->setValue(static_cast<int>(passedTimeStep));
  const auto defaultTimeUnit =
      settings.getDefault<SettingsManager::TimeUnit>(SettingsManager::Key::PlaybackTimeStepUnit);
  ui.comboTimeStepUnit->setCurrentIndex(ui.comboTimeStepUnit->findData(static_cast<int>(defaultTimeUnit)));
}

void SettingsDialog::defaultShowGrid() {
  ui.checkBoxShowGrid->setChecked(settings.getDefault<bool>(SettingsManager::Key::RenderGrid));
}

void SettingsDialog::defaultShowTrails() {
  const auto defaultTrailMode =
      settings.getDefault<SettingsManager::MotionTrailRenderMode>(SettingsManager::Key::RenderMotionTrails);
  ui.comboMotionTrailRender->setCurrentIndex(ui.comboMotionTrailRender->findData(static_cast<int>(defaultTrailMode)));
}

void SettingsDialog::defaultTrailsLength() {
  ui.sliderTrailLength->setValue(settings.getDefault<int>(SettingsManager::Key::RenderMotionTrailLength));
}

void SettingsDialog::defaultShowLabels() {
  const auto defaultMode = settings.getDefault<SettingsManager::LabelRenderMode>(SettingsManager::Key::RenderLabels);
  ui.comboLabelRender->setCurrentIndex(ui.comboLabelRender->findData(static_cast<int>(defaultMode)));
}

void SettingsDialog::defaultLabelScale() {
  ui.sliderLabelScale->setValue(
      static_cast<int>(settings.getDefault<float>(SettingsManager::Key::RenderLabelScale) * labelScaleScale));
}

void SettingsDialog::defaultGridStep() {
  const auto step = settings.getDefault<int>(SettingsManager::Key::RenderGridStep);
  ui.comboGridSize->setCurrentIndex(ui.comboGridSize->findData(step));
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
