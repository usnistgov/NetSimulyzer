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

parser::ValueAxis valueAxisFromObject(const nlohmann::json &object) {
  parser::ValueAxis axis;
  axis.boundMode = boundModeFromString(object["bound-mode"].get<std::string>());
  axis.max = object["max"].get<double>();
  axis.min = object["min"].get<double>();
  axis.name = object["name"].get<std::string>();
  axis.scale = scaleFromString(object["scale"].get<std::string>());

  return axis;
}

parser::CategoryAxis categoryAxisFromObject(const nlohmann::json &object) {
  parser::CategoryAxis axis;
  axis.name = object["name"].get<std::string>();

  const auto &values = object["values"];
  for (const auto &value : values) {
    parser::CategoryAxis::Category category;

    category.id = value["id"].get<unsigned int>();
    category.name = value["value"].get<std::string>();

    axis.values.emplace_back(category);
  }

  // Category values must be sorted least to greatest
  // since we may only define a category's end position
  std::sort(axis.values.begin(), axis.values.end(),
            [](const auto &left, const auto &right) { return left.id < right.id; });

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

void JsonHandler::do_parse(JsonHandler::Section section, const nlohmann::json &object) {
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
    std::cerr << "Unidentified key passed to do_parse with object:" << object << '\n';
    break;
  }
}

void JsonHandler::parseConfiguration(const nlohmann::json &object) {
  fileParser.globalConfiguration.millisecondsPerFrame = object["ms-per-frame"].get<double>();
}

void JsonHandler::parseNode(const nlohmann::json &object) {
  parser::Node node;

  node.id = object["id"].get<uint32_t>();
  node.name = object["name"].get<std::string>();
  node.model = object["model"].get<std::string>();
  node.scale = object["scale"].get<float>();
  node.opacity = object["opacity"].get<double>();

  if (object.contains("height")) {
    node.height = object["height"].get<float>();
  }

  node.orientation[0] = object["orientation"]["x"].get<double>();
  node.orientation[1] = object["orientation"]["y"].get<double>();
  node.orientation[2] = object["orientation"]["z"].get<double>();

  node.visible = object["visible"].get<bool>();

  node.position.x = object["position"]["x"].get<float>();
  node.position.y = object["position"]["y"].get<float>();
  node.position.z = object["position"]["z"].get<float>();

  if (object.contains("offset")) {
    node.offset.x = object["offset"]["x"].get<float>();
    node.offset.y = object["offset"]["y"].get<float>();
    node.offset.z = object["offset"]["z"].get<float>();
  }

  updateLocationBounds(node.position);

  fileParser.nodes.emplace_back(node);
}

void JsonHandler::parseBuilding(const nlohmann::json &object) {
  parser::Building building;

  building.id = object["id"].get<uint32_t>();

  if (object.contains("color")) {
    building.color.red = object["color"]["red"].get<uint8_t>();
    building.color.green = object["color"]["green"].get<uint8_t>();
    building.color.blue = object["color"]["blue"].get<uint8_t>();
  } else {
    // TODO: Remove later (v0.2.0)
    std::cerr << "Warning: Building ID " << building.id
              << " does not define a color! "
                 "'color' is a new, required attribute. "
                 "This message will be replaced with a parse error "
                 "in a future version!\n";

    // Old default color
    building.color.red = 204u;
    building.color.green = 204u;
    building.color.blue = 204u;
  }

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

  updateLocationBounds(building.min);
  updateLocationBounds(building.max);

  fileParser.buildings.emplace_back(building);
}

void JsonHandler::parseDecoration(const nlohmann::json &object) {
  parser::Decoration decoration;

  decoration.id = object["id"].get<uint32_t>();
  decoration.model = object["model"].get<std::string>();

  decoration.position.x = object["position"]["x"].get<float>();
  decoration.position.y = object["position"]["y"].get<float>();
  decoration.position.z = object["position"]["z"].get<float>();

  if (object.contains("height")) {
    decoration.height = object["height"].get<float>();
  }

  updateLocationBounds(decoration.position);

  decoration.orientation[0] = object["orientation"]["x"].get<double>();
  decoration.orientation[1] = object["orientation"]["y"].get<double>();
  decoration.orientation[2] = object["orientation"]["z"].get<double>();

  decoration.opacity = object["opacity"].get<double>();
  decoration.scale = object["scale"].get<float>();

  fileParser.decorations.emplace_back(decoration);
}

