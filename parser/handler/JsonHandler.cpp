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

#include "JsonHandler.h"
#include "Json.h"
#include "iostream"
#include <algorithm>
#include <cmath>
#include <exception>
#include <sstream>

using int_type = util::json::JsonValue::int_type;
using unsigned_int_type = util::json::JsonValue::unsigned_int_type;
const long long msToNsFactor = 1'000'000LL;

namespace {

class MissingRequiredFieldException : public std::exception {
  std::string message;

public:
  explicit MissingRequiredFieldException(const std::string &fieldName)
      : message{"Missing required field: " + fieldName} {
  }

  explicit MissingRequiredFieldException(const std::vector<std::string> &fieldNames) {
    std::stringstream ss;
    ss << "Object must have at least one of the follow fields: ";
    for (const auto &field : fieldNames)
      ss << field << ", ";

    message = ss.str();
    // Remove the ',' and ' ' character at the end
    message.erase(message.size() - 2, 2);
  }

  [[nodiscard]] const char *what() const noexcept override {
    return message.c_str();
  }
};

void requiredFields(const util::json::JsonObject &object, const std::vector<std::string> &fields) {
  for (const auto &field : fields) {
    if (!object.contains(field))
      throw MissingRequiredFieldException{field};
  }
}

void atLeastOneRequiredFields(const util::json::JsonObject &object, const std::vector<std::string> &fields) {
  for (const auto &field : fields) {
    if (object.contains(field))
      return;
  }

  throw MissingRequiredFieldException{fields};
}

// Copy of the palette from ns-3
// duplicated here until I find a better
// place for it
const parser::Ns3Color3 RED{245u, 61u, 0u};
const parser::Ns3Color3 DARK_RED{204u, 51u, 0u};

const parser::Ns3Color3 GREEN{9u, 232u, 94u};
const parser::Ns3Color3 DARK_GREEN{6u, 177u, 72u};

const parser::Ns3Color3 BLUE{37u, 137u, 189u};
const parser::Ns3Color3 DARK_BLUE{27u, 98u, 136u};

const parser::Ns3Color3 ORANGE{255u, 167u, 51u};
const parser::Ns3Color3 DARK_ORANGE{245u, 139u, 0u};

const parser::Ns3Color3 YELLOW{255u, 227u, 71u};
const parser::Ns3Color3 DARK_YELLOW{255u, 218u, 10u};

const parser::Ns3Color3 PURPLE{120u, 41u, 163u};
const parser::Ns3Color3 DARK_PURPLE{84u, 29u, 114u};

const parser::Ns3Color3 PINK{255u, 92u, 176u};
const parser::Ns3Color3 DARK_PINK{255u, 51u, 156u};

const parser::Ns3Color3 BLACK{7u, 6u, 0u};

const parser::Ns3Color3 WHITE{255u, 255u, 255u};

std::vector<parser::Ns3Color3> COLOR_PALETTE{BLUE,        RED,       GREEN,    ORANGE,     YELLOW,      PURPLE,
                                             PINK,        DARK_BLUE, DARK_RED, DARK_GREEN, DARK_ORANGE, DARK_YELLOW,
                                             DARK_PURPLE, DARK_PINK, BLACK,    WHITE};

parser::Ns3Color3 nextTrailColor(std::size_t &nextIndex) {
  const auto color = COLOR_PALETTE[nextIndex];

  nextIndex++;
  if (nextIndex == COLOR_PALETTE.size())
    nextIndex = 0;

  return color;
}

parser::nanoseconds getTimeCompatible(const util::json::JsonObject &object) {
  atLeastOneRequiredFields(object, {"milliseconds", "nanoseconds"});
  parser::nanoseconds time;

  if (object.contains("milliseconds")) {
    time = object["milliseconds"].get<int_type>() * msToNsFactor;
    return time;
  }

  return object["nanoseconds"].get<int_type>();
}

} // namespace

parser::ValueAxis::BoundMode boundModeFromString(const std::string &mode) {
  if (mode == "fixed")
    return parser::ValueAxis::BoundMode::Fixed;
  else if (mode == "highest value")
    return parser::ValueAxis::BoundMode::HighestValue;

  std::cerr << "Unsupported ValueAxis::BoundMode mode '" << mode << "'. Using 'HighestValue' instead\n";
  return parser::ValueAxis::BoundMode::HighestValue;
}

parser::ValueAxis::Scale scaleFromString(const std::string &mode) {
  if (mode == "linear")
    return parser::ValueAxis::Scale::Linear;
  else if (mode == "logarithmic")
    return parser::ValueAxis::Scale::Logarithmic;

  std::cerr << "Unsupported ValueAxis::Scale mode '" << mode << "'. Using 'Linear' instead\n";
  return parser::ValueAxis::Scale::Linear;
}

