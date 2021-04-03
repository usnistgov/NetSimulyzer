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
#include <QtCharts/QChart>

void ControlsChartView::keyPressEvent(QKeyEvent *event) {
  const auto ctrl = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
  const auto alt = event->modifiers() & Qt::KeyboardModifier::AltModifier;

  switch (event->key()) {
  case Qt::Key_Plus:
    [[fallthrough]];
  case Qt::Key_Equal: // Allow the + key next to Backspace to be used (without Shift)
    // Horizontal zoom
    if (ctrl) {
      auto area = chart()->plotArea();
      area.setWidth(area.width() / zoomFactor);
      chart()->zoomIn(area);
    } else if (alt) { // Vertical Zoom
      auto area = chart()->plotArea();
      area.setHeight(area.height() / zoomFactor);
      chart()->zoomIn(area);
    } else // Horizontal & Vertical zoom
      chart()->zoom(zoomFactor);
    break;
  case Qt::Key_Minus:
    // Horizontal zoom
    if (ctrl) {
      auto area = chart()->plotArea();
      area.setWidth(area.width() * zoomFactor);
      chart()->zoomIn(area);
    } else if (alt) { // Vertical Zoom
      auto area = chart()->plotArea();
      area.setHeight(area.height() * zoomFactor);
      chart()->zoomIn(area);
    } else // Horizontal & Vertical zoom
      chart()->zoom(1.0 / zoomFactor);
    break;
  case Qt::Key_R:
    chart()->zoomReset();
    break;
  case Qt::Key_Left:
    chart()->scroll(-scrollMagnitude, 0);
    break;
  case Qt::Key_Right:
    chart()->scroll(scrollMagnitude, 0);
    break;
  case Qt::Key_Up:
    chart()->scroll(0, scrollMagnitude);
    break;
  case Qt::Key_Down:
    chart()->scroll(0, -scrollMagnitude);
    break;
  default:
    QGraphicsView::keyPressEvent(event);
    break;
  }
}

void ControlsChartView::mousePressEvent(QMouseEvent *event) {
  QChartView::mousePressEvent(event);

  // Only allow moves with Left Mouse
  if (!(event->buttons() & Qt::LeftButton))
    return;

  mouseDown = true;
  lastMousePosition = event->pos();
}
void ControlsChartView::mouseMoveEvent(QMouseEvent *event) {
  QChartView::mouseMoveEvent(event);
  if (!mouseDown)
    return;

  auto delta = lastMousePosition - event->pos();

  // Invert delta Y otherwise the movement on the Y axis will
  // be inverted relative to mouse movements
  // (e.g. moving the mouse down will move the chart up)
  chart()->scroll(delta.x(), -delta.y());
  lastMousePosition = event->pos();
}
void ControlsChartView::mouseReleaseEvent(QMouseEvent *event) {
  QChartView::mouseReleaseEvent(event);

  // If the event was fired but LeftMouse is still down
  if (event->buttons() & Qt::LeftButton)
    return;

  mouseDown = false;
}

ControlsChartView::ControlsChartView(QWidget *parent) : QChartView(parent) {
}

void ControlsChartView::wheelEvent(QWheelEvent *event) {
  const auto delta = event->angleDelta().y();
  if (delta == 0) {
    QGraphicsView::wheelEvent(event);
    return;
  }

  const auto ctrl = event->modifiers() & Qt::KeyboardModifier::ControlModifier;
  const auto alt = event->modifiers() & Qt::KeyboardModifier::AltModifier;

  if (delta > 0) {
    // Horizontal zoom
    if (ctrl) {
      auto area = chart()->plotArea();
      area.setWidth(area.width() / zoomFactor);
      chart()->zoomIn(area);
    } else if (alt) { // Vertical Zoom
      auto area = chart()->plotArea();
      area.setHeight(area.height() / zoomFactor);
      chart()->zoomIn(area);
    } else // Horizontal & Vertical zoom
      chart()->zoom(zoomFactor);
  } else { // delta < 0
    // Horizontal zoom
    if (ctrl) {
      auto area = chart()->plotArea();
      area.setWidth(area.width() * zoomFactor);
      chart()->zoomIn(area);
    } else if (alt) { // Vertical Zoom
      auto area = chart()->plotArea();
      area.setHeight(area.height() * zoomFactor);
      chart()->zoomIn(area);
    } else // Horizontal & Vertical zoom
      chart()->zoom(1.0 / zoomFactor);
  }

  QGraphicsView::wheelEvent(event);
}
