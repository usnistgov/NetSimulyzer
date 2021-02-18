#include "LoadWorker.h"
#include <QElapsedTimer>

namespace netsimulyzer {

void LoadWorker::load(const QString &fileName) {
  QElapsedTimer timer;

  parser.reset();

  timer.start();
  parser.parse(fileName.toStdString().c_str());
  auto elapsed = static_cast<unsigned long long>(timer.elapsed());

  emit fileLoaded(fileName, elapsed);
}
parser::FileParser &LoadWorker::getParser() {
  return parser;
}

} // namespace netsimulyzer
