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

#include "DetailManager.h"
#include "src/window/MainWindow.h"
#include <QDockWidget>
#include <QMainWindow>
#include <algorithm>

namespace netsimulyzer {

DetailManager::DetailManager(QWidget *parent) : QObject{parent} {
}

void DetailManager::spawnWidget(QMainWindow *parent, const Node &node) {
  auto newWidget = new DetailWidget{parent, *this};
  newWidget->describe(node);

  const auto dockWidget = dockWidgets.emplace_back(new QDockWidget{parent});
  dockWidget->setWidget(newWidget);
  // Make sure we don't spawn something too small
  dockWidget->setMinimumWidth(newWidget->minimumWidth());

  dockWidget->setWindowTitle(QString::fromStdString(node.getNs3Model().name));

  parent->addDockWidget(Qt::RightDockWidgetArea, dockWidget);
  detailWidgets.emplace_back(newWidget);
}

void DetailManager::clearWidgets() {
  for (const auto dockWidget : dockWidgets) {
    dockWidget->close();
    dockWidget->deleteLater();
  }

  detailWidgets.clear();
  dockWidgets.clear();
}

void DetailManager::widgetClosed(DetailWidget *detailWidget) {
  // Remove the closed widget from our list
  detailWidgets.erase(std::remove(detailWidgets.begin(), detailWidgets.end(), detailWidget), detailWidgets.end());
}

void DetailManager::nodesUpdated(QVector<unsigned int> nodes) {
  for (const auto &widget : detailWidgets) {
    for (const auto nodeId : nodes) {
      if (widget->described()->getNs3Model().id == nodeId) {
        widget->update();
      }
    }
  }
}

void DetailManager::reset() {
  clearWidgets();
}

} // namespace netsimulyzer