parser::ValueAxis valueAxisFromObject(const util::json::JsonObject &object) {
  requiredFields(object, {"bound-mode", "max", "min", "name", "scale"});

  parser::ValueAxis axis;
  axis.boundMode = boundModeFromString(object["bound-mode"].get<std::string>());
  axis.max = object["max"].get<double>();
  axis.min = object["min"].get<double>();
  axis.name = object["name"].get<std::string>();
  axis.scale = scaleFromString(object["scale"].get<std::string>());

  return axis;
}

parser::CategoryAxis categoryAxisFromObject(const util::json::JsonObject &object) {
  requiredFields(object, {"name", "values", "name"});

  parser::CategoryAxis axis;
  axis.name = object["name"].get<std::string>();

  auto &values = object["values"].array();
  for (auto &value : values) {
    requiredFields(value.object(), {"id", "value"});
    parser::CategoryAxis::Category category;

    category.id = value.object()["id"].get<unsigned int>();
    category.name = value.object()["value"].get<std::string>();

    axis.values.emplace_back(category);
  }

  // Category values must be sorted least to greatest
  // since we may only define a category's end position
  std::sort(axis.values.begin(), axis.values.end(), [](const auto &left, const auto &right) {
    return left.id < right.id;
  });

  return axis;
}

parser::Area::DrawMode drawModeFromString(const std::string &mode) {
  if (mode == "solid")
    return parser::Area::DrawMode::Solid;
  else if (mode == "hidden")
    return parser::Area::DrawMode::Hidden;

  std::cerr << "Unsupported Area::DrawMode mode '" << mode << "'. Using 'Solid' instead\n";
  return parser::Area::DrawMode::Solid;
}

parser::Ns3Color3 colorFromObject(const util::json::JsonObject &object) {
  requiredFields(object, {"red", "green", "blue"});

  parser::Ns3Color3 color;

  color.red = object["red"].get<uint8_t>();
  color.green = object["green"].get<uint8_t>();
  color.blue = object["blue"].get<uint8_t>();

  return color;
}

constexpr JsonHandler::Section JsonHandler::isSection(std::string_view key) {

  if (key == "areas")
    return Section::Areas;
  else if (key == "buildings")
    return Section::Buildings;
  else if (key == "configuration")
    return Section::Configuration;
  else if (key == "decorations")
    return Section::Decorations;
  else if (key == "events")
    return Section::Events;
  else if (key == "links")
    return Section::Links;
  else if (key == "nodes")
    return Section::Nodes;
  else if (key == "series")
    return Section::Series;
  else if (key == "streams")
    return Section::Streams;
  else
    return Section::None;
}

void JsonHandler::do_parse(JsonHandler::Section section, const util::json::JsonObject &object) {
  switch (section) {
  case Section::Areas:
    parseArea(object);
    break;
  case Section::Buildings:
    parseBuilding(object);
    break;
  case Section::Configuration:
    parseConfiguration(object);
    break;
  case Section::Decorations:
    parseDecoration(object);
    break;
  case Section::Events: {
    auto type = object["type"].get<std::string>();
    if (type == "node-position")
      parseMoveEvent(object);
    else if (type == "node-model-change")
      parseNodeModelChangeEvent(object);
    else if (type == "node-orientation")
      parseNodeOrientationEvent(object);
    else if (type == "node-color")
      parseNodeColorChangeEvent(object);
    else if (type == "node-transmit")
      parseTransmitEvent(object);
    else if (type == "decoration-position")
      parseDecorationMoveEvent(object);
    else if (type == "decoration-orientation")
      parseDecorationOrientationEvent(object);
    else if (type == "xy-series-append")
      parseSeriesAppend(object);
    else if (type == "xy-series-append-array")
      parseSeriesAppendArray(object);
    else if (type == "xy-series-clear")
      parseSeriesClear(object);
    else if (type == "category-series-append")
      parseCategorySeriesAppend(object);
    else if (type == "stream-append")
      parseStreamAppend(object);
    else if (type == "logical-link-create")
      parseLogicalLinkCreate(object);
    else if (type == "logical-link-update")
      parseLogicalLinkUpdate(object);
    else
      std::cerr << "Unhandled Event type: " << type << '\n';
  } break;
  case Section::Links:
    if (object["type"].get<std::string>() == "point-to-point")
      parseP2PLink(object);
    else if (object["type"].get<std::string>() == "logical")
      parseLogicalLink(object);
    else
      std::cerr << "Unknown link type '" << object["type"].get<std::string>() << "' Ignoring\n";
    break;
  case Section::Nodes:
    parseNode(object);
    break;
  case Section::Series: {
    auto type = object["type"].get<std::string>();
    if (type == "xy-series")
      parseXYSeries(object);
    else if (type == "category-value-series")
      parseCategoryValueSeries(object);
    else if (type == "series-collection")
      parseSeriesCollection(object);
    else
      std::cerr << "Unhandled Series type: " << type << '\n';
  } break;
  case Section::Streams:
    parseLogStream(object);
    break;
  default:
    std::cerr << "Unidentified Section key passed to do_parse: " << static_cast<int>(section) << '\n';
    break;
  }
}

