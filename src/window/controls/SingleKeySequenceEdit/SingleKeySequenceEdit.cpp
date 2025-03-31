#include "SingleKeySequenceEdit.h"

SingleKeySequenceEdit::SingleKeySequenceEdit(QWidget *parent) : QKeySequenceEdit(parent) {
}

void SingleKeySequenceEdit::setDefault() {
  if (!defaultKey)
    return;

  if (keySequence()[0].key() != *defaultKey) {
    setKeySequence(*defaultKey);
    emit editingFinished();
  }
}

void SingleKeySequenceEdit::setDefaultKey(int value) {
  defaultKey = static_cast<Qt::Key>(value);
}

std::optional<int> SingleKeySequenceEdit::getDefaultKey() const {
  return defaultKey;
}

Qt::Key SingleKeySequenceEdit::key() const {
  return keySequence()[0].key();
}

void SingleKeySequenceEdit::keyPressEvent(QKeyEvent *pEvent) {
  QKeySequenceEdit::keyPressEvent(pEvent);

  auto sequence = keySequence();
  if (sequence.count() == 0 && defaultKey)
    setKeySequence(*defaultKey);
  else if (sequence.count() > 1)
    setKeySequence(sequence[0]);
}
