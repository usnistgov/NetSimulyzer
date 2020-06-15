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
#include "model.h"
#include <optional>
#include <stack>
#include <string>
#include <vector>

// Avoid including this from it's header, since that depends on json.hpp
class JsonHandler;

namespace parser {

class FileParser {
  friend JsonHandler;

public:
  /**
   * Read the JSON file specified by path,
   * sets the configuration, nodes, etc.
   *
   * @param path
   * The path to the JSON file
   */
  void parse(const char *path);

  /**
   * Clear stored information from a previous `parse()` call
   */
  void reset();

  /**
   * Gets the configuration from the parsed file
   * `parse()` should be called first
   *
   * @return The nodes specified by the parsed file
   */
  [[nodiscard]] const GlobalConfiguration &getConfiguration() const;

  /**
   * Gets the collection of nodes from the parsed file
   * `parse()` should be called first
   *
   * @return The nodes specified by the parsed file
   */
  [[nodiscard]] const std::vector<Node> &getNodes() const;

  /**
   * Gets the collection of buildings from the parsed file
   * `parse()` should be called first
   *
   * @return The buildings specified by the parsed file
   */
  [[nodiscard]] const std::vector<Building> &getBuildings() const;

  /**
   * Gets the collection of areas from the parsed file
   * `parse()` should be called first
   *
   * @return The areas specified by the parsed file
   */
  [[nodiscard]] const std::vector<Area> &getAreas() const;

  /**
   *  Gets the collection of decorations from the parsed file
   * `parse()` should be called first
   *
   * @return The decorations specified by the parsed file
   */
  [[nodiscard]] const std::vector<Decoration> &getDecorations() const;

  /**
   * Gets the collection of events for the Scene from the parsed file
   * `parse()` should be called first.
   *
   * This returns a partial collection of the events from the parsed file
   *
   * @return The events specified by the parsed file
   */
  [[nodiscard]] const std::vector<SceneEvent> &getSceneEvents() const;

  /**
   * Gets the collection of events for the Charts controller from the parsed file
   * `parse()` should be called first.
   *
   * This returns a partial collection of the events from the parsed file
   *
   * @return The events specified by the parsed file
   */
  [[nodiscard]] const std::vector<ChartEvent> &getChartsEvents() const;

  /**
   * Gets the collection of events for the Scenario Log controller from the parsed file
   * `parse()` should be called first.
   *
   * This returns a partial collection of the events from the parsed file
   *
   * @return The events specified by the parsed file
   */
  [[nodiscard]] const std::vector<LogEvent> &getLogEvents() const;

  /**
   * Gets the collection of XY series from the parsed file
   * `parse()` should be called first
   *
   * @return The XY series specified by the parsed file
   */
  [[nodiscard]] const std::vector<XYSeries> &getXYSeries() const;

  /**
   * Gets the collection of Category Value series from the parsed file
   * `parse()` should be called first
   *
   * @return The Category Value series specified by the parsed file
   */
  [[nodiscard]] const std::vector<CategoryValueSeries> &getCategoryValueSeries() const;

  /**
   * Gets the collection of series collections from the parsed file
   * `parse()` should be called first
   *
   * @return The series collections specified by the parsed file
   */
  [[nodiscard]] const std::vector<SeriesCollection> &getSeriesCollections() const;

  /**
   * Gets the collection of LogStreams for the Scenario Log controller from the parsed file
   * `parse()` should be called first.
   *
   *
   * @return The LogStreams specified by the parsed file
   */
  [[nodiscard]] const std::vector<LogStream> &getLogStreams() const;

private:
  /**
   * The overall configuration of the simulation
   */
  GlobalConfiguration globalConfiguration;

  /**
   * The Nodes defined by the 'nodes' JSON collection
   */
  std::vector<Node> nodes;

  /**
   * The Buildings defined by the 'buildings' JSON collection
   */
  std::vector<Building> buildings;

  /**
   * The Decorations defined by the 'decorations' JSON collection
   */
  std::vector<Decoration> decorations;

  /**
   * The areas defined by the 'areas' JSON collection
   */
  std::vector<Area> areas;

  /**
   * The events for the rendered scene defined by the 'events' JSON collection
   */
  std::vector<SceneEvent> sceneEvents;

  /**
   * The Events for the charts module defined by the 'events' JSON collection
   */
  std::vector<ChartEvent> chartEvents;

  /**
   * The events for the Log module defined by the 'events' JSON collection
   */
  std::vector<LogEvent> logEvents;

  /**
   * The XY Series defined within the 'series' JSON collection
   */
  std::vector<XYSeries> xySeries;

  /**
   * The Category Value Series defined within the 'series' JSON collection
   */
  std::vector<CategoryValueSeries> categoryValueSeries;

  /**
   * The Series Collections defined within the 'series' JSON collection
   */
  std::vector<SeriesCollection> seriesCollections;

  /**
   * The Streams defined within the 'streams' JSON collection
   */
  std::vector<LogStream> logStreams;
};

} // namespace parser
