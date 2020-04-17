#include "LoadWorker.h"

namespace visualization {

void LoadWorker::load(const QString &fileName) {
  parser.reset();
  parser.parse(fileName.toStdString().c_str());
  emit fileLoaded(fileName);
}
parser::FileParser &LoadWorker::getParser() {
  return parser;
}

} // namespace visualization
