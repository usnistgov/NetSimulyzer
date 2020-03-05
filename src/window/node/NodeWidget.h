#pragma once
#include "../../parser/model.h"
#include "ui_NodeWidget.h"
#include <QWidget>
#include <cstdint>
#include <vector>

namespace visualization {

class NodeWidget : public QWidget {
  Q_OBJECT
  Ui::NodeWidget *ui = new Ui::NodeWidget;
  std::vector<Node> nodes;

public:
  explicit NodeWidget(QWidget *parent = nullptr);
  ~NodeWidget() override;

  void addNode(const Node &node);
  void reset();

signals:
  void nodeSelected(uint32_t id);
};

} // namespace visualization
