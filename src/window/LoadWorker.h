#pragma once

#include <QObject>
#include <file-parser.h>

namespace visualization {

class LoadWorker : public QObject {
  Q_OBJECT
  parser::FileParser parser;

public:
  [[nodiscard]] parser::FileParser &getParser();
public slots:
  void load(const QString &fileName);
signals:
  void fileLoaded(const QString &fileName, unsigned long long milliseconds);
};

} // namespace visualization
