#include "PlaybackTimeStepDialog.h"
#include "src/conversion.h"
#include "src/settings/SettingsManager.h"
#include "ui_PlaybackTimeStepDialog.h"
#include <QApplication>
#include <QMessageBox>
#include <QObject>
#include <QString>

namespace netsimulyzer {

void PlaybackTimeStepDialog::dialogueButtonClicked(QAbstractButton *button) {
  const auto spinnerValue = ui.spinTimestep->value();
  const auto unit = ui.comboUnit->currentData().toInt();
  const auto suffix = suffixFromUnit(unit);
  ui.spinTimestep->setSuffix(suffix);

  switch (ui.buttonBox->standardButton(button)) {
  case QDialogButtonBox::Ok:
    if (spinnerValue != initialTimeStep || unit != static_cast<int>(initialUnit)) {
      switch (SettingsManager::TimeUnitFromInt(unit)) {
      case SettingsManager::TimeUnit::Nanoseconds:
        emit timeStepChanged(spinnerValue, unit);
        break;
      case SettingsManager::TimeUnit::Microseconds:
        emit timeStepChanged(fromMicroseconds(spinnerValue), unit);
        break;
      case SettingsManager::TimeUnit::Milliseconds:
        emit timeStepChanged(fromMilliseconds(spinnerValue), unit);
        break;
      }
      accept();
    }
    break;
  case QDialogButtonBox::Cancel:
    [[fallthrough]];
  default:
    ui.spinTimestep->setValue(initialTimeStep);
    ui.comboUnit->setCurrentIndex(ui.comboUnit->findData(static_cast<int>(initialUnit)));
    reject();
    break;
  }
}

QString PlaybackTimeStepDialog::suffixFromUnit(int unit) {
  switch (unit) {
  case static_cast<int>(SettingsManager::TimeUnit::Nanoseconds):
    return {"ns"};
  case static_cast<int>(SettingsManager::TimeUnit::Microseconds):
    return {"µs"};
  case static_cast<int>(SettingsManager::TimeUnit::Milliseconds):
    return {"ms"};
  default:
    QMessageBox::critical(this, "Unknown Time Unit Selected",
                          "Unknown unit with ID: " + QString::number(unit) + " selected");
    QApplication::exit(1);
    break;
  }

  // Should never get here, but just in case
  return {};
}

void PlaybackTimeStepDialog::showEvent(QShowEvent *event) {
  initialTimeStep = ui.spinTimestep->value();
  initialUnit = SettingsManager::TimeUnitFromInt(ui.comboUnit->currentData().toInt());
  QDialog::showEvent(event);
}

PlaybackTimeStepDialog::PlaybackTimeStepDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);

  ui.comboUnit->addItem("ns", static_cast<int>(SettingsManager::TimeUnit::Nanoseconds));
  ui.comboUnit->addItem("µs", static_cast<int>(SettingsManager::TimeUnit::Microseconds));
  ui.comboUnit->addItem("ms", static_cast<int>(SettingsManager::TimeUnit::Milliseconds));

  QObject::connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &PlaybackTimeStepDialog::dialogueButtonClicked);

  QObject::connect(ui.comboUnit, qOverload<int>(&QComboBox::currentIndexChanged), [this](int /* index */) {
    ui.spinTimestep->setSuffix(suffixFromUnit(ui.comboUnit->currentData().toInt()));
  });
}

void PlaybackTimeStepDialog::setValue(parser::nanoseconds value, SettingsManager::TimeUnit unit) {
  switch (unit) {
  case SettingsManager::TimeUnit::Milliseconds:
    ui.spinTimestep->setValue(toMilliseconds(value));
    break;
  case SettingsManager::TimeUnit::Microseconds:
    ui.spinTimestep->setValue(toMicroseconds(value));
    break;
  case SettingsManager::TimeUnit::Nanoseconds:
    ui.spinTimestep->setValue(value);
    break;
  }

  ui.comboUnit->setCurrentIndex(ui.comboUnit->findData(static_cast<int>(unit)));
}

} // namespace netsimulyzer