void JsonHandler::parseConfiguration(const util::json::JsonObject &object) {
  // If you add Object types, there's some additional code in `JsonHandler::EndObject`
  // Specific to the configuration
  requiredFields(object, {"module-version"});

  auto &config = fileParser.globalConfiguration;

  const auto &jsonVersion = object["module-version"].object();
  requiredFields(object["module-version"].object(), {"major", "minor", "patch"});

  config.moduleVersion.major = jsonVersion["major"].get<long>();
  config.moduleVersion.minor = jsonVersion["minor"].get<long>();
  config.moduleVersion.patch = jsonVersion["patch"].get<long>();

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  if (object.contains("max-time-ms"))
    config.endTime = object["max-time-ms"].get<int_type>() * msToNsFactor;
  else if (object.contains("max-time"))
    config.endTime = object["max-time"].get<int_type>();

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  if (jsonVersion.contains("suffix"))
    config.moduleVersion.suffix = jsonVersion["suffix"].get<std::string>();

  if (object.contains("time-step")) {
    // TODO: compatibility with v1.0.0, remove for v1.1.0
    // time-step converted to an object in 1.0.4+
    if (object["time-step"].isObject()) {
      const auto timeStepObject = object["time-step"].object();
      requiredFields(timeStepObject, {"increment", "granularity"});
      config.timeStep = timeStepObject["increment"].get<int_type>();
      config.granularity = timeStepObject["granularity"].get<std::string>();
    } else
      config.timeStep = object["time-step"].get<int>() * msToNsFactor;
  }
}

void JsonHandler::parseNode(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "name", "model", "scale", "orientation", "visible", "position"});
  parser::Node node;

  node.id = object["id"].get<unsigned int>();
  node.name = object["name"].get<std::string>();

  // TODO: Compatability with v1.0.0, remove for v1.1.0
  // This is required 1.0.6+
  if (object.contains("label-enabled"))
    node.labelEnabled = object["label-enabled"].get<bool>();
  else
    node.labelEnabled = true;

  node.model = object["model"].get<std::string>();

  if (object["scale"].isObject()) {
    requiredFields(object["scale"].object(), {"x", "y", "z"});
    node.scale[0] = object["scale"].object()["x"].get<float>();
    node.scale[1] = object["scale"].object()["y"].get<float>();
    node.scale[2] = object["scale"].object()["z"].get<float>();
  } else {
    // TODO: compatibility with v1.0.0, remove for v1.1.0
    node.scale.fill(object["scale"].get<float>());
  }

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  // Moved to "target-scale" in 1.0.4+
  if (object.contains("height")) {
    node.height = object["height"].get<double>();
  }

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  // This is required 1.0.4+
  if (object.contains("target-scale")) {
    const auto &o = object["target-scale"].object();
    requiredFields(o, {"keep-ratio"});

    node.keepRatio = o["keep-ratio"].get<bool>();

    if (o.contains("height"))
      node.height = o["height"].get<double>();

    if (o.contains("width"))
      node.width = o["width"].get<double>();

    if (o.contains("depth"))
      node.depth = o["depth"].get<double>();
  }

  requiredFields(object["orientation"].object(), {"x", "y", "z"});
  node.orientation[0] = object["orientation"].object()["x"].get<double>();
  node.orientation[1] = object["orientation"].object()["y"].get<double>();
  node.orientation[2] = object["orientation"].object()["z"].get<double>();

  node.visible = object["visible"].get<bool>();

  requiredFields(object["position"].object(), {"x", "y", "z"});
  node.position.x = object["position"].object()["x"].get<double>();
  node.position.y = object["position"].object()["y"].get<double>();
  node.position.z = object["position"].object()["z"].get<double>();

  if (object.contains("offset")) {
    requiredFields(object["offset"].object(), {"x", "y", "z"});
    node.offset.x = object["offset"].object()["x"].get<double>();
    node.offset.y = object["offset"].object()["y"].get<double>();
    node.offset.z = object["offset"].object()["z"].get<double>();
  }

  if (object.contains("base-color"))
    node.baseColor = colorFromObject(object["base-color"].object());

  if (object.contains("highlight-color"))
    node.highlightColor = colorFromObject(object["highlight-color"].object());

  // TODO: Compatability with v1.0.0, remove for v1.1.0
  // This is required 1.0.6+
  if (object.contains("trail-enabled"))
    node.trailEnabled = object["trail-enabled"].get<bool>();
  else
    node.trailEnabled = false;

  // TODO: Compatability with v1.0.0, remove for v1.1.0
  // This is required 1.0.4+
  if (object.contains("trail-color"))
    node.trailColor = colorFromObject(object["trail-color"].object());
  else
    node.trailColor = node.baseColor.value_or(node.highlightColor.value_or(nextTrailColor(nextColorIndex)));

  updateLocationBounds(node.position);

  fileParser.nodes.emplace_back(node);
}

