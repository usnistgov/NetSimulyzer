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

#include <QPointF>
#include <QVector>
#include <array>
#include <glm/vec3.hpp>
#include <model.h>
#include <variant>

namespace netsimulyzer::undo {

/**
 * An event which undoes a `parser::MoveEvent`
 */
struct MoveEvent {
  /**
   * The point the Node was at before `event`
   * was applied.
   */
  glm::vec3 position;

  /**
   * Position stored in the ns-3 model
   */
  parser::Ns3Coordinate ns3Position;

  /**
   * The event which generated this undo event
   */
  parser::MoveEvent event;
};

/**
 * An event which undoes a `parser::TransmitEvent`
 */
struct TransmitEvent {
  parser::nanoseconds stopTime;
  parser::TransmitEvent event;
};

struct TransmitEndEvent {
  parser::TransmitEndEvent event;
};

/**
 * An event which undoes a `parser::DecorationMoveEvent`
 */
struct DecorationMoveEvent {
  /**
   * The point the Node was at before `event`
   * was applied.
   */
  glm::vec3 position;

  /**
   * The event which generated this undo event
   */
  parser::DecorationMoveEvent event;
};

/**
 * An event which undoes a `parser::NodeOrientationChangeEvent`
 */
struct NodeOrientationChangeEvent {
  /**
   * The original orientation of the Node before `event` was applied
   *
   * Note: each axis is rotated independently (the x rotation is applied, then y, then z)
   * rather than combining all three and then rotating.
   */
  std::array<float, 3> orientation{0.0};

  /**
   * The event which generated this undo event
   */
  parser::NodeOrientationChangeEvent event;
};

/**
 * An event which undoes a `parser::DecorationOrientationChangeEvent`
 */
struct DecorationOrientationChangeEvent {
  /**
   * The original orientation of the Decoration before `event` was applied
   *
   * Note: each axis is rotated independently (the x rotation is applied, then y, then z)
   * rather than combining all three and then rotating.
   */
  std::array<float, 3> orientation{0.0};

  /**
   * The event which generated this undo event
   */
  parser::DecorationOrientationChangeEvent event;
};

struct NodeColorChangeEvent {
  /**
   * The original color before `event` was applied.
   * If no color was specified before, then this optional
   * is also unset set.
   */
  std::optional<glm::vec3> originalColor;

  /**
   * The event which generated this undo event
   */
  parser::NodeColorChangeEvent event;
};

/**
 * An event which undoes a `parser::XYSeriesAddValue`
 */
struct XYSeriesAddValue {
  /**
   * The event which generated this undo event
   */
  parser::XYSeriesAddValue event;
};

struct XYSeriesAddValues {
  /**
   * The event which generated this undo event
   */
  parser::XYSeriesAddValues event;
};

struct XYSeriesClear {
  /**
   * The event which generated this undo event
   */
  parser::XYSeriesClear event;

  /**
   * The list of points on the chart before it was cleared
   */
  QVector<QPointF> points;
};

/**
 * An event which undoes a `parser::CategorySeriesAddValue`
 */
struct CategorySeriesAddValue {
  /**
   * The event which generated this undo event
   */
  parser::CategorySeriesAddValue event;
};

struct StreamAppendEvent {
  /**
   * Number of characters to erase from the unified log
   * Includes the size of the prompt for every newline
   */
  int unifiedLogEraseCount{0};

  /**
   * Original ID of the last writer to the unified log
   */
  unsigned int lastUnifiedWriter{0u};

  /**
   * The event which generated this undo event
   */
  parser::StreamAppendEvent event;
};

using SceneUndoEvent =
    std::variant<MoveEvent, TransmitEvent, TransmitEndEvent, DecorationMoveEvent, NodeOrientationChangeEvent,
                 NodeColorChangeEvent, DecorationOrientationChangeEvent, XYSeriesAddValue, StreamAppendEvent>;

using ChartUndoEvent = std::variant<XYSeriesAddValue, XYSeriesAddValues, XYSeriesClear, CategorySeriesAddValue>;

using LogUndoEvent = std::variant<StreamAppendEvent>;

} // namespace netsimulyzer::undo