void JsonHandler::parseArea(const nlohmann::json &object) {
  parser::Area area;

  area.id = object["id"].get<unsigned int>();
  area.name = object["name"].get<std::string>();
  area.height = object["height"].get<float>();

  for (const auto &point : object["points"]) {
    parser::Ns3Coordinate coordinate;

    coordinate.x = point["x"].get<float>();
    coordinate.y = point["y"].get<float>();

    // An area is 2D, so each point shares the same
    // height, included with the coordinates for
    // convenience
    coordinate.z = area.height;

    area.points.emplace_back(coordinate);
  }

  area.fillMode = drawModeFromString(object["fill-mode"].get<std::string>());

  area.fillColor.red = object["fill-color"]["red"].get<uint8_t>();
  area.fillColor.green = object["fill-color"]["green"].get<uint8_t>();
  area.fillColor.blue = object["fill-color"]["blue"].get<uint8_t>();

  area.borderMode = drawModeFromString(object["border-mode"].get<std::string>());

  area.borderColor.red = object["border-color"]["red"].get<uint8_t>();
  area.borderColor.green = object["border-color"]["green"].get<uint8_t>();
  area.borderColor.blue = object["border-color"]["blue"].get<uint8_t>();

  fileParser.areas.emplace_back(area);
}

void JsonHandler::parseMoveEvent(const nlohmann::json &object) {
  parser::MoveEvent event;

  event.nodeId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition.x = object["x"].get<float>();
  event.targetPosition.y = object["y"].get<float>();
  event.targetPosition.z = object["z"].get<float>();

  updateLocationBounds(event.targetPosition);

  fileParser.sceneEvents.emplace_back(event);
}

