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

parser::ValueAxis::BoundMode boundModeFromString(const std::string &mode) {
  if (mode == "fixed")
    return parser::ValueAxis::BoundMode::Fixed;
  else if (mode == "highest value")
    return parser::ValueAxis::BoundMode::HighestValue;
  else
    assert(!"Unhandled ValueAxis::BoundMode mode");
}

parser::ValueAxis::Scale scaleFromString(const std::string &mode) {
  if (mode == "linear")
    return parser::ValueAxis::Scale::Linear;
  else if (mode == "logarithmic")
    return parser::ValueAxis::Scale::Logarithmic;
  else
    assert(!"Unhandled ValueAxis::Scale mode");
}

parser::ValueAxis valueAxisFromObject(const util::json::JsonObject &object) {
  parser::ValueAxis axis;
  axis.boundMode = boundModeFromString(object["bound-mode"].get<std::string>());
  axis.max = object["max"].get<double>();
  axis.min = object["min"].get<double>();
  axis.name = object["name"].get<std::string>();
  axis.scale = scaleFromString(object["scale"].get<std::string>());

  return axis;
}

parser::CategoryAxis categoryAxisFromObject(const util::json::JsonObject &object) {
  parser::CategoryAxis axis;
  axis.name = object["name"].get<std::string>();

  auto &values = object["values"].array();
  for (auto &value : values) {
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
  else
    assert(!"Unhandled Area::DrawMode mode");
}

parser::Ns3Color3 colorFromObject(const util::json::JsonObject &object) {
  parser::Ns3Color3 color;

  color.red = object["red"].get<uint8_t>();
  color.green = object["green"].get<uint8_t>();
  color.blue = object["blue"].get<uint8_t>();

  return color;
}

constexpr JsonHandler::Section JsonHandler::isSection(std::string_view key) {
  {
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
    else if (key == "nodes")
      return Section::Nodes;
    else if (key == "series")
      return Section::Series;
    else if (key == "streams")
      return Section::Streams;
    else
      return Section::None;
  }
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
    else if (type == "node-orientation")
      parseNodeOrientationEvent(object);
    else if (type == "node-color")
      parseNodeColorChangeEvent(object);
    else if (type == "decoration-position")
      parseDecorationMoveEvent(object);
    else if (type == "decoration-orientation")
      parseDecorationOrientationEvent(object);
    else if (type == "xy-series-append")
      parseSeriesAppend(object);
    else if (type == "category-series-append")
      parseCategorySeriesAppend(object);
    else if (type == "stream-append")
      parseStreamAppend(object);
    else
      std::cerr << "Unhandled Event type: " << type << '\n';
  } break;
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
  auto &config = fileParser.globalConfiguration;

  const auto &jsonVersion = object["module-version"].object();
  config.moduleVersion.major = jsonVersion["major"].get<long>();
  config.moduleVersion.minor = jsonVersion["minor"].get<long>();
  config.moduleVersion.patch = jsonVersion["patch"].get<long>();

  // TODO: compatibility with v1.0.0, remove for v1.1.0
  if (jsonVersion.contains("suffix"))
    config.moduleVersion.suffix = jsonVersion["suffix"].get<std::string>();

  if (object.contains("time-step")) {
    config.timeStep = object["time-step"].get<int>();
  }
}

void JsonHandler::parseNode(const util::json::JsonObject &object) {
  parser::Node node;

  node.id = object["id"].get<unsigned int>();
  node.name = object["name"].get<std::string>();
  node.model = object["model"].get<std::string>();
  node.scale = object["scale"].get<double>();

  if (object.contains("height")) {
    node.height = object["height"].get<double>();
  }

  node.orientation[0] = object["orientation"].object()["x"].get<double>();
  node.orientation[1] = object["orientation"].object()["y"].get<double>();
  node.orientation[2] = object["orientation"].object()["z"].get<double>();

  node.visible = object["visible"].get<bool>();

  node.position.x = object["position"].object()["x"].get<double>();
  node.position.y = object["position"].object()["y"].get<double>();
  node.position.z = object["position"].object()["z"].get<double>();

  if (object.contains("offset")) {
    node.offset.x = object["offset"].object()["x"].get<double>();
    node.offset.y = object["offset"].object()["y"].get<double>();
    node.offset.z = object["offset"].object()["z"].get<double>();
  }

  if (object.contains("base-color"))
    node.baseColor = colorFromObject(object["base-color"].object());

  if (object.contains("highlight-color"))
    node.highlightColor = colorFromObject(object["highlight-color"].object());

  updateLocationBounds(node.position);

  fileParser.nodes.emplace_back(node);
}

