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

#pragma once
#include "ui_NodeWidget.h"
#include <QAbstractTableModel>
#include <QSortFilterProxyModel>
#include <QStandardItemModel>
#include <QVariant>
#include <QWidget>
#include <cstdint>
#include <model.h>
#include <vector>

namespace netsimulyzer {

class NodeWidget : public QWidget {
  Q_OBJECT

  /**
   * Provides the data for the nodeTable
   */
  class NodeModel : public QAbstractTableModel {
    /**
     * Each individual row in the table
     */
    std::vector<parser::Node> nodes;

  public:
    explicit NodeModel(QObject *parent = {}) : QAbstractTableModel(parent){};

    [[nodiscard]] int rowCount(const QModelIndex &) const override;
    [[nodiscard]] int columnCount(const QModelIndex &) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    /**
     * Add a Node to the table
     *
     * @param node
     * The Node to add to the table
     */
    void append(const parser::Node &node);

    /**
     * Clear all Nodes from the model
     */
    void reset();
  };

  Ui::NodeWidget *ui = new Ui::NodeWidget;
  NodeModel model;
  QSortFilterProxyModel proxyModel;

public:
  explicit NodeWidget(QWidget *parent = nullptr);
  ~NodeWidget() override;

  void addNode(const parser::Node &node);
  void reset();

signals:
  void nodeSelected(uint32_t id);
};

} // namespace netsimulyzer