void JsonHandler::parseDecorationMoveEvent(const nlohmann::json &object) {
  parser::DecorationMoveEvent event;

  event.decorationId = object["id"].get<uint32_t>();
  event.time = object["milliseconds"].get<double>();
  event.targetPosition.x = object["x"].get<float>();
  event.targetPosition.y = object["y"].get<float>();
  event.targetPosition.z = object["z"].get<float>();

  updateLocationBounds(event.targetPosition);

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

void JsonHandler::parseCategorySeriesAppend(const nlohmann::json &object) {
  parser::CategorySeriesAddValue event;

  event.time = object["milliseconds"].get<double>();
  event.seriesId = object["series-id"].get<uint32_t>();
  event.category = object["category"].get<unsigned int>();
  event.value = object["value"].get<double>();

  fileParser.chartEvents.emplace_back(event);
}

void JsonHandler::parseXYSeries(const nlohmann::json &object) {
  parser::XYSeries series;

  series.id = object["id"].get<uint32_t>();
  series.name = object["name"].get<std::string>();

  // TODO: Remove later (v0.2.0)
  if (!object.contains("legend")) {
    std::cerr << "XY Series ID: " << series.id
              << " Missing required attribute 'legend'.\n"
                 "This message will be replaced with a "
                 "parse error in the future\n";
    series.legend = series.name;
  } else {
    series.legend = object["legend"].get<std::string>();
  }

  // TODO: Remove later (v0.2.0)
  if (!object.contains("visible")) {
    std::cerr << "XY Series ID: " << series.id
              << " Missing required attribute 'visible'.\n"
                 "This message will be replaced with a "
                 "parse error in the future\n";
    series.visible = true;
  } else {
    series.visible = object["visible"].get<bool>();
  }

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

void JsonHandler::parseCategoryValueSeries(const nlohmann::json &object) {
  parser::CategoryValueSeries series;

  series.id = object["id"].get<uint32_t>();
  series.name = object["name"].get<std::string>();

  // TODO: Remove later (v0.2.0)
  if (!object.contains("legend")) {
    std::cerr << "Category Value Series ID: " << series.id
              << " Missing required attribute 'legend'.\n"
                 "This message will be replaced with a "
                 "parse error in the future\n";
    series.legend = series.name;
  } else {
    series.legend = object["legend"].get<std::string>();
  }

  // TODO: Remove later (v0.2.0)
  if (!object.contains("visible")) {
    std::cerr << "Category Value Series ID: " << series.id
              << " Missing required attribute 'visible'.\n"
                 "This message will be replaced with a "
                 "parse error in the future\n";
    series.visible = true;
  } else {
    series.visible = object["visible"].get<bool>();
  }

  series.alpha = object["color"]["alpha"].get<uint8_t>();
  series.blue = object["color"]["blue"].get<uint8_t>();
  series.green = object["color"]["green"].get<uint8_t>();
  series.red = object["color"]["red"].get<uint8_t>();

  auto connectionMode = object["connection-mode"].get<std::string>();
  if (connectionMode == "all")
    series.connectionMode = parser::CategoryValueSeries::ConnectionMode::All;
  else if (connectionMode == "in category")
    series.connectionMode = parser::CategoryValueSeries::ConnectionMode::InCategory;
  else
    std::cerr << "Unrecognized connection mode: " << connectionMode << '\n';

  series.xAxis = valueAxisFromObject(object["x-axis"]);
  series.yAxis = categoryAxisFromObject(object["y-axis"]);

  fileParser.categoryValueSeries.emplace_back(series);
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

void JsonHandler::parseLogStream(const nlohmann::json &object) {
  parser::LogStream stream;
  stream.id = object["id"].get<unsigned int>();
  stream.name = object["name"].get<std::string>();

  if (object.contains("color")) {
    parser::Ns3Color3 color{};
    color.blue = object["color"]["blue"].get<uint8_t>();
    color.green = object["color"]["green"].get<uint8_t>();
    color.red = object["color"]["red"].get<uint8_t>();

    stream.color = color;
  }

  // TODO: Remove later (v0.2.0)
  if (!object.contains("visible")) {
    std::cerr << "Log Stream ID: " << stream.id << " "
              << "Missing required attribute 'visible'.\n"
                 "This message will be replaced with a "
                 "parse error in the future\n";
    stream.visible = true;
  } else {
    stream.visible = object["visible"].get<bool>();
  }

  fileParser.logStreams.emplace_back(stream);
}

void JsonHandler::parseStreamAppend(const nlohmann::json &object) {
  parser::StreamAppendEvent event;
  event.time = object["milliseconds"].get<double>();
  event.streamId = object["stream-id"].get<unsigned int>();
  event.value = object["data"].get<std::string>();

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
  // Root object case
  if (jsonStack.empty()) {
    jsonStack.push({"root", nlohmann::json::object()});
    return true;
  }

  auto &top = jsonStack.top();

  // Do not overwrite existing values
  if (top.value.is_array()) {
    jsonStack.push({"", nlohmann::json::object()});
    return true;
  }

  top.value = nlohmann::json::object();
  sectionDepth++;
  return true;
}

bool JsonHandler::end_object() {
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
  if (currentSection == Section::Configuration && oldTop.key == "configuration") {
    parseConfiguration(oldTop.value);
    return true;
  }

  // All other sections have one parse call per item
  if (isSection(jsonStack.top().key) != Section::None) {
    do_parse(currentSection, oldTop.value);
    return true;
  }

  if (currentTop.value.is_array()) {
    currentTop.value.emplace_back(oldTop.value);
    return true;
  }

  if (currentTop.value.is_object()) {
    currentTop.value[oldTop.key] = oldTop.value;
    return true;
  }

  return false;
}

bool JsonHandler::start_array(std::size_t elements) {
  if (jsonStack.empty()) {
    return false;
  }

  jsonStack.top().value = nlohmann::json::array();
  return true;
}

bool JsonHandler::end_array() {
  auto oldTop = jsonStack.top();
  jsonStack.pop();

  auto &currentTop = jsonStack.top();
  if (currentTop.value.is_object())
    currentTop.value[oldTop.key] = oldTop.value;
  else if (currentTop.value.is_array())
    currentTop.value.emplace_back(oldTop.value);
  else {
    std::cerr << currentTop.value << '\n';
    std::abort();
  }

  return true;
}

bool JsonHandler::key(nlohmann::json::string_t &value) {
  jsonStack.push({value});

  auto possibleSection = isSection(value);
  if (possibleSection != Section::None) {
    currentSection = possibleSection;
    sectionDepth = 0;
  }
  return true;
}
bool JsonHandler::parse_error(std::size_t position, const std::string &last_token,
                              const nlohmann::detail::exception &ex) {
  throw ex;
}
