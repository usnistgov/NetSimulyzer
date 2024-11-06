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
 * Author: Megan Lizambri <megan.lizambri@nist.gov>
 */

#include "DetailWidget.h"
#include "src/group/node/Node.h"
#include "ui_DetailWidget.h"
#include <QMessageBox>
#include <QStandardItem>
#include <algorithm>
#include <glm/vec3.hpp>

namespace {
QString ns3ToQString(const parser::Ns3Color3 &color) {
  return QString("(%1, %2, %3)").arg(color.red).arg(color.green).arg(color.blue);
}

QString ns3ToQString(const parser::Ns3Coordinate &position) {
  return QString("(%1, %2, %3)").arg(position.x).arg(position.y).arg(position.z);
}

QString netsimCoordinateToNs3Formatted(const glm::vec3 &value) {
  return QString("(%1, %2, %3)").arg(value[0]).arg(value[2]).arg(value[1]);
}

QString netsimColorToQString(const glm::vec3 &color) {
  return QString("(%1, %2, %3)")
      .arg(static_cast<int>(color.r * 255.0f))
      .arg(static_cast<int>(color.g * 255.0f))
      .arg(static_cast<int>(color.b * 255.0f));
}

QColor netsimColorFormatted(const glm::vec3 &color) {
  return QColor::fromRgb(static_cast<int>(color.r * 255.0f), static_cast<int>(color.g * 255.0f),
                         static_cast<int>(color.b * 255.0f));
}

QVariant netsimColorFormatted(const std::optional<glm::vec3> &color) {
  if (color.has_value())
    return netsimColorFormatted(color.value());
  return {};
}

QColor ns3ColorFormatted(const parser::Ns3Color3 &color) {
  return QColor::fromRgb(color.red, color.green, color.blue);
}

[[maybe_unused]] QVariant ns3ColorFormatted(const std::optional<parser::Ns3Color3> color) {
  if (!color.has_value())
    return {};

  return ns3ColorFormatted(color.value());
}

} // namespace

