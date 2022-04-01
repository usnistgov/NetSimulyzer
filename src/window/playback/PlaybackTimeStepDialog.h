#pragma once

#include "parser/model.h"
#include "src/settings/SettingsManager.h"
#include "ui_PlaybackTimeStepDialog.h"
#include <QAbstractButton>
#include <QDialog>
#include <QString>

namespace netsimulyzer {
class PlaybackTimeStepDialog : public QDialog {
  Q_OBJECT

  SettingsManager settings;
  int initialTimeStep{10};
  SettingsManager::TimeUnit initialUnit;
  Ui::PlaybackTimeStepDialog ui{};

  void dialogueButtonClicked(QAbstractButton *button);
  QString suffixFromUnit(int unit);

protected:
  void showEvent(QShowEvent *event) override;

public:
  explicit PlaybackTimeStepDialog(QWidget *parent);
  void setValue(parser::nanoseconds value, SettingsManager::TimeUnit unit);

signals:
  void timeStepChanged(parser::nanoseconds value, int unit);
};

} // namespace netsimulyzer