void JsonHandler::parseBuilding(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "color", "visible", "floors", "bounds"});
  parser::Building building;

  building.id = object["id"].get<int>();

  building.color = colorFromObject(object["color"].object());

  building.visible = object["visible"].get<bool>();
  building.floors = object["floors"].get<int>();

  building.roomsX = object["rooms"].object()["x"].get<int>();
  building.roomsY = object["rooms"].object()["y"].get<int>();

  requiredFields(object["bounds"].object()["x"].object(), {"min", "max"});
  requiredFields(object["bounds"].object()["y"].object(), {"min", "max"});
  requiredFields(object["bounds"].object()["z"].object(), {"min", "max"});

  building.min.x = object["bounds"].object()["x"].object()["min"].get<double>();
  building.min.y = object["bounds"].object()["y"].object()["min"].get<double>();
  building.min.z = object["bounds"].object()["z"].object()["min"].get<double>();

  building.max.x = object["bounds"].object()["x"].object()["max"].get<double>();
  building.max.y = object["bounds"].object()["y"].object()["max"].get<double>();
  building.max.z = object["bounds"].object()["z"].object()["max"].get<double>();

  updateLocationBounds(building.min);
  updateLocationBounds(building.max);

  fileParser.buildings.emplace_back(building);
}

void JsonHandler::parseDecoration(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "model", "position", "orientation", "scale"});
  parser::Decoration decoration;

  decoration.id = object["id"].get<unsigned int>();
  decoration.model = object["model"].get<std::string>();

  requiredFields(object["position"].object(), {"x", "y", "z"});
  decoration.position.x = object["position"].object()["x"].get<double>();
  decoration.position.y = object["position"].object()["y"].get<double>();
  decoration.position.z = object["position"].object()["z"].get<double>();

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  // Moved to "target-scale" in 1.0.4+
  if (object.contains("height")) {
    decoration.height = object["height"].get<double>();
  }

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  // This is required 1.0.4+
  if (object.contains("target-scale")) {
    const auto &o = object["target-scale"].object();
    requiredFields(o, {"keep-ratio"});

    decoration.keepRatio = o["keep-ratio"].get<bool>();

    if (o.contains("height"))
      decoration.height = o["height"].get<double>();

    if (o.contains("width"))
      decoration.width = o["width"].get<double>();

    if (o.contains("depth"))
      decoration.depth = o["depth"].get<double>();
  }

  updateLocationBounds(decoration.position);

  requiredFields(object["orientation"].object(), {"x", "y", "z"});
  decoration.orientation[0] = object["orientation"].object()["x"].get<double>();
  decoration.orientation[1] = object["orientation"].object()["y"].get<double>();
  decoration.orientation[2] = object["orientation"].object()["z"].get<double>();

  if (object["scale"].isObject()) {
    requiredFields(object["scale"].object(), {"x", "y", "z"});
    decoration.scale[0] = object["scale"].object()["x"].get<float>();
    decoration.scale[1] = object["scale"].object()["y"].get<float>();
    decoration.scale[2] = object["scale"].object()["z"].get<float>();
  } else {
    // TODO: compatibility with v1.0.0, remove for v1.1.0
    decoration.scale.fill(object["scale"].get<float>());
  }

  fileParser.decorations.emplace_back(decoration);
}

void JsonHandler::parseArea(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "name", "height", "fill-mode", "fill-color", "border-mode", "border-color"});
  parser::Area area;

  area.id = object["id"].get<int>();
  area.name = object["name"].get<std::string>();
  area.height = object["height"].get<double>();

  for (auto &point : object["points"].array()) {
    requiredFields(point.object(), {"x", "y"});
    parser::Ns3Coordinate coordinate;

    coordinate.x = point.object()["x"].get<double>();
    coordinate.y = point.object()["y"].get<double>();

    // An area is 2D, so each point shares the same
    // height, included with the coordinates for
    // convenience
    coordinate.z = area.height;

    area.points.emplace_back(coordinate);
  }

  area.fillMode = drawModeFromString(object["fill-mode"].get<std::string>());
  area.fillColor = colorFromObject(object["fill-color"].object());

  area.borderMode = drawModeFromString(object["border-mode"].get<std::string>());
  area.borderColor = colorFromObject(object["border-color"].object());

  fileParser.areas.emplace_back(area);
}

