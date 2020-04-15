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

parser::ValueAxis valueAxisFromObject(const nlohmann::json &object) {
  parser::ValueAxis axis;
  axis.boundMode = boundModeFromString(object["bound-mode"].get<std::string>());
  axis.max = object["max"].get<double>();
  axis.min = object["min"].get<double>();
  axis.name = object["name"].get<std::string>();
  axis.scale = scaleFromString(object["scale"].get<std::string>());

  return axis;
}

constexpr JsonHandler::Section JsonHandler::isSection(std::string_view key) {
  {
    if (key == "buildings")
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
    else
      return Section::None;
  }
}

void JsonHandler::do_parse(JsonHandler::Section section, const nlohmann::json &object) {
  switch (section) {
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
    else if (type == "decoration-position")
      parseDecorationMoveEvent(object);
    else if (type == "decoration-orientation")
      parseDecorationOrientationEvent(object);
    else if (type == "xy-series-append")
      parseSeriesAppend(object);
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
    else if (type == "series-collection")
      parseSeriesCollection(object);
    else
      std::cerr << "Unhandled Series type: " << type << '\n';
  } break;
  default:
    std::cerr << "Non-section key passed to do_parse with object:" << object << '\n';
    break;
  }
}

void JsonHandler::parseConfiguration(const nlohmann::json &object) {
  fileParser.globalConfiguration.millisecondsPerFrame = object["ms-per-frame"].get<double>();
}

void JsonHandler::parseNode(const nlohmann::json &object) {
  parser::Node node;

  node.id = object["id"].get<uint32_t>();
  node.model = object["model"].get<std::string>();
  node.scale = object["scale"].get<float>();
  node.opacity = object["opacity"].get<double>();

  node.orientation[0] = object["orientation"]["x"].get<double>();
  node.orientation[1] = object["orientation"]["y"].get<double>();
  node.orientation[2] = object["orientation"]["z"].get<double>();

  node.visible = object["visible"].get<bool>();

  node.position.x = object["position"]["x"].get<float>();
  node.position.y = object["position"]["y"].get<float>();
  node.position.z = object["position"]["z"].get<float>();
  fileParser.nodes.emplace_back(node);
}

void JsonHandler::parseBuilding(const nlohmann::json &object) {
  parser::Building building;

  building.id = object["id"].get<uint32_t>();
  building.opacity = object["opacity"].get<double>();
  building.visible = object["visible"].get<bool>();
  building.floors = object["floors"].get<uint16_t>();

  building.roomsX = object["rooms"]["x"].get<uint16_t>();
  building.roomsY = object["rooms"]["y"].get<uint16_t>();

  building.min.x = object["bounds"]["x"]["min"].get<float>();
  building.min.y = object["bounds"]["y"]["min"].get<float>();
  building.min.z = object["bounds"]["z"]["min"].get<float>();

  building.max.x = object["bounds"]["x"]["max"].get<float>();
  building.max.y = object["bounds"]["y"]["max"].get<float>();
  building.max.z = object["bounds"]["z"]["max"].get<float>();

  fileParser.buildings.emplace_back(building);
}

void JsonHandler::parseDecoration(const nlohmann::json &object) {
  parser::Decoration decoration;

  decoration.id = object["id"].get<uint32_t>();
  decoration.model = object["model"].get<std::string>();

  decoration.position.x = object["position"]["x"].get<float>();
  decoration.position.y = object["position"]["y"].get<float>();
  decoration.position.z = object["position"]["z"].get<float>();

  decoration.orientation[0] = object["orientation"]["x"].get<double>();
  decoration.orientation[1] = object["orientation"]["y"].get<double>();
  decoration.orientation[2] = object["orientation"]["z"].get<double>();

  decoration.opacity = object["opacity"].get<double>();
  decoration.scale = object["scale"].get<float>();

  fileParser.decorations.emplace_back(decoration);
}

