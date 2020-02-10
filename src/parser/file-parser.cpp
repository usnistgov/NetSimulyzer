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
#include "file-parser.h"
#include <cassert>
#include <cstring>
#include <fstream>
#include <iostream>
#include <json.hpp>
#include <memory>

visualization::ValueAxis::BoundMode boundModeFromString(const std::string &mode) {
  if (mode == "fixed")
    return visualization::ValueAxis::BoundMode::Fixed;
  else if (mode == "highest value")
    return visualization::ValueAxis::BoundMode::HighestValue;
  else
    assert(!"Unhandled ValueAxis::BoundMode mode");
}

visualization::ValueAxis::Scale scaleFromString(const std::string &mode) {
  if (mode == "linear")
    return visualization::ValueAxis::Scale::Linear;
  else if (mode == "logarithmic")
    return visualization::ValueAxis::Scale::Logarithmic;
  else
    assert(!"Unhandled ValueAxis::Scale mode");
}

visualization::ValueAxis valueAxisFromObject(const nlohmann::json &object) {
  visualization::ValueAxis axis;
  axis.boundMode = boundModeFromString(object["bound-mode"].get<std::string>());
  axis.max = object["max"].get<double>();
  axis.min = object["min"].get<double>();
  axis.name = object["name"].get<std::string>();
  axis.scale = scaleFromString(object["scale"].get<std::string>());

  return axis;
}