void JsonHandler::parseP2PLink(const util::json::JsonObject &object) {
  requiredFields(object, {"node-ids"});
  parser::WiredLink link;

  const auto &nodes = object["node-ids"].array();

  if (nodes.size() != 2u) {
    std::cerr << "Error: Links of type 'point-to-point' must have exactly 2 nodes, got: " << nodes.size()
              << " Ignoring.\n";
    return;
  }

  for (const auto &nodeId : nodes) {
    link.nodes.emplace_back(nodeId.get<unsigned int>());
  }

  fileParser.wiredLinks.emplace_back(link);
}

void JsonHandler::parseLogicalLink(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "color", "active", "nodes"});
  parser::LogicalLink link;

  link.id = object["id"].get<unsigned_int_type>();
  link.color = colorFromObject(object["color"].object());
  link.active = object["active"].get<bool>();

  const auto &nodes = object["nodes"].array();
  if (nodes.size() != 2u) {
    std::cerr << "Error: Links of type 'logical' must have exactly 2 nodes, got: " << nodes.size()
              << " Ignoring.\n";
    return;
  }
  link.nodes = {nodes[0].get<unsigned_int_type>(), nodes[1].get<unsigned_int_type>()};

  fileParser.logicalLinks.emplace_back(link);
}

void JsonHandler::parseMoveEvent(const util::json::JsonObject &object) {
  // TODO: add "time" after 1.1.0
  requiredFields(object, {"id", "x", "y", "z"});
  parser::MoveEvent event;

  event.nodeId = object["id"].get<int>();
  event.time = getTimeCompatible(object);
  event.targetPosition.x = object["x"].get<double>();
  event.targetPosition.y = object["y"].get<double>();
  event.targetPosition.z = object["z"].get<double>();

  updateLocationBounds(event.targetPosition);

  updateEndTime(event.time);
  processEndTransmits(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseNodeModelChangeEvent(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "nanoseconds", "model"});
  parser::NodeModelChangeEvent event;

  event.nodeId = object["id"].get<unsigned_int_type>();
  event.time = object["nanoseconds"].get<int_type>();
  event.model = object["model"].get<std::string>();

  updateEndTime(event.time);
  processEndTransmits(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseTransmitEvent(const util::json::JsonObject &object) {
  // TODO: add "time" after 1.1.0
  requiredFields(object, {"id", "duration", "target-size"});
  parser::TransmitEvent event;

  event.nodeId = object["id"].get<int>();
  event.time = getTimeCompatible(object);
  event.duration = object["duration"].get<int_type>();

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  // Check to see if this object was specified in milliseconds,
  // or nanoseconds
  if (object.contains("milliseconds"))
    event.duration *= msToNsFactor;

  event.targetSize = object["target-size"].get<double>();
  event.color = colorFromObject(object["color"].object());

  updateEndTime(event.time);
  processEndTransmits(event.time);

  // End any previous transmits by this Node
  // Even if they're incomplete
  const auto &transmittingIter = transmittingNodes.find(event.nodeId);
  if (transmittingIter != transmittingNodes.end() && transmittingIter->second.has_value()) {
    parser::TransmitEndEvent endEvent;
    endEvent.time = event.time;
    endEvent.startEvent = transmittingIter->second.value();
    fileParser.sceneEvents.emplace_back(endEvent);
  }
  transmittingNodes[event.nodeId] = event;
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationMoveEvent(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"id", "x", "y", "z"});
  parser::DecorationMoveEvent event;

  event.decorationId = object["id"].get<int>();
  event.time = getTimeCompatible(object);
  event.targetPosition.x = object["x"].get<double>();
  event.targetPosition.y = object["y"].get<double>();
  event.targetPosition.z = object["z"].get<double>();

  updateLocationBounds(event.targetPosition);

  updateEndTime(event.time);
  processEndTransmits(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseNodeOrientationEvent(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"id", "x", "y", "z"});
  parser::NodeOrientationChangeEvent event;

  event.nodeId = object["id"].get<int>();
  event.time = getTimeCompatible(object);
  event.targetOrientation[0] = object["x"].get<double>();
  event.targetOrientation[1] = object["y"].get<double>();
  event.targetOrientation[2] = object["z"].get<double>();

  updateEndTime(event.time);
  processEndTransmits(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationOrientationEvent(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"id", "x", "y", "z"});
  parser::DecorationOrientationChangeEvent event;

  event.decorationId = object["id"].get<int>();
  event.time = getTimeCompatible(object);
  event.targetOrientation[0] = object["x"].get<double>();
  event.targetOrientation[1] = object["y"].get<double>();
  event.targetOrientation[2] = object["z"].get<double>();

  updateEndTime(event.time);
  processEndTransmits(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseNodeColorChangeEvent(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"id", "color-type"});
  parser::NodeColorChangeEvent event;

  event.nodeId = object["id"].get<unsigned int>();
  event.time = getTimeCompatible(object);

  auto type = object["color-type"].get<std::string>();
  if (type == "base")
    event.type = parser::NodeColorChangeEvent::ColorType::Base;
  else if (type == "highlight")
    event.type = parser::NodeColorChangeEvent::ColorType::Highlight;
  else
    std::cerr << "Error: unhandled 'color-type': \"" << type << "\" in `NodeColorChangeEvent`\n";

  if (object.contains("color"))
    event.targetColor = colorFromObject(object["color"].object());

  updateEndTime(event.time);
  processEndTransmits(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseSeriesAppend(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"series-id", "x", "y"});
  parser::XYSeriesAddValue event;

  event.time = getTimeCompatible(object);
  event.seriesId = object["series-id"].get<int>();
  event.point.x = object["x"].get<double>();
  event.point.y = object["y"].get<double>();

  updateEndTime(event.time);
  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseSeriesAppendArray(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"series-id", "points"});
  parser::XYSeriesAddValues event;

  event.time = getTimeCompatible(object);
  event.seriesId = object["series-id"].get<int>();

  auto points = object["points"].array();

  // Ignore events with empty point arrays
  if (points.empty()) {
    std::cerr << "Ignoring empty `xy-series-append-array` event\n";
    return;
  }

  for (const auto &point : points) {
    event.points.emplace_back(parser::XYPoint{point.object()["x"].get<double>(), point.object()["y"].get<double>()});
  }

  updateEndTime(event.time);
  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseSeriesClear(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"series-id"});
  parser::XYSeriesClear event;

  event.time = getTimeCompatible(object);
  event.seriesId = object["series-id"].get<int>();

  updateEndTime(event.time);
  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseCategorySeriesAppend(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"series-id", "category", "value"});
  parser::CategorySeriesAddValue event;

  event.time = getTimeCompatible(object);
  event.seriesId = object["series-id"].get<int>();
  event.category = object["category"].get<int>();
  event.value = object["value"].get<double>();

  updateEndTime(event.time);
  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseXYSeries(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "name", "legend", "visible", "color", "connection", "labels", "x-axis", "y-axis"});
  parser::XYSeries series;

  series.id = object["id"].get<int>();
  series.name = object["name"].get<std::string>();

  series.legend = object["legend"].get<std::string>();
  series.visible = object["visible"].get<bool>();

  series.color = colorFromObject(object["color"].object());

  auto connection = object["connection"].get<std::string>();
  if (connection == "none")
    series.connection = parser::XYSeries::Connection::None;
  else if (connection == "line")
    series.connection = parser::XYSeries::Connection::Line;
  else if (connection == "spline") {
    std::clog << "Warning: 'spline' connection type no longer supported, using 'line' for series ID: " << series.id
              << '\n';
    series.connection = parser::XYSeries::Connection::Line;
  } else
    std::cerr << "Unrecognized connection type: " << connection << '\n';

  auto labelMode = object["labels"].get<std::string>();
  if (labelMode == "hidden")
    series.labelMode = parser::XYSeries::LabelMode::Hidden;
  else if (labelMode == "shown")
    series.labelMode = parser::XYSeries::LabelMode::Shown;
  else
    std::cerr << "Unrecognized labels type: " << labelMode << '\n';

  series.xAxis = valueAxisFromObject(object["x-axis"].object());
  series.yAxis = valueAxisFromObject(object["y-axis"].object());

  fileParser.xySeries.emplace_back(series);
}