namespace netsimulyzer {

DetailWidget::DetailTreeModel::DetailTreeModel(QObject *parent) : QAbstractItemModel(parent) {
  auto &name = rootElement.children.emplace_back(DisplayField::Name, &rootElement, 3u);
  name.children.emplace_back(DisplayField::NameSub, &name);
  name.children.emplace_back(DisplayField::NameId, &name);
  name.children.emplace_back(DisplayField::NameModelFile, &name);

  auto &color = rootElement.children.emplace_back(DisplayField::Color, &rootElement, 3u);
  color.children.emplace_back(DisplayField::ColorBase, &color);
  color.children.emplace_back(DisplayField::ColorHighlight, &color);
  color.children.emplace_back(DisplayField::ColorMotionTrail, &color);

  auto &position = rootElement.children.emplace_back(DisplayField::Position, &rootElement, 4u);
  auto &positionSub = position.children.emplace_back(DisplayField::PositionSub, &position, 3u);
  positionSub.children.emplace_back(DisplayField::PositionSubX, &positionSub);
  positionSub.children.emplace_back(DisplayField::PositionSubY, &positionSub);
  positionSub.children.emplace_back(DisplayField::PositionSubZ, &positionSub);

  auto &positionOffset = position.children.emplace_back(DisplayField::PositionOffset, &position, 3u);
  positionOffset.children.emplace_back(DisplayField::PositionOffsetX, &positionOffset);
  positionOffset.children.emplace_back(DisplayField::PositionOffsetY, &positionOffset);
  positionOffset.children.emplace_back(DisplayField::PositionOffsetZ, &positionOffset);

  auto &positionRendered = position.children.emplace_back(DisplayField::PositionRendered, &position, 3u);
  positionRendered.children.emplace_back(DisplayField::PositionRenderedX, &positionRendered);
  positionRendered.children.emplace_back(DisplayField::PositionRenderedY, &positionRendered);
  positionRendered.children.emplace_back(DisplayField::PositionRenderedZ, &positionRendered);

  auto &positionOrientation = position.children.emplace_back(DisplayField::PositionOrientation, &position, 3u);
  positionOrientation.children.emplace_back(DisplayField::PositionOrientationX, &positionOrientation);
  positionOrientation.children.emplace_back(DisplayField::PositionOrientationY, &positionOrientation);
  positionOrientation.children.emplace_back(DisplayField::PositionOrientationZ, &positionOrientation);
}

void DetailWidget::DetailTreeModel::describe(const Node &n) {
  beginResetModel();
  node = &n;
  endResetModel();
}

void DetailWidget::DetailTreeModel::reset() {
  node = nullptr;
}

void DetailWidget::DetailTreeModel::refresh() {
  beginResetModel();
  endResetModel();
}
const Node *DetailWidget::DetailTreeModel::getNode() const {
  return node;
}

QVariant DetailWidget::DetailTreeModel::data(const QModelIndex &index, int role) const {
  if (!index.isValid() || (role != Qt::DisplayRole && role != Qt::DecorationRole))
    return {};

  const auto item = static_cast<DetailTreeItem *>(index.internalPointer());
  const auto field = item->field;

  // Add color decorations For color values
  if (role == Qt::DecorationRole && node != nullptr) {
    // No decoration for the label column
    if (index.column() == 0)
      return {};

    switch (field) {
    case DisplayField::ColorBase:
      return netsimColorFormatted(node->getModel().getBaseColor());
    case DisplayField::ColorHighlight:
      return netsimColorFormatted(node->getModel().getHighlightColor());
    case DisplayField::ColorMotionTrail:
      return ns3ColorFormatted(node->getNs3Model().trailColor);
    default:
      return {};
    }

    return {};
  }

  switch (index.column()) {
  case 0:
    switch (field) {
    case DisplayField::None:
      return "";
    case DisplayField::Name:
      [[fallthrough]];
    case DisplayField::NameSub:
      return "Name";
    case DisplayField::NameId:
      return "ID";
    case DisplayField::NameModelFile:
      return "Model";
    case DisplayField::Color:
      return "Color";
    case DisplayField::ColorBase:
      return "Base";
    case DisplayField::ColorHighlight:
      return "Highlight";
    case DisplayField::ColorMotionTrail:
      return "Motion Trail";
    case DisplayField::PositionSub:
      [[fallthrough]];
    case DisplayField::Position:
      return "Position";
    case DisplayField::PositionRendered:
      return "Rendered Position";
    case DisplayField::PositionSubX:
    case DisplayField::PositionRenderedX:
    case DisplayField::PositionOffsetX:
      [[fallthrough]];
    case DisplayField::PositionOrientationX:
      return "X";
    case DisplayField::PositionSubY:
    case DisplayField::PositionRenderedY:
    case DisplayField::PositionOffsetY:
      [[fallthrough]];
    case DisplayField::PositionOrientationY:
      return "Y";
    case DisplayField::PositionSubZ:
    case DisplayField::PositionRenderedZ:
    case DisplayField::PositionOffsetZ:
      [[fallthrough]];
    case DisplayField::PositionOrientationZ:
      return "Z";
    case DisplayField::PositionOffset:
      return "Offset";
    case DisplayField::PositionOrientation:
      return "Orientation";
    case DisplayField::Size:
      return "Size";
    case DisplayField::SizeScale:
      return "Scale";
    case DisplayField::SizeHeight:
      return "Height";
    case DisplayField::SizeWidth:
      return "Width";
    case DisplayField::SizeDepth:
      return "Depth";
    default:
      return "!ERROR!";
    }
  case 1: {
    if (node == nullptr)
      return "";

    const auto ns3 = node->getNs3Model();
    const auto netsim = node->getModel();

    switch (field) {
    case DisplayField::None:
      return "";
    // ----- Name -----
    case DisplayField::Name:
      return QString("%1 (%2)").arg(QString::fromStdString(ns3.name)).arg(ns3.id);
    case DisplayField::NameSub:
      return QString::fromStdString(node->getNs3Model().name);
    case DisplayField::NameId:
      return node->getNs3Model().id;
    case DisplayField::NameModelFile:
      return QString::fromStdString(ns3.model);

    // ----- Color -----
    case DisplayField::Color:
      return "";
    case DisplayField::ColorBase:
      if (netsim.getBaseColor())
        return netsimColorToQString(netsim.getBaseColor().value());
      return "";
    case DisplayField::ColorHighlight:
      if (netsim.getHighlightColor())
        return netsimColorToQString(netsim.getHighlightColor().value());
      return "";
    case DisplayField::ColorMotionTrail:
      return ns3ToQString(node->getNs3Model().trailColor);

    // ----- Position -----
    case DisplayField::Position:
      [[fallthrough]];
    case DisplayField::PositionSub:
      return ns3ToQString(ns3.position);
    case DisplayField::PositionSubX:
      return QString::number(ns3.position.x);
    case DisplayField::PositionSubY:
      return QString::number(ns3.position.y);
    case DisplayField::PositionSubZ:
      return QString::number(ns3.position.z);

    // ----- Position Final -----
    case DisplayField::PositionRendered:
      return netsimCoordinateToNs3Formatted(netsim.getPosition());
    case DisplayField::PositionRenderedX:
      return QString::number(netsim.getPosition().x);
    case DisplayField::PositionRenderedY:
      return QString::number(netsim.getPosition().z);
    case DisplayField::PositionRenderedZ:
      return QString::number(netsim.getPosition().y);

      // ----- Position Offset -----
    case DisplayField::PositionOffset:
      return QString("(%1, %2, %3)").arg(ns3.offset.x).arg(ns3.offset.y).arg(ns3.offset.z);
    case DisplayField::PositionOffsetX:
      return QString::number(ns3.offset.x);
    case DisplayField::PositionOffsetY:
      return QString::number(ns3.offset.y);
    case DisplayField::PositionOffsetZ:
      return QString::number(ns3.offset.z);

      // ----- Position Orientation -----
    case DisplayField::PositionOrientation: {
      const auto &rotate = netsim.getRotate();
      return QString("(%1, %2, %3)").arg(rotate[0]).arg(rotate[2]).arg(rotate[1]);
    }
    case DisplayField::PositionOrientationX:
      return QString::number(netsim.getRotate()[0]);
    case DisplayField::PositionOrientationY:
      return QString::number(netsim.getRotate()[2]);
    case DisplayField::PositionOrientationZ:
      return QString::number(netsim.getRotate()[1]);

    // ----- Size -----
    case DisplayField::Size:
      return "";
    case DisplayField::SizeScale:
      return QString("(%1, %2, %3)").arg(ns3.scale[0]).arg(ns3.scale[1]).arg(ns3.scale[2]);
    case DisplayField::SizeHeight: {
      if (ns3.height)
        return QString::number(ns3.height.value());
      return "";
    }
    case DisplayField::SizeWidth: {
      if (ns3.width)
        return QString::number(ns3.width.value());
      return "";
    }
    case DisplayField::SizeDepth: {
      if (ns3.depth)
        return QString::number(ns3.depth.value());
      return "";
    }
    default:
      return "!ERROR!";
    }
  }
  default:
    return "!ERROR!";
  }
}

QVariant DetailWidget::DetailTreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Orientation::Horizontal || role != Qt::DisplayRole)
    return {};

  switch (section) {
  case 0:
    return "Name";
  case 1:
    return "Value";
  default:
    return QString::number(section);
  }
}

