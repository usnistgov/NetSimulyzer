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
#include <QStandardItem>
#include <QMessageBox>

namespace netsimulyzer {

DetailWidget::DetailWidget(QWidget *parent) : QWidget(parent) {
  ui.setupUi(this);
}


void DetailWidget::describe(const Node &node) {

    QStandardItemModel* model = new QStandardItemModel(5, 3);
    QStandardItem* name = new QStandardItem("Name");
    QStandardItem* color = new QStandardItem("Color");
    QStandardItem* position = new QStandardItem("Position");
    QStandardItem* size = new QStandardItem("Size");
    QStandardItem* network = new QStandardItem("Network");

    model->setItem(0, 0, name);
    model->setItem(1, 0, color);
    model->setItem(2, 0, position);
    model->setItem(3, 0, size);
    model->setItem(4, 0, network);

    model->setItem(0, 1, new QStandardItem(node.getNs3Model().name.c_str()));
    name->setChild(0, 0, new QStandardItem("Node ID"));
    name->setChild(0, 1, new QStandardItem(QString::number(node.getNs3Model().id)));
    name->setChild(1, 0, new QStandardItem("Node Name"));
    name->setChild(1, 1, new QStandardItem(node.getNs3Model().name.c_str()));

    QString base_red = QString::number(node.getNs3Model().baseColor->red);
    QString base_blue = QString::number(node.getNs3Model().baseColor->blue);
    QString base_green = QString::number(node.getNs3Model().baseColor->green);

    QStandardItem* widget1Space = new QStandardItem("");
    color->setChild(1, 0, widget1Space);

    color->setChild(0, 0, new QStandardItem("Base"));
    color->setChild(0, 1, new QStandardItem("Red"));
    color->setChild(0, 2, new QStandardItem(base_red));
    color->setChild(1, 1, new QStandardItem("Green"));
    color->setChild(1, 2, new QStandardItem(base_green));
    color->setChild(2, 1, new QStandardItem("Blue"));
    color->setChild(2, 2, new QStandardItem(base_blue));
    ui.widget->setStyleSheet("QWidget {background-color: rgb("+base_red+", "+base_green+", "+base_blue+");}");

    QString highlight_red = QString::number(node.getNs3Model().highlightColor->red);
    QString highlight_blue = QString::number(node.getNs3Model().highlightColor->blue);
    QString highlight_green = QString::number(node.getNs3Model().highlightColor->green);

    QStandardItem* widget2Space = new QStandardItem("");
    color->setChild(4, 0, widget2Space);

    color->setChild(3, 0, new QStandardItem("Highlight"));
    color->setChild(3, 1, new QStandardItem("Red"));
    color->setChild(3, 2, new QStandardItem(highlight_red));
    color->setChild(4, 1, new QStandardItem("Green"));
    color->setChild(4, 2, new QStandardItem(highlight_green));
    color->setChild(5, 1, new QStandardItem("Blue"));
    color->setChild(5, 2, new QStandardItem(highlight_blue));
    ui.widget_2->setStyleSheet("QWidget {background-color: rgb("+highlight_red+", "+highlight_green+", "+highlight_blue+");}");

    QString trail_red = QString::number(node.getNs3Model().trailColor.red);
    QString trail_blue = QString::number(node.getNs3Model().trailColor.blue);
    QString trail_green = QString::number(node.getNs3Model().trailColor.green);

    QStandardItem* widget3Space = new QStandardItem("");
    color->setChild(7, 0, widget3Space);

    color->setChild(6, 0, new QStandardItem("Motion Trail"));
    color->setChild(6, 1, new QStandardItem("Red"));
    color->setChild(6, 2, new QStandardItem(trail_red));
    color->setChild(7, 1, new QStandardItem("Green"));
    color->setChild(7, 2, new QStandardItem(trail_green));
    color->setChild(8, 1, new QStandardItem("Blue"));
    color->setChild(8, 2, new QStandardItem(trail_blue));
    ui.widget_3->setStyleSheet("QWidget {background-color: rgb("+trail_red+", "+trail_green+", "+trail_blue+");}");

    QString location = "("+QString::number(node.getNs3Model().position.x)+", "+QString::number(node.getNs3Model().position.y)+", "+QString::number(node.getNs3Model().position.z)+")";

    position->setChild(0, 0, new QStandardItem("Location"));
    position->setChild(0, 1, new QStandardItem(location));

    QString offset_x = QString::number(node.getNs3Model().offset.x);
    QString offset_y = QString::number(node.getNs3Model().offset.y);
    QString offset_z = QString::number(node.getNs3Model().offset.z);

    position->setChild(1, 0, new QStandardItem("Offset"));
    position->setChild(1, 1, new QStandardItem("("+offset_x+", "+offset_y+", "+offset_z+")"));

    QString orientation_x = QString::number(node.getNs3Model().orientation[0]);
    QString orientation_y = QString::number(node.getNs3Model().orientation[1]);
    QString orientation_z = QString::number(node.getNs3Model().orientation[2]);

    position->setChild(2, 0, new QStandardItem("Orientation"));
    position->setChild(2, 1, new QStandardItem("("+orientation_x+", "+orientation_y+", "+orientation_z+")"));

    model->setItem(2, 1, new QStandardItem(location));

    QString scale_x = QString::number(node.getNs3Model().scale[0]);
    QString scale_y = QString::number(node.getNs3Model().scale[1]);
    QString scale_z = QString::number(node.getNs3Model().scale[2]);

    size->setChild(0, 0, new QStandardItem("Scale Axes"));
    size->setChild(0, 1, new QStandardItem("("+scale_x+", "+scale_y+", "+scale_z+")"));

    QString scale = QString::number(node.getModel().getScale()[1]);
    QString height = QString::number(node.getNs3Model().height.value());
    QString width = QString::number(node.getNs3Model().width.value());
    QString depth = QString::number(node.getNs3Model().depth.value());

    size->setChild(1, 0, new QStandardItem("Scale"));
    size->setChild(1, 1, new QStandardItem(scale));
    size->setChild(2, 0, new QStandardItem("Height"));
    size->setChild(2, 1, new QStandardItem(height));
    size->setChild(3, 0, new QStandardItem("Width"));
    size->setChild(3, 1, new QStandardItem(width));
    size->setChild(4, 0, new QStandardItem("Depth"));
    size->setChild(4, 1, new QStandardItem(depth));
    model->setItem(3, 1, new QStandardItem(height+" x "+width+" x "+depth));

    network->setChild(0, 0, new QStandardItem("IP Addresses"));
    network->setChild(1, 0, new QStandardItem("MAC Addresses"));

    ui.treeView->setModel(model);
    ui.treeView->setHeaderHidden(true);
    ui.treeView->setColumnWidth(0, 150);

    //these three statements cause the crash
    ui.treeView->setIndexWidget(widget1Space->index(), ui.widget);
    ui.treeView->setIndexWidget(widget2Space->index(), ui.widget_2);
    ui.treeView->setIndexWidget(widget3Space->index(), ui.widget_3);


}
} // namespace netsimulyzer


