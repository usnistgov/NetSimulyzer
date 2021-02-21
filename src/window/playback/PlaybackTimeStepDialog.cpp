#include "PlaybackTimeStepDialog.h"
#include "ui_PlaybackTimeStepDialog.h"

namespace netsimulyzer {

void PlaybackTimeStepDialog::dialogueButtonClicked(QAbstractButton *button) {
  switch (ui.buttonBox->standardButton(button)) {
  case QDialogButtonBox::Ok:
    if (ui.spinTimestep->value() != initialTimeStep)
      emit timeStepChanged(ui.spinTimestep->value());
    accept();
    break;
  case QDialogButtonBox::Cancel:
    [[fallthrough]];
  default:
    ui.spinTimestep->setValue(initialTimeStep);
    reject();
    break;
  }
}

void PlaybackTimeStepDialog::showEvent(QShowEvent *event) {
  initialTimeStep = ui.spinTimestep->value();
  QDialog::showEvent(event);
}

PlaybackTimeStepDialog::PlaybackTimeStepDialog(QWidget *parent) : QDialog(parent) {
  ui.setupUi(this);
  QObject::connect(ui.buttonBox, &QDialogButtonBox::clicked, this, &PlaybackTimeStepDialog::dialogueButtonClicked);
}

void PlaybackTimeStepDialog::setValue(int value) {
  ui.spinTimestep->setValue(value);
}

} // namespace netsimulyzer