void JsonHandler::parseCategoryValueSeries(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "name", "legend", "visible", "color", "x-axis", "y-axis", "auto-update"});
  parser::CategoryValueSeries series;

  series.id = object["id"].get<int>();
  series.name = object["name"].get<std::string>();

  series.legend = object["legend"].get<std::string>();
  series.visible = object["visible"].get<bool>();

  series.color = colorFromObject(object["color"].object());

  series.xAxis = valueAxisFromObject(object["x-axis"].object());
  series.yAxis = categoryAxisFromObject(object["y-axis"].object());

  if (object["auto-update"].get<bool>()) {
    requiredFields(object, {"auto-update-interval", "auto-update-increment"});
    series.autoUpdate = true;

    const auto &autoUpdateInterval = object["auto-update-interval"];
    // TODO: compatibility with v1.0.0, remove for v1.1.0
    if (autoUpdateInterval.is<double>()) // Old times (in ms) were specified as a double
      series.autoUpdateInterval = static_cast<long long>(object["auto-update-interval"].get<double>()) * msToNsFactor;
    else
      series.autoUpdateInterval = object["auto-update-interval"].get<int_type>();

    series.autoUpdateIncrement = object["auto-update-increment"].get<double>();
  }

  fileParser.categoryValueSeries.emplace_back(series);
}

