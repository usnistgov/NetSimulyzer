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
#include <iostream>
#include <json.hpp>

namespace parser {

void FileParser::parse(const char *path) {
  std::ifstream infile{path};

  if (!infile)
    std::cerr << "Failed to open " << path << '\n';

  JsonHandler handler{*this};
  nlohmann::json::sax_parse(infile, &handler);
  std::sort(nodes.begin(), nodes.end(), [](const Node &left, const Node &right) { return left.id < right.id; });

  std::sort(buildings.begin(), buildings.end(),
            [](const Building &left, const Building &right) { return left.id < right.id; });

  std::sort(decorations.begin(), decorations.end(),
            [](const Decoration &left, const Decoration &right) { return left.id < right.id; });
}

void FileParser::reset() {
  nodes.clear();
  buildings.clear();
  decorations.clear();
  sceneEvents.clear();
  chartEvents.clear();
  logEvents.clear();
  logStreams.clear();
  xySeries.clear();
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

const std::vector<Decoration> &FileParser::getDecorations() const {
  return decorations;
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

const std::vector<SeriesCollection> &FileParser::getSeriesCollections() const {
  return seriesCollections;
}

const std::vector<LogStream> &FileParser::getLogStreams() const {
  return logStreams;
}

} // namespace parser
