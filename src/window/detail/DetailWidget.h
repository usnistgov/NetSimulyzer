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
 * Author: Megan Lizambri <megan.lizambri@nist.gov>, Evan Black <evan.black@nist.gov>
 */

#pragma once
#include "src/group/node/Node.h"
#include "ui_DetailWidget.h"
#include <QAbstractItemModel>
#include <QObject>
#include <QPoint>
#include <QString>
#include <QWidget>
#include <vector>

namespace netsimulyzer {

class DetailWidget : public QWidget {
  Q_OBJECT

  enum class DisplayField {
    None, // Used by the root element only
    Name,
    NameSub,
    NameId,
    NameModelFile,
    Color,
    ColorBase,
    ColorHighlight,
    ColorMotionTrail,
    Position,
    PositionSub,
    PositionSubX,
    PositionSubY,
    PositionSubZ,
    PositionRendered,
    PositionRenderedX,
    PositionRenderedY,
    PositionRenderedZ,
    PositionOffset,
    PositionOffsetX,
    PositionOffsetY,
    PositionOffsetZ,
    PositionOrientation,
    PositionOrientationX,
    PositionOrientationY,
    PositionOrientationZ,
    Size,
    SizeScale,
    SizeHeight,
    SizeWidth,
    SizeDepth
  };

  struct DetailTreeItem {
    explicit DetailTreeItem(std::size_t maxChildren) {
      children.reserve(maxChildren);
    };

    DetailTreeItem(DisplayField field, DetailTreeItem *parent) : field(field), parent(parent){};

    DetailTreeItem(DisplayField field, DetailTreeItem *parent, std::size_t maxChildren) : field(field), parent(parent) {
      children.reserve(maxChildren);
    };

    DisplayField field{DisplayField::None};
    std::vector<DetailTreeItem> children;
    DetailTreeItem *parent{nullptr};
  };

  class DetailTreeModel : public QAbstractItemModel {
    const Node *node{nullptr};
    DetailTreeItem rootElement{10u};

  public:
    explicit DetailTreeModel(QObject *parent);
    void describe(const Node &n);
    void reset();
    void refresh();
    [[nodiscard]] QModelIndexList getPersistentIndexList() const;

    QVariant data(const QModelIndex &index, int role) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    QModelIndex index(int row, int column, const QModelIndex &parent) const override;
    QModelIndex parent(const QModelIndex &index) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
  };

  Ui::DetailWidget ui{};
  DetailTreeModel model{this};
  std::vector<QModelIndex> oldExpandedItems;

  void saveExpandedItems();
  void restoreExpandedItems();

public:
  explicit DetailWidget(QWidget *parent = nullptr);
  void describe(const Node &node);
  void describedItemUpdated();
  void reset();
};

} // namespace netsimulyzer