void JsonHandler::parseSeriesCollection(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "name", "child-series", "x-axis", "y-axis"});
  parser::SeriesCollection collection;
  collection.id = object["id"].get<int>();
  collection.name = object["name"].get<std::string>();

  auto childSeries = object["child-series"];
  for (const auto &child : childSeries.array()) {
    collection.series.emplace_back(child.get<int>());
  }

  collection.xAxis = valueAxisFromObject(object["x-axis"].object());
  collection.yAxis = valueAxisFromObject(object["y-axis"].object());
  fileParser.seriesCollections.emplace_back(collection);
}

void JsonHandler::parseLogStream(const util::json::JsonObject &object) {
  requiredFields(object, {"id", "name", "visible"});
  parser::LogStream stream;
  stream.id = object["id"].get<int>();
  stream.name = object["name"].get<std::string>();

  if (object.contains("color"))
    stream.color = colorFromObject(object["color"].object());

  stream.visible = object["visible"].get<bool>();

  fileParser.logStreams.emplace_back(stream);
}

void JsonHandler::parseStreamAppend(const util::json::JsonObject &object) {
  // TODO: "nanoseconds" field checked by `getTimeCompatible`, add here for 1.1.0+
  requiredFields(object, {"stream-id", "data"});
  parser::StreamAppendEvent event;
  event.time = getTimeCompatible(object);
  event.streamId = object["stream-id"].get<int>();
  event.value = object["data"].get<std::string>();

  updateEndTime(event.time);
  fileParser.logEvents.emplace_back(event);
}