void JsonHandler::parseBuilding(const util::json::JsonObject &object) {
  parser::Building building;

  building.id = object["id"].get<int>();

  building.color.red = object["color"].object()["red"].get<int>();
  building.color.green = object["color"].object()["green"].get<int>();
  building.color.blue = object["color"].object()["blue"].get<int>();

  building.visible = object["visible"].get<bool>();
  building.floors = object["floors"].get<int>();

  building.roomsX = object["rooms"].object()["x"].get<int>();
  building.roomsY = object["rooms"].object()["y"].get<int>();

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
  parser::Decoration decoration;

  decoration.id = object["id"].get<unsigned int>();
  decoration.model = object["model"].get<std::string>();

  decoration.position.x = object["position"].object()["x"].get<double>();
  decoration.position.y = object["position"].object()["y"].get<double>();
  decoration.position.z = object["position"].object()["z"].get<double>();

  if (object.contains("height")) {
    decoration.height = object["height"].get<double>();
  }

  updateLocationBounds(decoration.position);

  decoration.orientation[0] = object["orientation"].object()["x"].get<double>();
  decoration.orientation[1] = object["orientation"].object()["y"].get<double>();
  decoration.orientation[2] = object["orientation"].object()["z"].get<double>();

  decoration.scale = object["scale"].get<double>();

  fileParser.decorations.emplace_back(decoration);
}

