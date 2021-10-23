#include "LoadWorker.h"
#include <QElapsedTimer>

namespace netsimulyzer {

void LoadWorker::load(const QString &fileName) {
  QElapsedTimer timer;

  parser.reset();

  timer.start();
  auto parseError = parser.parse(fileName.toStdString().c_str());
  auto elapsed = static_cast<unsigned long long>(timer.elapsed());

  if (parseError) {
    emit error(QString::fromStdString(parseError.value().message), parseError.value().offset);
    return;
  }

  emit fileLoaded(fileName, elapsed);
}

parser::FileParser &LoadWorker::getParser() {
  return parser;
}

} // namespace netsimulyzer
