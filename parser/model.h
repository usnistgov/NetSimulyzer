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
#include <array>
#include <cstdint>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace parser {

struct Ns3Coordinate {
  float x = 0.0f;
  float y = 0.0f;
  float z = 0.0f;
};

struct Ns3Color3 {
  uint8_t red = 0u;
  uint8_t green = 0u;
  uint8_t blue = 0u;
};

struct Ns3ModuleVersion {
  long major;
  long minor;
  long patch;
  std::string suffix;
};

struct GlobalConfiguration {
  Ns3ModuleVersion moduleVersion;
  double endTime = 0.0;
  std::optional<int> timeStep;
  Ns3Coordinate minLocation;
  Ns3Coordinate maxLocation;
};

// ----- Scene Models -----

struct Node {
  unsigned int id = 0;
  std::string name;
  std::string model;
  std::array<float, 3> scale{1.0f};
  bool keepRatio{true};
  std::optional<float> height;
  std::optional<float> width;
  std::optional<float> depth;
  bool visible = true;
  Ns3Coordinate position;
  Ns3Coordinate offset;
  std::optional<Ns3Color3> baseColor;
  std::optional<Ns3Color3> highlightColor;
  std::array<double, 3> orientation{0.0};
};

struct Building {
  unsigned int id = 0;
  Ns3Color3 color;
  bool visible = true;
  uint16_t floors = 0;
  uint16_t roomsX = 0;
  uint16_t roomsY = 0;
  Ns3Coordinate min;
  Ns3Coordinate max;
};

struct Decoration {
  unsigned int id;
  std::string model;
  Ns3Coordinate position;
  std::array<double, 3> orientation{0.0};
  bool keepRatio{true};
  std::optional<float> height;
  std::optional<float> width;
  std::optional<float> depth;
  std::array<float, 3> scale{1.0f};
};

struct Area {
  enum class DrawMode { Solid, Hidden };

  unsigned int id;
  std::string name;

  Ns3Color3 fillColor;
  DrawMode fillMode;

  Ns3Color3 borderColor;
  DrawMode borderMode;

  float height;
  std::vector<Ns3Coordinate> points;
};

struct WiredLink {
  std::vector<unsigned int> nodes;
};

// ----- Chart Models -----

struct ValueAxis {
  enum class BoundMode { Fixed, HighestValue };
  enum class Scale { Linear, Logarithmic };

  std::string name;
  BoundMode boundMode = BoundMode::HighestValue;
  Scale scale = Scale::Linear;
  double min = 0.0;
  double max = 0.0;
};

struct CategoryAxis {
  struct Category {
    unsigned int id = 0u;
    std::string name;
  };

  std::string name;
  std::vector<Category> values;
};

struct XYSeries {
  enum class Connection { None, Line, Spline };
  enum class LabelMode { Hidden, Shown };

  unsigned int id = 0u;
  bool visible;
  std::string name;
  std::string legend;
  Connection connection = Connection::Line;
  LabelMode labelMode = LabelMode::Shown;
  Ns3Color3 color;
  ValueAxis xAxis;
  ValueAxis yAxis;
};

struct CategoryValueSeries {
  enum class ConnectionMode { All, InCategory };

  unsigned int id = 0u;
  bool visible;
  bool autoUpdate = false;
  double autoUpdateInterval;
  double autoUpdateIncrement;
  std::string name;
  std::string legend;
  Ns3Color3 color;
  ValueAxis xAxis;
  CategoryAxis yAxis;
};

struct SeriesCollection {
  unsigned int id = 0u;
  std::string name;
  std::vector<uint32_t> series;
  ValueAxis xAxis;
  ValueAxis yAxis;
};

// ----- Log Models -----

struct LogStream {
  unsigned int id = 0u;
  bool visible = true;
  std::string name;
  std::optional<Ns3Color3> color;
};

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
   */
  Ns3Coordinate targetPosition;
};

/**
 * Event that indicates a Node has begun transmitting
 */
struct TransmitEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Node that triggered the event
   */
  uint32_t nodeId = 0;

  /**
   * How long the transmission sphere should
   * expand, in milliseconds
   */
  double duration = 50.0;

  /**
   * The size the transmission sphere should grow to,
   * by the time `time + duration` milliseconds have
   * passed. In ns-3 units
   */
  double targetSize = 2.0;

  /**
   * The color to use as the base color of
   * the transmission bubble
   */
  Ns3Color3 color;
};

/**
 * Event inserted by the parser when a transmission is supposed to end
 */
struct TransmitEndEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Node that triggered the event.
   * Included for consistency with other events
   */
  uint32_t nodeId = 0;

  /**
   * The event that started the transmission
   */
  TransmitEvent startEvent;
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
  Ns3Coordinate targetPosition;
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
  std::array<double, 3> targetOrientation{0.0};
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
  std::array<double, 3> targetOrientation{0.0};
};

/**
 * Event that changes (or unsets) a configurable color of a Node
 */
struct NodeColorChangeEvent {
  /**
   * The possible color types the event may affect.
   */
  enum class ColorType { Base, Highlight };

  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The Node to change the color of
   */
  unsigned int nodeId = 0u;

  /**
   * Which color type the change affects
   */
  ColorType type{ColorType::Base};

  /**
   * The color to change the Node to. If the color was unset, then
   * this optional is not set.
   */
  std::optional<Ns3Color3> targetColor;
};

/**
 * Convenience struct for packaging points.
 * Does not correspond to anything in the ns-3 module
 */
struct XYPoint {
  double x;
  double y;
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
   * The point to append to the series
   */
  XYPoint point;
};

/**
 * Event that appends several values to an existing series
 */
struct XYSeriesAddValues {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The series to append the values to
   */
  uint32_t seriesId = 0u;

  /**
   * The points to add to the series.
   * Should be added in order
   */
  std::vector<XYPoint> points;
};

/**
 * Event signaling that the series should be cleared
 */
struct XYSeriesClear {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The series to clear
   */
  uint32_t seriesId = 0u;
};

/**
 * Event that appends a value with a category to a series
 */
struct CategorySeriesAddValue {
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
   * The value to append to the series
   * on the X axis
   */
  double value = 0.0;

  /**
   * The ID of the category
   * to append `value` to
   * on the y axis
   */
  unsigned int category = 0u;
};

/**
 * Event that appends a message to a given LogStream
 */
struct StreamAppendEvent {
  /**
   * The simulation time (in milliseconds)
   * for when the event should be run
   */
  double time = 0.0;

  /**
   * The ID of the stream to append to
   */
  unsigned int streamId = 0u;

  /**
   * The string to append to the log
   */
  std::string value;
};

/**
 * Variant defined for every event model
 */
using Event = std::variant<MoveEvent, TransmitEvent, DecorationMoveEvent, NodeOrientationChangeEvent,
                           DecorationOrientationChangeEvent, XYSeriesAddValue, XYSeriesAddValues, XYSeriesClear,
                           StreamAppendEvent>;

/**
 * Events which affect the rendered scene
 */
using SceneEvent = std::variant<MoveEvent, TransmitEvent, TransmitEndEvent, NodeOrientationChangeEvent,
                                NodeColorChangeEvent, DecorationMoveEvent, DecorationOrientationChangeEvent>;

/**
 * Event types specific to the charts model
 */
using ChartEvent = std::variant<XYSeriesAddValue, XYSeriesAddValues, XYSeriesClear, CategorySeriesAddValue>;

/**
 * Events which affect the Scenario Log
 */
using LogEvent = std::variant<StreamAppendEvent>;

} // namespace parser
