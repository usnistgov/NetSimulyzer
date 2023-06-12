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

#include "ControlsChartView.h"
#include "src/window/chart/ChartWidget.h"
#include <QClipboard>
#include <QFileDialog>
#include <QGuiApplication>
#include <QMenu>
#include <QPixmap>
#include <QtCharts/QChart>

void ControlsChartView::keyPressEvent(QKeyEvent *event) {
  const auto ctrl = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
  const auto alt = event->modifiers() & Qt::KeyboardModifier::AltModifier;

  switch (event->key()) {
  case Qt::Key_Plus:
    [[fallthrough]];
  case Qt::Key_Equal: // Allow the + key next to Backspace to be used (without Shift)
    if (ctrl)         // Horizontal zoom
      xAxis->scaleRange(zoomFactor);
    else if (alt) // Vertical Zoom
      yAxis->scaleRange(zoomFactor);
    else { // Horizontal & Vertical zoom
      xAxis->scaleRange(zoomFactor);
      yAxis->scaleRange(zoomFactor);
    }
    replot();
    break;
  case Qt::Key_Minus:
    if (ctrl) // Horizontal zoom
      xAxis->scaleRange(1.0 / zoomFactor);
    else if (alt) // Vertical Zoom
      yAxis->scaleRange(1.0 / zoomFactor);
    else { // Horizontal & Vertical zoom
      xAxis->scaleRange(1.0 / zoomFactor);
      yAxis->scaleRange(1.0 / zoomFactor);
    }
    replot();
    break;
  case Qt::Key_R: {
    const auto &range = chartWidget->getTieRange();
    xAxis->setRange(range.x);
    yAxis->setRange(range.y);
    replot();
  } break;
  case Qt::Key_Left:
    //    chart()->scroll(-scrollMagnitude, 0);
    break;
  case Qt::Key_Right:
    //    chart()->scroll(scrollMagnitude, 0);
    break;
  case Qt::Key_Up:
    //    chart()->scroll(0, scrollMagnitude);
    break;
  case Qt::Key_Down:
    //    chart()->scroll(0, -scrollMagnitude);
    break;
  }
  QCustomPlot::keyPressEvent(event);
}

void ControlsChartView::wheelEvent(QWheelEvent *event) {
  const auto delta = event->angleDelta().y();
  if (delta == 0) {
    QCustomPlot::wheelEvent(event);
    return;
  }

  const auto horizontal = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
  const auto vertical = event->modifiers() & Qt::KeyboardModifier::AltModifier;

  if (delta > 0) {
    if (horizontal)
      xAxis->scaleRange(zoomFactor);
    else if (vertical)
      yAxis->scaleRange(zoomFactor);
    else { // Horizontal & Vertical zoom
      xAxis->scaleRange(zoomFactor);
      yAxis->scaleRange(zoomFactor);
    }
    replot();
  } else { // delta < 0
    if (horizontal)
      xAxis->scaleRange(1.0 / zoomFactor);
    else if (vertical)
      yAxis->scaleRange(1.0 / zoomFactor);
    else {
      xAxis->scaleRange(1.0 / zoomFactor);
      yAxis->scaleRange(1.0 / zoomFactor);
    }
    replot();
  }
  QCustomPlot::wheelEvent(event);
}

void ControlsChartView::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu;
  menu.addAction("Save Chart Image", [this]() {
    const auto image = grab();
    const auto fileName = QFileDialog::getSaveFileName(this, "Save Chart Image", "", "Images (*.png *.jpeg)");
    if (fileName.isEmpty()) {
      return;
    }
    image.save(fileName);
  });

  menu.addAction("Copy Chart Image to Clipboard", [this]() {
    auto image = grab();
    auto clipboard = QGuiApplication::clipboard();
    clipboard->setPixmap(image);
  });

  menu.exec(event->globalPos());
}

ControlsChartView::ControlsChartView(QWidget *parent)
    : QCustomPlot(parent), title(std::make_unique<QCPTextElement>(this, "")) {

  // No `iRangeZoom`, since we handle that by hand
  setInteractions(QCP::iRangeDrag);
  legend->setVisible(true);

  plotLayout()->insertRow(0);
  plotLayout()->addElement(0, 0, title.get());
}

void ControlsChartView::setChartWidget(netsimulyzer::ChartWidget *value) {
  chartWidget = value;
}