namespace visualization {

void FileParser::parse(const char *path) {
  std::ifstream infile{path};

  if (!infile)
    std::cerr << "Failed to open " << path << '\n';

  nlohmann::json::sax_parse(infile, this);
}

const GlobalConfiguration &FileParser::getConfiguration() const {
  return globalConfiguration;
}

const std::vector<Node> &FileParser::getNodes() const {
  return nodes;
}

const std::vector<Building> &FileParser::getBuildings() const {
  return buildings;
}

const std::vector<Decoration> &FileParser::getDecorations() const {
  return decorations;
}

const std::vector<Event> &FileParser::getEvents() const {
  return events;
}

const std::vector<XYSeries> &FileParser::getXYSeries() const {
  return xySeries;
}

const std::vector<SeriesCollection> &FileParser::getSeriesCollections() const {
  return seriesCollections;
}

bool FileParser::null() {
  handle(nullptr);
  return true;
}

bool FileParser::boolean(bool value) {
  handle(value);
  return true;
}
bool FileParser::number_integer(nlohmann::json::number_integer_t value) {
  handle(value);
  return true;
}
bool FileParser::number_unsigned(nlohmann::json::number_unsigned_t value) {
  handle(value);
  return true;
}
bool FileParser::number_float(nlohmann::json::number_float_t value, const nlohmann::json::string_t &) {
  handle(value);
  return true;
}
bool FileParser::string(nlohmann::json::string_t &value) {
  handle(value);
  return true;
}
bool FileParser::start_object(std::size_t) {
  if (currentKey)
    jsonStack.push({*currentKey, nlohmann::json::object()});

  sectionDepth++;
  return true;
}
bool FileParser::end_object() {
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
bool FileParser::start_array(std::size_t) {
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
bool FileParser::end_array() {
  return true;
}
bool FileParser::key(nlohmann::json::string_t &value) {
  currentKey = value;

  auto possibleSection = isSection(value);
  if (possibleSection != Section::None) {
    currentSection = possibleSection;
  }
  return true;
}
bool FileParser::parse_error(std::size_t, const std::string &, const nlohmann::detail::exception &ex) {
  throw ex;
}

constexpr FileParser::Section FileParser::isSection(std::string_view key) {
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

void FileParser::do_parse(FileParser::Section section, const nlohmann::json &object) {
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

void FileParser::parseConfiguration(const nlohmann::json &object) {
  globalConfiguration.millisecondsPerFrame = object["ms-per-frame"].get<double>();
}

void FileParser::parseNode(const nlohmann::json &object) {
  Node node;

  node.id = object["id"].get<uint32_t>();
  node.model = object["model"].get<std::string>();
  node.opacity = object["scale"].get<double>();
  node.opacity = object["opacity"].get<double>();

  node.orientation[0] = object["orientation"]["x"].get<double>() * TO_RADIANS;
  node.orientation[1] = object["orientation"]["y"].get<double>() * TO_RADIANS;
  node.orientation[2] = object["orientation"]["z"].get<double>() * TO_RADIANS;

  node.visible = object["visible"].get<bool>();

  node.position[0] = object["position"]["x"].get<double>();
  node.position[1] = object["position"]["y"].get<double>();
  node.position[2] = object["position"]["z"].get<double>();
  nodes.emplace_back(node);
}

void FileParser::parseBuilding(const nlohmann::json &object) {
  Building building;

  building.id = object["id"].get<uint32_t>();
  building.opacity = object["opacity"].get<double>();
  building.visible = object["visible"].get<bool>();
  building.floors = object["floors"].get<uint16_t>();

  building.roomsX = object["rooms"]["x"].get<uint16_t>();
  building.roomsY = object["rooms"]["y"].get<uint16_t>();

  building.xMin = object["bounds"]["x"]["min"].get<double>();
  building.xMax = object["bounds"]["x"]["max"].get<double>();

  building.yMin = object["bounds"]["y"]["min"].get<double>();
  building.yMax = object["bounds"]["y"]["max"].get<double>();

  building.zMin = object["bounds"]["z"]["min"].get<double>();
  building.zMax = object["bounds"]["z"]["max"].get<double>();

  buildings.emplace_back(building);
}

void FileParser::parseDecoration(const nlohmann::json &object) {
  Decoration decoration;

  decoration.id = object["id"].get<uint32_t>();
  decoration.model = object["model"].get<std::string>();

  decoration.position[0] = object["position"]["x"].get<double>();
  decoration.position[1] = object["position"]["y"].get<double>();
  decoration.position[2] = object["position"]["z"].get<double>();

  decoration.orientation[0] = object["orientation"]["x"].get<double>() * TO_RADIANS;
  decoration.orientation[1] = object["orientation"]["y"].get<double>() * TO_RADIANS;
  decoration.orientation[2] = object["orientation"]["z"].get<double>() * TO_RADIANS;

  decoration.opacity = object["opacity"].get<double>();
  decoration.opacity = object["scale"].get<double>();

  decorations.emplace_back(decoration);
}

void FileParser::parseMoveEvent(const nlohmann::json &object) {
  MoveEvent event;

  event.nodeId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition[0] = object["x"].get<double>();
  event.targetPosition[1] = object["y"].get<double>();
  event.targetPosition[2] = object["z"].get<double>();

  events.emplace_back(event);
}

void FileParser::parseDecorationMoveEvent(const nlohmann::json &object) {
  DecorationMoveEvent event;

  event.decorationId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition[0] = object["x"].get<double>();
  event.targetPosition[1] = object["y"].get<double>();
  event.targetPosition[2] = object["z"].get<double>();

  events.emplace_back(event);
}

void FileParser::parseNodeOrientationEvent(const nlohmann::json &object) {
  NodeOrientationChangeEvent event;

  event.nodeId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetOrientation[0] = object["x"].get<double>() * TO_RADIANS;
  event.targetOrientation[1] = object["y"].get<double>() * TO_RADIANS;
  event.targetOrientation[2] = object["z"].get<double>() * TO_RADIANS;

  events.emplace_back(event);
}

void FileParser::parseDecorationOrientationEvent(const nlohmann::json &object) {
  DecorationOrientationChangeEvent event;

  event.decorationId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetOrientation[0] = object["x"].get<double>() * TO_RADIANS;
  event.targetOrientation[1] = object["y"].get<double>() * TO_RADIANS;
  event.targetOrientation[2] = object["z"].get<double>() * TO_RADIANS;

  events.emplace_back(event);
}

void FileParser::parseSeriesAppend(const nlohmann::json &object) {
  XYSeriesAddValue event;

  event.time = object["milliseconds"].get<double>();
  event.seriesId = object["series-id"].get<uint32_t>();
  event.x = object["x"].get<double>();
  event.y = object["y"].get<double>();

  events.emplace_back(event);
}

void FileParser::parseXYSeries(const nlohmann::json &object) {
  XYSeries series;

  series.id = object["id"].get<uint32_t>();
  series.name = object["name"].get<std::string>();
  series.alpha = object["color"]["alpha"].get<uint8_t>();
  series.blue = object["color"]["blue"].get<uint8_t>();
  series.green = object["color"]["green"].get<uint8_t>();
  series.red = object["color"]["red"].get<uint8_t>();

  auto connection = object["connection"].get<std::string>();
  if (connection == "none")
    series.connection = XYSeries::Connection::None;
  else if (connection == "line")
    series.connection = XYSeries::Connection::Line;
  else if (connection == "spline")
    series.connection = XYSeries::Connection::Spline;
  else
    std::cerr << "Unrecognized connection type: " << connection << '\n';

  series.xAxis = valueAxisFromObject(object["x-axis"]);
  series.yAxis = valueAxisFromObject(object["y-axis"]);

  xySeries.emplace_back(series);
}

void FileParser::parseSeriesCollection(const nlohmann::json &object) {
  SeriesCollection collection;
  collection.name = object["name"].get<std::string>();

  auto childSeries = object["child-series"];
  for (const auto &child : childSeries) {
    collection.series.emplace_back(child.get<uint32_t>());
  }

  collection.xAxis = valueAxisFromObject(object["x-axis"]);
  collection.yAxis = valueAxisFromObject(object["y-axis"]);
  seriesCollections.emplace_back(collection);
}

} // namespace visualization
