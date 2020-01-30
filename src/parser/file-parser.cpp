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

namespace visualization {

FileParser::FileParser() {
  saxHandler.startElement = FileParser::startElementCallback;
  saxHandler.endElement = FileParser::endElementCallback;
  saxHandler.characters = FileParser::charactersCallback;
}

void FileParser::parse(const char *path) {
  LIBXML_TEST_VERSION
  xmlSAXUserParseFile(&saxHandler, this, path);
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

void FileParser::interpretCharacters(const std::string &data) {
  switch (currentTag) {
  case Tag::None:
    // Intentionally blank
    break;
  case Tag::MsPerFrame:
    globalConfiguration.millisecondsPerFrame = std::stod(data);
    break;
  case Tag::SeriesCollection:
    // Intentionally blank
    // Parsing occurs in 'child-series' tag
    break;
  }
}

void FileParser::parseNode(const xmlChar *attributes[]) {
  std::string attribute;
  Node node{};

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    // Even indexes mark keys, odd marks values
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        node.id = std::stoul(value);
      else if (attribute == "model")
        node.model = value;
      else if (attribute == "scale")
        node.scale = std::stod(value);
      else if (attribute == "opacity")
        node.opacity = std::stod(value);
      else if (attribute == "visible")
        node.visible = std::strcmp(value, "1") == 0;
      else if (attribute == "x")
        node.position[0] = std::stod(value);
      else if (attribute == "y")
        node.position[1] = std::stod(value);
      else if (attribute == "z")
        node.position[2] = std::stod(value);
      else if (attribute == "x-orientation")
        node.orientation[0] = std::stod(value) * TO_RADIANS;
      else if (attribute == "y-orientation")
        node.orientation[1] = std::stod(value) * TO_RADIANS;
      else if (attribute == "z-orientation")
        node.orientation[2] = std::stod(value) * TO_RADIANS;
    }
  }

  nodes.emplace_back(node);
}

void FileParser::parseBuilding(const xmlChar *attributes[]) {
  std::string attribute;
  Building building{};

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        building.id = std::stoul(value);
      else if (attribute == "opacity")
        building.opacity = std::stod(value);
      else if (attribute == "visible")
        building.visible = std::strcmp(value, "1") == 0;
      else if (attribute == "rooms-x")
        building.roomsX = std::stoi(value);
      else if (attribute == "rooms-y")
        building.roomsY = std::stoi(value);
      else if (attribute == "x-min")
        building.xMin = std::stod(value);
      else if (attribute == "x-max")
        building.xMax = std::stod(value);
      else if (attribute == "y-min")
        building.yMin = std::stod(value);
      else if (attribute == "y-max")
        building.yMax = std::stod(value);
      else if (attribute == "z-min")
        building.zMin = std::stod(value);
      else if (attribute == "z-max")
        building.zMax = std::stod(value);
    }
  }

  buildings.emplace_back(building);
}

void FileParser::parseDecoration(const xmlChar *attributes[]) {
  std::string attribute;
  Decoration decoration{};

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "model")
        decoration.model = value;
      else if (attribute == "x")
        decoration.position[0] = std::stod(value);
      else if (attribute == "y")
        decoration.position[1] = std::stod(value);
      else if (attribute == "z")
        decoration.position[2] = std::stod(value);
      else if (attribute == "opacity")
        decoration.opacity = std::stod(value);
      else if (attribute == "scale")
        decoration.scale = std::stod(value);
    }
  }

  decorations.emplace_back(decoration);
}

void FileParser::parseMoveEvent(const xmlChar *attributes[]) {
  std::string attribute;
  MoveEvent event{};

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        event.nodeId = std::stol(value);
      else if (attribute == "milliseconds")
        event.time = std::stod(value);
      else if (attribute == "x")
        event.targetPosition[0] = std::stod(value);
      else if (attribute == "y")
        event.targetPosition[1] = std::stod(value);
      else if (attribute == "z")
        event.targetPosition[2] = std::stod(value);
    }
  }

  events.emplace_back(event);
}

