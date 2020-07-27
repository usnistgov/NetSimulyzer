/*
 * NIST-developed software is provided by NIST as a public service. You may use,
 * copy and distribute copies of the software in any medium, provided that you
 * keep intact this entire notice. You may improve,modify and create derivative
 * works of the software or any portion of the software, and you may copy and
 * distribute such modifications or works. Modified works should carry a notice
 * stating that you changed the software and should note the date and nature of
 * any such change. Please explicitly acknowledge the National Institute of
 * Standards and Technology as the source of the software.
 *
 * NIST-developed software is expressly provided "AS IS." NIST MAKES NO
 * WARRANTY OF ANY KIND, EXPRESS, IMPLIED, IN FACT OR ARISING BY OPERATION OF
 * LAW, INCLUDING, WITHOUT LIMITATION, THE IMPLIED WARRANTY OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, NON-INFRINGEMENT
 * AND DATA ACCURACY. NIST NEITHER REPRESENTS NOR WARRANTS THAT THE
 * OPERATION OF THE SOFTWARE WILL BE UNINTERRUPTED OR ERROR-FREE, OR THAT
 * ANY DEFECTS WILL BE CORRECTED. NIST DOES NOT WARRANT OR MAKE ANY
 * REPRESENTATIONS REGARDING THE USE OF THE SOFTWARE OR THE RESULTS THEREOF,
 * INCLUDING BUT NOT LIMITED TO THE CORRECTNESS, ACCURACY, RELIABILITY,
 * OR USEFULNESS OF THE SOFTWARE.
 *
 * You are solely responsible for determining the appropriateness of using and
 * distributing the software and you assume all risks associated with its use,
 * including but not limited to the risks and costs of program errors,
 * compliance with applicable laws, damage to or loss of data, programs or
 * equipment, and the unavailability or interruption of operation. This
 * software is not intended to be used in any situation where a failure could
 * cause risk of injury or damage to property. The software developed by NIST
 * employees is not subject to copyright protection within the United States.
 *
 * Author: Evan Black <evan.black@nist.gov>
 */

#include "NodeWidget.h"
#include <QListWidget>
#include <QObject>
#include <QStandardItemModel>
#include <QString>
#include <string>

namespace visualization {

int NodeWidget::NodeModel::rowCount(const QModelIndex &) const {
  return static_cast<int>(nodes.size());
}

int NodeWidget::NodeModel::columnCount(const QModelIndex &) const {
  return 2;
}

QVariant NodeWidget::NodeModel::data(const QModelIndex &index, int role) const {

  switch (role) {
  case Qt::UserRole:
    // Used for QTableView::doubleClicked signal for focusing on a Node
    return {nodes[index.row()].id};
  case Qt::DisplayRole: {
    const auto &node = nodes[index.row()];
    switch (index.column()) {
    case 0:
      return {node.id};
    case 1:
      return {QString::fromStdString(node.name)};
    default:
      return {};
    }
  }
  case Qt::TextAlignmentRole:
    switch (index.column()) {
    case 0:
      return {Qt::AlignRight | Qt::AlignVCenter};
    case 1:
      return {Qt::AlignLeft | Qt::AlignVCenter};
    default:
      return {};
    }
  default:
    return {};
  }
}
QVariant NodeWidget::NodeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return {};
  switch (section) {
  case 0:
    return "ID";
  case 1:
    return "Name";
  default:
    return {};
  }
}
void NodeWidget::NodeModel::append(const parser::Node &node) {
  beginInsertRows({}, static_cast<int>(nodes.size()), static_cast<int>(nodes.size()));
  nodes.emplace_back(node);
  endInsertRows();
}

Qt::ItemFlags NodeWidget::NodeModel::flags(const QModelIndex &index) const {
  // Disallow editing on all items
  return QAbstractTableModel::flags(index) & ~Qt::ItemIsEditable;
}

void NodeWidget::NodeModel::reset() {
  if (nodes.empty())
    return;

  beginResetModel();
  nodes.clear();
  endResetModel();
}

NodeWidget::NodeWidget(QWidget *parent) : QWidget(parent) {
  ui->setupUi(this);

  QObject::connect(ui->nodeTable, &QTableView::doubleClicked,
                   [this](const QModelIndex &index) { emit nodeSelected(index.data(Qt::UserRole).toUInt()); });

  proxyModel.setSourceModel(&model);
  ui->nodeTable->setModel(&proxyModel);

  // Sort by Node ID, ascending by default
  proxyModel.sort(0, Qt::AscendingOrder);

  ui->nodeTable->resizeColumnsToContents();
  ui->nodeTable->horizontalHeader()->setStretchLastSection(true);
}

NodeWidget::~NodeWidget() {
  delete ui;
}

void NodeWidget::addNode(const parser::Node &node) {
  model.append(node);
}

void NodeWidget::reset() {
  model.reset();
}

} // namespace visualization