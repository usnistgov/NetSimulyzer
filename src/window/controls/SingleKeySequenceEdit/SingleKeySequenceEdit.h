#pragma once

#include <QKeyEvent>
#include <QKeySequenceEdit>
#include <QObject>
#include <QWidget>
#include <optional>

class SingleKeySequenceEdit : public QKeySequenceEdit {
  Q_OBJECT

  std::optional<int> defaultKey;

public:
  explicit SingleKeySequenceEdit(QWidget *parent = nullptr);
  void setDefault();
  void setDefaultKey(int value);
  [[nodiscard]] std::optional<int> getDefaultKey() const;

protected:
  void keyPressEvent(QKeyEvent *pEvent) override;
};