void FileParser::parseNodeOrientationEvent(const xmlChar *attributes[]) {
  std::string attribute;
  NodeOrientationChangeEvent event{};

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        event.nodeId = std::stol(value);
      else if (attribute == "milliseconds")
        event.time = std::stod(value);
      else if (attribute == "x")
        event.targetOrientation[0] = std::stod(value) * TO_RADIANS;
      else if (attribute == "y")
        event.targetOrientation[1] = std::stod(value) * TO_RADIANS;
      else if (attribute == "z")
        event.targetOrientation[2] = std::stod(value) * TO_RADIANS;
    }
  }

  events.emplace_back(event);
}

void FileParser::parseSeriesAppend(const xmlChar *attributes[]) {
  std::string attribute;
  XYSeriesAddValue event{};

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "series-id")
        event.seriesId = std::stol(value);
      else if (attribute == "milliseconds")
        event.time = std::stod(value);
      else if (attribute == "x")
        event.x = std::stod(value);
      else if (attribute == "y")
        event.y = std::stod(value);
    }
  }

  events.emplace_back(event);
}

void FileParser::parseXYSeries(const xmlChar *attributes[]) {
  std::string attribute;
  XYSeries series;

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        series.id = std::stol(value);
      else if (attribute == "connection") {
        if (strcmp(value, "none") == 0)
          series.connection = XYSeries::Connection::None;
        else if (strcmp(value, "line") == 0)
          series.connection = XYSeries::Connection::Line;
        else if (strcmp(value, "spline") == 0)
          series.connection = XYSeries::Connection::Spline;
      } else if (attribute == "name")
        series.name = value;
      else if (attribute == "red")
        series.red = std::stol(value);
      else if (attribute == "green")
        series.green = std::stol(value);
      else if (attribute == "blue")
        series.blue = std::stol(value);
      else if (attribute == "alpha")
        series.alpha = std::stol(value);
      else if (attribute == "x-axis")
        series.xAxis.name = value;
      else if (attribute == "x-axis-min")
        series.xAxis.min = std::stod(value);
      else if (attribute == "x-axis-max")
        series.xAxis.max = std::stod(value);
      else if (attribute == "x-axis-scale")
        series.xAxis.scale = scaleFromString(value);
      else if (attribute == "x-axis-bound-mode")
        series.xAxis.boundMode = boundModeFromString(value);
      else if (attribute == "y-axis")
        series.yAxis.name = value;
      else if (attribute == "y-axis-min")
        series.yAxis.min = std::stod(value);
      else if (attribute == "y-axis-max")
        series.yAxis.max = std::stod(value);
      else if (attribute == "y-axis-scale")
        series.yAxis.scale = scaleFromString(value);
      else if (attribute == "y-axis-bound-mode")
        series.yAxis.boundMode = boundModeFromString(value);
    }
  }

  xySeries.emplace_back(series);
}

void FileParser::parseSeriesCollection(const xmlChar *attributes[]) {
  std::string attribute;
  SeriesCollection collection;

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        collection.id = std::stol(value);
      else if (attribute == "name")
        collection.name = value;
      else if (attribute == "x-axis")
        collection.xAxis.name = value;
      else if (attribute == "x-axis-min")
        collection.xAxis.min = std::stod(value);
      else if (attribute == "x-axis-max")
        collection.xAxis.max = std::stod(value);
      else if (attribute == "x-axis-scale")
        collection.xAxis.scale = scaleFromString(value);
      else if (attribute == "x-axis-bound-mode")
        collection.xAxis.boundMode = boundModeFromString(value);
      else if (attribute == "y-axis")
        collection.yAxis.name = value;
      else if (attribute == "y-axis-min")
        collection.yAxis.min = std::stod(value);
      else if (attribute == "y-axis-max")
        collection.yAxis.max = std::stod(value);
      else if (attribute == "y-axis-scale")
        collection.yAxis.scale = scaleFromString(value);
      else if (attribute == "y-axis-bound-mode")
        collection.yAxis.boundMode = boundModeFromString(value);
    }
  }

  seriesCollections.emplace_back(collection);
}