void JsonHandler::parseLogicalLinkCreate(const util::json::JsonObject &object) {
  requiredFields(object, {"nanoseconds", "link-id", "nodes", "active", "color"});
  parser::LogicalLinkCreate event;
  event.time = object["nanoseconds"].get<int_type>();
  event.model.id = object["link-id"].get<unsigned_int_type>();

  const auto &nodes = object["nodes"].array();
  if (nodes.size() != 2u) {
    std::cerr << "Error: Links of type 'logical' must have exactly 2 nodes, got: " << nodes.size()
              << " Ignoring.\n";
    return;
  }
  event.model.nodes = {nodes[0].get<unsigned_int_type>(), nodes[1].get<unsigned_int_type>()};

  event.model.active = object["active"].get<bool>();
  event.model.color = colorFromObject(object["color"].object());

  updateEndTime(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseLogicalLinkUpdate(const util::json::JsonObject &object) {
  requiredFields(object, {"nanoseconds", "link-id", "nodes", "active", "color"});
  parser::LogicalLinkUpdate event;
  event.time = object["nanoseconds"].get<int_type>();
  event.id = object["link-id"].get<unsigned_int_type>();

  const auto &nodes = object["nodes"].array();
  if (nodes.size() != 2u) {
    std::cerr << "Error: Links of type 'logical' must have exactly 2 nodes, got: " << nodes.size()
              << " Ignoring.\n";
    return;
  }
  event.nodes = {nodes[0].get<unsigned_int_type>(), nodes[1].get<unsigned_int_type>()};

  event.active = object["active"].get<bool>();
  event.color = colorFromObject(object["color"].object());

  updateEndTime(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::updateLocationBounds(const parser::Ns3Coordinate &coordinate) {
  auto &config = fileParser.globalConfiguration;

  if (coordinate.x < config.minLocation.x)
    config.minLocation.x = coordinate.x;
  else if (coordinate.x > config.maxLocation.x)
    config.maxLocation.x = coordinate.x;

  if (coordinate.y < config.minLocation.y)
    config.minLocation.y = coordinate.y;
  else if (coordinate.y > config.maxLocation.y)
    config.maxLocation.y = coordinate.y;

  if (coordinate.z < config.minLocation.z)
    config.minLocation.z = coordinate.z;
  else if (coordinate.z > config.maxLocation.z)
    config.maxLocation.z = coordinate.z;
}

void JsonHandler::updateEndTime(parser::nanoseconds time) {
  if (time > fileParser.globalConfiguration.endTime)
    fileParser.globalConfiguration.endTime = time;
}

void JsonHandler::processEndTransmits(parser::nanoseconds time) {
  for (auto &[nodeId, transmitEvent] : transmittingNodes) {
    if (!transmitEvent.has_value())
      return;

    const auto endTransmitTime = transmitEvent.value().time + transmitEvent.value().duration;
    if (time < endTransmitTime)
      return;

    parser::TransmitEndEvent endEvent;
    endEvent.time = time;
    endEvent.startEvent = transmitEvent.value();
    endEvent.nodeId = endEvent.startEvent.nodeId;
    fileParser.sceneEvents.emplace_back(endEvent);

    transmitEvent.reset();
  }
}

JsonHandler::JsonHandler(parser::FileParser &parser) : fileParser(parser) {
}

bool JsonHandler::Null() {
  handle(nullptr);
  return true;
}

bool JsonHandler::Bool(bool value) {
  handle(value);
  return true;
}

bool JsonHandler::Int(int value) {
  handle(value);
  return true;
}

bool JsonHandler::Uint(unsigned int value) {
  handle(value);
  return true;
}

bool JsonHandler::Int64(std::int64_t value) {
  handle(value);
  return true;
}

bool JsonHandler::Uint64(std::uint64_t value) {
  handle(value);
  return true;
}

bool JsonHandler::Double(double value) {
  handle(value);
  return true;
}

bool JsonHandler::String(const char *value, rapidjson::SizeType length, bool) {
  handle(std::string(value, length));
  return true;
}

bool JsonHandler::StartObject() {
  // Root object case
  if (jsonStack.empty()) {
    jsonStack.push({"root", util::json::JsonObject()});
    return true;
  }

  auto &top = jsonStack.top();

  // Do not overwrite existing values
  if (top.value.isArray()) {
    jsonStack.push({"", util::json::JsonObject()});
    return true;
  }

  top.value = util::json::JsonObject();
  return true;
}

bool JsonHandler::EndObject(rapidjson::SizeType) {
  // TODO: Error
  if (jsonStack.empty()) {
    return false;
  }

  auto oldTop = jsonStack.top();
  jsonStack.pop();

  if (jsonStack.empty()) {
    assert(oldTop.key == "root");
    return true;
  }

  auto &currentTop = jsonStack.top();

  // Parsing
  try {
    // Special case, "configuration" is parsed
    // as a normal unit
    if (currentSection == Section::Configuration) {

      if (oldTop.key == "module-version") {
        currentTop.value.object().insert("module-version", oldTop.value);
        return true;
      } else if (oldTop.key == "time-step") {
        currentTop.value.object().insert("time-step", oldTop.value);
        return true;
      } else if (oldTop.key == "configuration") {
        parseConfiguration(oldTop.value.object());
        return true;
      }
      return false;
    }

    // All other sections have one parse call per item
    if (isSection(jsonStack.top().key) != Section::None) {
      do_parse(currentSection, oldTop.value.object());
      return true;
    }
  } catch (const MissingRequiredFieldException &e) {
    fileParser.errorMessage = e.what();
    return false;
  }

  if (currentTop.value.isArray()) {
    currentTop.value.array().emplace_back(oldTop.value);
    return true;
  }

  if (currentTop.value.isObject()) {
    currentTop.value.object().insert(oldTop.key, oldTop.value);
    return true;
  }

  // Shouldn't hit this point, but just in case
  fileParser.errorMessage = "Unknown parsing error in `EndObject()`";
  return false;
}

bool JsonHandler::StartArray() {
  if (jsonStack.empty()) {
    return false;
  }

  jsonStack.top().value = util::json::JsonArray();
  return true;
}

bool JsonHandler::EndArray(rapidjson::SizeType) {
  auto oldTop = jsonStack.top();
  jsonStack.pop();

  auto &currentTop = jsonStack.top();
  if (currentTop.value.isObject())
    currentTop.value.object().insert(oldTop.key, oldTop.value);
  else if (currentTop.value.isArray())
    currentTop.value.array().emplace_back(oldTop.value);
  else {
    std::cerr << currentTop.key << " is not of a root type\n";
    std::abort();
  }

  return true;
}

bool JsonHandler::Key(const char *value, rapidjson::SizeType length, bool) {
  jsonStack.push({std::string(value, length)});

  // Only Check for sections for keys immediately
  // descending from the root object.
  // 2 deep so the stack looks like:
  // ----------------
  // |  current key |
  // ----------------
  // |     root     |
  // ----------------
  if (jsonStack.size() != 2u)
    return true;
  auto possibleSection = isSection(value);
  if (possibleSection != Section::None) {
    currentSection = possibleSection;
  }
  return true;
}
