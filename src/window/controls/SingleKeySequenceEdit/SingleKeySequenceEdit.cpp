#include "SingleKeySequenceEdit.h"

SingleKeySequenceEdit::SingleKeySequenceEdit(QWidget *parent) : QKeySequenceEdit(parent) {
}

void SingleKeySequenceEdit::setDefault() {
  if (!defaultKey)
    return;

  if (keySequence()[0] != *defaultKey) {
    setKeySequence(*defaultKey);
    emit editingFinished();
  }
}

void SingleKeySequenceEdit::setDefaultKey(int value) {
  defaultKey = value;
}

std::optional<int> SingleKeySequenceEdit::getDefaultKey() const {
  return defaultKey;
}

void SingleKeySequenceEdit::keyPressEvent(QKeyEvent *pEvent) {
  QKeySequenceEdit::keyPressEvent(pEvent);

  auto sequence = keySequence();
  if (sequence.count() == 0 && defaultKey)
    setKeySequence(*defaultKey);
  else if (sequence.count() > 1)
    setKeySequence(sequence[0]);
}