void FileParser::parseChildSeries(const xmlChar *attributes[]) {
  std::string attribute;
  auto &lastCollection = seriesCollections.back();

  for (auto i = 0; attributes[i] != nullptr; i++) {
    auto value = reinterpret_cast<const char *>(attributes[i]);
    if (i % 2 == 0) {
      attribute.erase();
      attribute.insert(0, value);
    } else {
      if (attribute == "id")
        lastCollection.series.emplace_back(std::stol(value));
    }
  }
}
void FileParser::startElementCallback(void *user_data, const xmlChar *name, const xmlChar **attrs) {
  using Section = FileParser::Section;
  auto parser = static_cast<visualization::FileParser *>(user_data);
  std::string tagName(reinterpret_cast<const char *>(name));
  auto section = parser->currentSection;

  if (section == FileParser::Section::None) {
    if (tagName == "configuration")
      parser->currentSection = FileParser::Section::Configuration;
    else if (tagName == "nodes")
      parser->currentSection = FileParser::Section::Nodes;
    else if (tagName == "buildings")
      parser->currentSection = FileParser::Section::Buildings;
    else if (tagName == "decorations")
      parser->currentSection = FileParser::Section::Decorations;
    else if (tagName == "events")
      parser->currentSection = FileParser::Section::Events;
    else if (tagName == "series")
      parser->currentSection = FileParser::Section::Series;
    return;
  }

  if (section == FileParser::Section::Configuration) {
    if (tagName == "ms-per-frame")
      parser->currentTag = FileParser::Tag::MsPerFrame;
    return;
  } else if (section == FileParser::Section::Series) {
    if (tagName == "series-collection") {
      parser->currentTag = FileParser::Tag::SeriesCollection;
      parser->parseSeriesCollection(attrs);
    }
  }

  if (section == FileParser::Section::Nodes && tagName == "node")
    parser->parseNode(attrs);
  else if (section == FileParser::Section::Buildings && tagName == "building")
    parser->parseBuilding(attrs);
  else if (section == FileParser::Section::Decorations && tagName == "decoration")
    parser->parseDecoration(attrs);
  else if (section == Section::Series) {
    if (tagName == "xy-series")
      parser->parseXYSeries(attrs);
    if (tagName == "child-series" && parser->currentTag == FileParser::Tag::SeriesCollection)
      parser->parseChildSeries(attrs);
  } else if (section == Section::Events) {
    if (tagName == "position")
      parser->parseMoveEvent(attrs);
    else if (tagName == "xy-series-append")
      parser->parseSeriesAppend(attrs);
    else if (tagName == "node-orientation")
      parser->parseNodeOrientationEvent(attrs);
  }
}
void FileParser::charactersCallback(void *user_data, const xmlChar *characters, int length) {
  auto parser = static_cast<visualization::FileParser *>(user_data);

  // Skip this callback if we're in a tag that has no meaningful
  // character content
  if (parser->currentTag == FileParser::Tag::None)
    return;

  // Be careful with the length here.
  // The implementation will likely hand us much more then the content of the tag
  std::string data(reinterpret_cast<const char *>(characters), length);
  parser->interpretCharacters(data);
}
void FileParser::endElementCallback(void *user_data, const xmlChar *name) {
  auto parser = static_cast<visualization::FileParser *>(user_data);
  std::string tagName(reinterpret_cast<const char *>(name));

  if (tagName == "configuration" || tagName == "nodes" || tagName == "buildings" || tagName == "decorations" ||
      tagName == "events" || tagName == "series") {
    parser->currentSection = FileParser::Section::None;
  }

  if (tagName == "ms-per-frame" || tagName == "series-collection")
    parser->currentTag = FileParser::Tag::None;
}

} // namespace visualization