void JsonHandler::parseMoveEvent(const nlohmann::json &object) {
  parser::MoveEvent event;

  event.nodeId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition.x = object["x"].get<float>();
  event.targetPosition.y = object["y"].get<float>();
  event.targetPosition.z = object["z"].get<float>();

  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationMoveEvent(const nlohmann::json &object) {
  parser::DecorationMoveEvent event;

  event.decorationId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition.x = object["x"].get<float>();
  event.targetPosition.y = object["y"].get<float>();
  event.targetPosition.z = object["z"].get<float>();

  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseNodeOrientationEvent(const nlohmann::json &object) {
  parser::NodeOrientationChangeEvent event;

  event.nodeId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetOrientation[0] = object["x"].get<double>();
  event.targetOrientation[1] = object["y"].get<double>();
  event.targetOrientation[2] = object["z"].get<double>();

  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationOrientationEvent(const nlohmann::json &object) {
  parser::DecorationOrientationChangeEvent event;

  event.decorationId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetOrientation[0] = object["x"].get<double>();
  event.targetOrientation[1] = object["y"].get<double>();
  event.targetOrientation[2] = object["z"].get<double>();

  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseSeriesAppend(const nlohmann::json &object) {
  parser::XYSeriesAddValue event;

  event.time = object["milliseconds"].get<double>();
  event.seriesId = object["series-id"].get<uint32_t>();
  event.x = object["x"].get<double>();
  event.y = object["y"].get<double>();

  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseXYSeries(const nlohmann::json &object) {
  parser::XYSeries series;

  series.id = object["id"].get<uint32_t>();
  series.name = object["name"].get<std::string>();
  series.alpha = object["color"]["alpha"].get<uint8_t>();
  series.blue = object["color"]["blue"].get<uint8_t>();
  series.green = object["color"]["green"].get<uint8_t>();
  series.red = object["color"]["red"].get<uint8_t>();

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

  series.xAxis = valueAxisFromObject(object["x-axis"]);
  series.yAxis = valueAxisFromObject(object["y-axis"]);

  fileParser.xySeries.emplace_back(series);
}

void JsonHandler::parseSeriesCollection(const nlohmann::json &object) {
  parser::SeriesCollection collection;
  collection.id = object["id"].get<uint32_t>();
  collection.name = object["name"].get<std::string>();

  auto childSeries = object["child-series"];
  for (const auto &child : childSeries) {
    collection.series.emplace_back(child.get<uint32_t>());
  }

  collection.xAxis = valueAxisFromObject(object["x-axis"]);
  collection.yAxis = valueAxisFromObject(object["y-axis"]);
  fileParser.seriesCollections.emplace_back(collection);
}

JsonHandler::JsonHandler(parser::FileParser &parser) : fileParser(parser) {
}

bool JsonHandler::null() {
  handle(nullptr);
  return true;
}

bool JsonHandler::boolean(bool value) {
  handle(value);
  return true;
}

bool JsonHandler::number_integer(nlohmann::json::number_integer_t value) {
  handle(value);
  return true;
}

bool JsonHandler::number_unsigned(nlohmann::json::number_unsigned_t value) {
  handle(value);
  return true;
}

bool JsonHandler::number_float(nlohmann::json::number_float_t value, const nlohmann::json::string_t &raw) {
  handle(value);
  return true;
}

bool JsonHandler::string(nlohmann::json::string_t &value) {
  handle(value);
  return true;
}

bool JsonHandler::start_object(std::size_t elements) {
  if (currentKey)
    jsonStack.push({*currentKey, nlohmann::json::object()});

  sectionDepth++;
  return true;
}

bool JsonHandler::end_object() {
  // Possibly empty at the end of a message
  if (!jsonStack.empty()) {
    sectionDepth--;

    if (sectionDepth == 0) {
      auto frame = jsonStack.top();
      jsonStack.pop();
      do_parse(currentSection, frame.value);
    } else {
      auto top = jsonStack.top();
      jsonStack.pop();
      jsonStack.top().value[top.key] = top.value;
    }
  }
  return true;
}

bool JsonHandler::start_array(std::size_t elements) {
  if (!currentKey) {
    return true;
  }

  if (isSection(*currentKey) != Section::None)
    sectionDepth = 0;
  else {
    jsonStack.top().value[*currentKey] = nlohmann::json::array();
  }

  return true;
}

bool JsonHandler::end_array() {
  return true;
}

bool JsonHandler::key(nlohmann::json::string_t &value) {
  currentKey = value;

  auto possibleSection = isSection(value);
  if (possibleSection != Section::None) {
    currentSection = possibleSection;
  }
  return true;
}

bool JsonHandler::parse_error(std::size_t position, const std::string &last_token,
                              const nlohmann::detail::exception &ex) {
  throw ex;
}
