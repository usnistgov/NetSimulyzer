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
#include <cstdint>
#include <osg/Vec3d>
#include <variant>

namespace visualization {

/**
 * Event that changes the position of the indicated node
 */
struct MoveEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Node to move to `targetPosition`
   */
  uint32_t nodeId = 0;

  /**
   * The position in the scene to move the Node to.
   *
   * Note: this will act on the relative position of
   * the node, not the absolute position.
   * If the node is affected by another
   * transform, this will be relative to that transform
   */
  osg::Vec3d targetPosition;
};

/**
 * Event that changes the position of the indicated Decoration
 */
struct DecorationMoveEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Decoration to move to `targetPosition`
   */
  uint32_t decorationId = 0;

  /**
   * The position in the scene to move the Decoration to.
   */
  osg::Vec3d targetPosition;
};

/**
 * Event that changes the orientation of the indicated node
 */
struct NodeOrientationChangeEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Node to move to rotate to `targetOrientation`
   */
  uint32_t nodeId = 0;

  /**
   * The new orientation of the Node after this event has fired.
   *
   * Note: each axis is rotated independently (the x rotation is applied, then y, then z)
   * rather than combining all three and then rotating.
   */
  osg::Vec3d targetOrientation;
};

/**
 * Event that changes the orientation of the indicated Decoration
 */
struct DecorationOrientationChangeEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Decoration to move to rotate to `targetOrientation`
   */
  uint32_t decorationId = 0;

  /**
   * The new orientation of the Decoration after this event has fired.
   */
  osg::Vec3d targetOrientation;
};

/**
 * Event that appends a value to an existing series
 */
struct XYSeriesAddValue {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The series to append the value to
   */
  uint32_t seriesId = 0u;

  /**
   * The x value to append to the series
   */
  double x = 0.0;

  /**
   * The y value to append to the series
   */
  double y = 0.0;
};

/**
 * Variant defined for every event model
 */
using Event = std::variant<MoveEvent, DecorationMoveEvent, NodeOrientationChangeEvent, DecorationOrientationChangeEvent,
                           XYSeriesAddValue>;

/**
 * Events specific to Nodes
 */
using NodeEvent = std::variant<MoveEvent, NodeOrientationChangeEvent>;

/**
 * Events specific to decorations
 */
using DecorationEvent = std::variant<DecorationMoveEvent, DecorationOrientationChangeEvent>;

/**
 * Event types specific to the charts model
 */
using ChartEvent = std::variant<XYSeriesAddValue>;

} // namespace visualization