void JsonHandler::parseArea(const util::json::JsonObject &object) {
  parser::Area area;

  area.id = object["id"].get<int>();
  area.name = object["name"].get<std::string>();
  area.height = object["height"].get<double>();

  for (auto &point : object["points"].array()) {
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

  area.fillColor.red = object["fill-color"].object()["red"].get<int>();
  area.fillColor.green = object["fill-color"].object()["green"].get<int>();
  area.fillColor.blue = object["fill-color"].object()["blue"].get<int>();

  area.borderMode = drawModeFromString(object["border-mode"].get<std::string>());

  area.borderColor.red = object["border-color"].object()["red"].get<int>();
  area.borderColor.green = object["border-color"].object()["green"].get<int>();
  area.borderColor.blue = object["border-color"].object()["blue"].get<int>();

  fileParser.areas.emplace_back(area);
}

void JsonHandler::parseMoveEvent(const util::json::JsonObject &object) {
  parser::MoveEvent event;

  event.nodeId = object["id"].get<int>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition.x = object["x"].get<double>();
  event.targetPosition.y = object["y"].get<double>();
  event.targetPosition.z = object["z"].get<double>();

  updateLocationBounds(event.targetPosition);

  updateEndTime(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationMoveEvent(const util::json::JsonObject &object) {
  parser::DecorationMoveEvent event;

  event.decorationId = object["id"].get<int>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition.x = object["x"].get<double>();
  event.targetPosition.y = object["y"].get<double>();
  event.targetPosition.z = object["z"].get<double>();

  updateLocationBounds(event.targetPosition);

  updateEndTime(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseNodeOrientationEvent(const util::json::JsonObject &object) {
  parser::NodeOrientationChangeEvent event;

  event.nodeId = object["id"].get<int>();
  event.time = object["milliseconds"].get<double>();
  event.targetOrientation[0] = object["x"].get<double>();
  event.targetOrientation[1] = object["y"].get<double>();
  event.targetOrientation[2] = object["z"].get<double>();

  updateEndTime(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationOrientationEvent(const util::json::JsonObject &object) {
  parser::DecorationOrientationChangeEvent event;

  event.decorationId = object["id"].get<int>();
  event.time = object["milliseconds"].get<double>();
  event.targetOrientation[0] = object["x"].get<double>();
  event.targetOrientation[1] = object["y"].get<double>();
  event.targetOrientation[2] = object["z"].get<double>();

  updateEndTime(event.time);
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseNodeColorChangeEvent(const util::json::JsonObject &object) {
  parser::NodeColorChangeEvent event;

  event.nodeId = object["id"].get<unsigned int>();
  event.time = object["milliseconds"].get<double>();

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
  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseSeriesAppend(const util::json::JsonObject &object) {
  parser::XYSeriesAddValue event;

  event.time = object["milliseconds"].get<double>();
  event.seriesId = object["series-id"].get<int>();
  event.x = object["x"].get<double>();
  event.y = object["y"].get<double>();

  updateEndTime(event.time);
  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseCategorySeriesAppend(const util::json::JsonObject &object) {
  parser::CategorySeriesAddValue event;

  event.time = object["milliseconds"].get<double>();
  event.seriesId = object["series-id"].get<int>();
  event.category = object["category"].get<int>();
  event.value = object["value"].get<double>();

  updateEndTime(event.time);
  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseXYSeries(const util::json::JsonObject &object) {
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
  else if (connection == "spline")
    series.connection = parser::XYSeries::Connection::Spline;
  else
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
  parser::CategoryValueSeries series;

  series.id = object["id"].get<int>();
  series.name = object["name"].get<std::string>();

  series.legend = object["legend"].get<std::string>();
  series.visible = object["visible"].get<bool>();

  series.color = colorFromObject(object["color"].object());

  series.xAxis = valueAxisFromObject(object["x-axis"].object());
  series.yAxis = categoryAxisFromObject(object["y-axis"].object());

  if (object["auto-update"].get<bool>()) {
    series.autoUpdate = true;
    series.autoUpdateInterval = object["auto-update-interval"].get<double>();
    series.autoUpdateIncrement = object["auto-update-increment"].get<double>();
  }

  fileParser.categoryValueSeries.emplace_back(series);
}

void JsonHandler::parseSeriesCollection(const util::json::JsonObject &object) {
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
  parser::LogStream stream;
  stream.id = object["id"].get<int>();
  stream.name = object["name"].get<std::string>();

  if (object.contains("color")) {
    parser::Ns3Color3 color{};
    color.blue = object["color"].object()["blue"].get<uint8_t>();
    color.green = object["color"].object()["green"].get<uint8_t>();
    color.red = object["color"].object()["red"].get<uint8_t>();

    stream.color = color;
  }

  stream.visible = object["visible"].get<bool>();

  fileParser.logStreams.emplace_back(stream);
}

void JsonHandler::parseStreamAppend(const util::json::JsonObject &object) {
  parser::StreamAppendEvent event;
  event.time = object["milliseconds"].get<double>();
  event.streamId = object["stream-id"].get<int>();
  event.value = object["data"].get<std::string>();

  updateEndTime(event.time);
  fileParser.logEvents.emplace_back(event);
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

void JsonHandler::updateEndTime(double milliseconds) {
  if (milliseconds > fileParser.globalConfiguration.endTime)
    fileParser.globalConfiguration.endTime = milliseconds;
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

  // Special case, "configuration" is parsed
  // as a normal unit
  if (currentSection == Section::Configuration) {

    if (oldTop.key == "module-version") {
      currentTop.value.object().insert("module-version", oldTop.value);
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

  if (currentTop.value.isArray()) {
    currentTop.value.array().emplace_back(oldTop.value);
    return true;
  }

  if (currentTop.value.isObject()) {
    currentTop.value.object().insert(oldTop.key, oldTop.value);
    return true;
  }

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

  auto possibleSection = isSection(value);
  if (possibleSection != Section::None) {
    currentSection = possibleSection;
  }
  return true;
}