QModelIndex DetailWidget::DetailTreeModel::index(int row, int column, const QModelIndex &parent) const {
  if (!hasIndex(row, column, parent))
    return {};

  DetailTreeItem *parentItem;
  if (!parent.isValid())
    parentItem = const_cast<DetailTreeItem *>(&rootElement); // :(
  else
    parentItem = static_cast<DetailTreeItem *>(parent.internalPointer());

  if (row < 0 || row >= static_cast<int>(parentItem->children.size()))
    return {};

  auto element = const_cast<DetailTreeItem *>(&parentItem->children[row]); // :((

  return createIndex(row, column, element);
}

QModelIndex DetailWidget::DetailTreeModel::parent(const QModelIndex &index) const {
  if (!index.isValid())
    return {};

  auto child = static_cast<DetailTreeItem *>(index.internalPointer());

  if (child->parent == nullptr || child->parent == &rootElement)
    return {};

  const auto parent = child->parent;
  const auto grandParent = parent->parent;
  if (grandParent == nullptr || grandParent->children.empty())
    return createIndex(0, 0, const_cast<DetailTreeItem *>(parent)); // :(((

  auto iter = std::find_if(grandParent->children.begin(), grandParent->children.end(), [parent](const auto &possible) {
    return possible.field == parent->field;
  });

  if (iter == grandParent->children.end())
    return createIndex(0, 0, const_cast<DetailTreeItem *>(parent)); // :((((

  const auto row = static_cast<int>(iter - grandParent->children.begin());
  return createIndex(row, 0, const_cast<DetailTreeItem *>(parent));
}

int DetailWidget::DetailTreeModel::rowCount(const QModelIndex &parent) const {
  if (!parent.isValid())
    return static_cast<int>(rootElement.children.size());

  return static_cast<int>(static_cast<DetailTreeItem *>(parent.internalPointer())->children.size());
}

int DetailWidget::DetailTreeModel::columnCount(const QModelIndex &) const {
  return 2;
}

QModelIndexList DetailWidget::DetailTreeModel::getPersistentIndexList() const {
  return persistentIndexList();
}

void DetailWidget::saveExpandedItems() {
  oldExpandedItems.clear();
  for (const auto &index : model.getPersistentIndexList()) {
    if (ui.treeView->isExpanded(index))
      oldExpandedItems.emplace_back(index);
  }
}

void DetailWidget::restoreExpandedItems() {
  for (const auto &index : oldExpandedItems) {
    ui.treeView->expand(index);
  }
}

void DetailWidget::closeEvent(QCloseEvent *event) {
  manager.widgetClosed(this);
  QWidget::closeEvent(event);
}

DetailWidget::DetailWidget(QWidget *parent, DetailManager &manager) : QWidget{parent}, manager{manager} {
  ui.setupUi(this);
  ui.treeView->setModel(&model);
}

void DetailWidget::describe(const Node &node) {
  saveExpandedItems();
  model.describe(node);
  restoreExpandedItems();
}

void DetailWidget::describedItemUpdated() {
  saveExpandedItems();
  model.refresh();
  restoreExpandedItems();
}
const Node *DetailWidget::described() const {
  return model.getNode();
}

void DetailWidget::reset() {
  model.reset();
}
} // namespace netsimulyzer
