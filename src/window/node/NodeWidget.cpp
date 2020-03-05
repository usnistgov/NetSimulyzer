#include "NodeWidget.h"
#include <string>
#include <QObject>
#include <QString>
#include <QListWidget>

namespace visualization {

NodeWidget::NodeWidget(QWidget *parent) : QWidget(parent) {
  ui->setupUi(this);

  QObject::connect(ui->nodeList, &QListWidget::itemActivated, [this] (const QListWidgetItem *item) {
    auto id = item->data(Qt::UserRole).toUInt();
    emit nodeSelected(id);
  });
}

NodeWidget::~NodeWidget() {
  delete ui;
}

void NodeWidget::addNode(const Node &node) {
  nodes.emplace_back(node);

  auto listItem = new QListWidgetItem{QString("Node: ") + std::to_string(node.id).c_str(), ui->nodeList};
  listItem->setData(Qt::UserRole, node.id);

  ui->nodeList->addItem(listItem);
}

void NodeWidget::reset() {
  ui->nodeList->clear();
}

} // namespace visualization