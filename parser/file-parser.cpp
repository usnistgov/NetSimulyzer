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
#include "handler/JsonHandler.h"
#include <algorithm>
#include <cstdio>
#include <iostream>
#include <memory>
#include <rapidjson/filereadstream.h>
#include <rapidjson/reader.h>

namespace parser {

std::optional<ParseError> FileParser::parse(const char *path) {

  // RapidJSON prefers FILE*, so this is a safe wrapper for that
  // Add a 'b' in the mode flags to keep Windows from stupid handling of newlines
  std::unique_ptr<FILE, decltype(&std::fclose)> file{std::fopen(path, "rb"), std::fclose};

  if (!file) {
    std::cerr << "Failed to open file: " << path << '\n';
    return {ParseError{"Failed to open file", 0u}};
  }

  // Mostly arbitrary buffer size
  char buffer[65536];
  rapidjson::FileReadStream stream{file.get(), buffer, sizeof(buffer)};

  JsonHandler handler{*this};
  rapidjson::Reader reader;

  reader.Parse(stream, handler);

  if (reader.HasParseError()) {
    ParseError error;
    error.offset = reader.GetErrorOffset();

    switch (reader.GetParseErrorCode()) {
      // Error from the `JsonHandler`
    case rapidjson::kParseErrorTermination:
      error.message = errorMessage.value_or("Unknown parsing error");
      break;
      // Generic Errors
    case rapidjson::kParseErrorDocumentEmpty:
      error.message = "Document empty";
      break;
    case rapidjson::kParseErrorDocumentRootNotSingular:
      error.message = "More than one root element";
      break;
    case rapidjson::kParseErrorValueInvalid:
      error.message = "Invalid value";
      break;
    case rapidjson::kParseErrorObjectMissName:
      error.message = "Object member missing name";
      break;
    case rapidjson::kParseErrorObjectMissColon:
      error.message = "Object property missing colon";
      break;
    case rapidjson::kParseErrorObjectMissCommaOrCurlyBracket:
      error.message = "Missing comma or curly brace after object member";
      break;
    case rapidjson::kParseErrorArrayMissCommaOrSquareBracket:
      error.message = "Missing comma or curly brace after array element";
      break;
    case rapidjson::kParseErrorStringUnicodeEscapeInvalidHex:
      error.message = "Invalid Unicode escape sequence";
      break;
    case rapidjson::kParseErrorStringUnicodeSurrogateInvalid:
      error.message = "Invalid Unicode surrogate pair";
      break;
    case rapidjson::kParseErrorStringEscapeInvalid:
      error.message = "Invalid character escape sequence";
      break;
    case rapidjson::kParseErrorStringMissQuotationMark:
      error.message = "Missing string quotation mark";
      break;
    case rapidjson::kParseErrorStringInvalidEncoding:
      error.message = "Invalid string encoding";
      break;
    case rapidjson::kParseErrorNumberTooBig:
      error.message = "Number too large to be stored in a double";
      break;
    case rapidjson::kParseErrorNumberMissFraction:
      error.message = "Number missing fraction component";
      break;
    case rapidjson::kParseErrorNumberMissExponent:
      error.message = "Number missing exponent component";
      break;
    case rapidjson::kParseErrorUnspecificSyntaxError:
    default:
      error.message = "Unspecific syntax error";
      break;
    }

    return {error};
  }

  std::sort(nodes.begin(), nodes.end(), [](const Node &left, const Node &right) {
    return left.id < right.id;
  });

  std::sort(buildings.begin(), buildings.end(), [](const Building &left, const Building &right) {
    return left.id < right.id;
  });

  std::sort(decorations.begin(), decorations.end(), [](const Decoration &left, const Decoration &right) {
    return left.id < right.id;
  });

  return {};
}

void FileParser::reset() {
  globalConfiguration = {};
  nodes.clear();
  buildings.clear();
  decorations.clear();
  areas.clear();
  sceneEvents.clear();
  chartEvents.clear();
  logEvents.clear();
  logStreams.clear();
  xySeries.clear();
  categoryValueSeries.clear();
  seriesCollections.clear();
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

const std::vector<Area> &FileParser::getAreas() const {
  return areas;
}

const std::vector<Decoration> &FileParser::getDecorations() const {
  return decorations;
}

const std::vector<WiredLink> &FileParser::getLinks() const {
  return wiredLinks;
}

const std::vector<SceneEvent> &FileParser::getSceneEvents() const {
  return sceneEvents;
}

const std::vector<ChartEvent> &FileParser::getChartsEvents() const {
  return chartEvents;
}

const std::vector<LogEvent> &FileParser::getLogEvents() const {
  return logEvents;
}

const std::vector<XYSeries> &FileParser::getXYSeries() const {
  return xySeries;
}

const std::vector<CategoryValueSeries> &FileParser::getCategoryValueSeries() const {
  return categoryValueSeries;
}

const std::vector<SeriesCollection> &FileParser::getSeriesCollections() const {
  return seriesCollections;
}

const std::vector<LogStream> &FileParser::getLogStreams() const {
  return logStreams;
}

} // namespace parser
