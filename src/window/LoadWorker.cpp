#include "LoadWorker.h"
#include <chrono>

namespace visualization {

void LoadWorker::load(const QString &fileName) {
  using namespace std::chrono;

  parser.reset();

  auto start = steady_clock::now();
  parser.parse(fileName.toStdString().c_str());
  auto end = steady_clock::now();

  emit fileLoaded(fileName, duration_cast<milliseconds, long>(end - start).count());
}
parser::FileParser &LoadWorker::getParser() {
  return parser;
}

} // namespace visualization
