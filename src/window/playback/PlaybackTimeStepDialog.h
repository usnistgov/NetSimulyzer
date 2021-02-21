#pragma once

#include "ui_PlaybackTimeStepDialog.h"
#include <QAbstractButton>
#include <QDialog>

namespace netsimulyzer {
class PlaybackTimeStepDialog : public QDialog {
  Q_OBJECT

  int initialTimeStep{10};
  Ui::PlaybackTimeStepDialog ui{};

  void dialogueButtonClicked(QAbstractButton *button);

protected:
  void showEvent(QShowEvent *event) override;

public:
  explicit PlaybackTimeStepDialog(QWidget *parent);
  void setValue(int value);

signals:
  void timeStepChanged(int value);
};

} // namespace netsimulyzer
