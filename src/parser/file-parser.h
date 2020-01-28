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
#include "../event/model.h"
#include "model.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <string>
#include <vector>

namespace visualization {

class FileParser {
public:
  /**
   * States for any tags that have data actually within the tags
   * (not all in the attributes)
   */
  enum class Tag { None, MsPerFrame, SeriesCollection };

  /**
   * States for each collection in the document
   */
  enum class Section { None, Configuration, Nodes, Buildings, Decorations, Series, Events };

  /**
   * Setup the internal callbacks for the parser
   */
  FileParser();

  /**
   * Read the XML file specified by path,
   * sets the configuration, nodes, etc.
   *
   * @param path
   * The path to the XML file
   */
  void parse(const char *path);

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
   *  Gets the collection of decorations from the parsed file
   * `parse()` should be called first
   *
   * @return The decorations specified by the parsed file
   */
  [[nodiscard]] const std::vector<Decoration> &getDecorations() const;

  /**
   * Gets the collection of events from the parsed file
   * `parse()` should be called first
   *
   * @return The events specified by the parsed file
   */
  [[nodiscard]] const std::vector<Event> &getEvents() const;

  /**
   * Gets the collection of XY series from the parsed file
   * `parse()` should be called first
   *
   * @return The XY series specified by the parsed file
   */
  [[nodiscard]] const std::vector<XYSeries> &getXYSeries() const;

  /**
   * Gets the collection of series collections from the parsed file
   * `parse()` should be called first
   *
   * @return The series collections specified by the parsed file
   */
  [[nodiscard]] const std::vector<SeriesCollection> &getSeriesCollections() const;
private:
  /**
   * Handle the data in between an opening and closing tag.
   * Based on the `currentTag`.
   *
   * @param data
   * The content of the current tag
   */
  void interpretCharacters(const std::string &data);

  /**
   * Parse and emplace a node.
   *
   * @param attributes
   * The attribute array from the 'node' tag
   */
  void parseNode(const xmlChar *attributes[]);

  /**
   * Parse and emplace a building
   *
   * @param attributes
   * The attribute array from the 'building' tag
   */
  void parseBuilding(const xmlChar *attributes[]);

  /**
   * Parse and emplace a decoration
   *
   * @param attributes
   * The attribute array from the 'decoration' tag
   */
  void parseDecoration(const xmlChar *attributes[]);

  /**
   * Parse and emplace a linear series
   *
   * @param attributes
   * The attribute array from the 'linear-series' tag
   */
  void parseXYSeries(const xmlChar *attributes[]);

  /**
   * Parse and emplace a the attributes of a series collection
   *
   * @param attributes
   * The attribute array from the 'series-collection' tag
   */
  void parseSeriesCollection(const xmlChar *attributes[]);

  /**
   * Parse and emplace a the attributes of a child series
   * into the last emplace SeriesCollection
   *
   * @param attributes
   * The attribute array from the 'child-series' tag
   */
  void parseChildSeries(const xmlChar *attributes[]);

  /**
   * Parse and emplace a move event
   *
   * @param attributes
   * The attribute array from the 'position' tag
   */
  void parseMoveEvent(const xmlChar *attributes[]);

  /**
   * Parse and emplace a series append event
   *
   * @param attributes
   * The attribute array from the 'series-add' tag
   */
  void parseSeriesAppend(const xmlChar *attributes[]);

  /**
   * SAX Handler for when the parser detects a tag has been opened
   *
   * @param user_data
   * A pointer to the current FileParser
   *
   * @param name
   * The tag name
   *
   * @param attrs
   * The attribute array. May be null
   */
  static void startElementCallback(void *user_data, const xmlChar *name, const xmlChar **attrs);

  /**
   * SAX Handler for when the parser detects tag content
   *
   * @param user_data
   * A pointer to the current FileParser
   *
   * @param characters
   * A C string containing (at least) the content of the tag
   *
   * @param length
   * The length of `characters` containing the content of the tag
   */
  static void charactersCallback(void *user_data, const xmlChar *characters, int length);

  /**
   * SAX Handler for when the parser detects a tag has been closed
   *
   * @param user_data
   * A pointer to the current FileParser
   *
   * @param name
   * The tag name
   */
  static void endElementCallback(void *user_data, const xmlChar *name);

  /**
   * The overall configuration of the simulation
   */
  GlobalConfiguration globalConfiguration;

  /**
   * The Nodes defined by the 'nodes' XML collection
   */
  std::vector<Node> nodes;

  /**
   * The Buildings defined by the 'buildings' XML collection
   */
  std::vector<Building> buildings;

  /**
   * The Decorations defined by the 'decorations' XML collection
   */
  std::vector<Decoration> decorations;

  /**
   * The Events defined by the 'events' XML collection
   */
  std::vector<Event> events;

  /**
   * The XY Series defined within the 'series' XML collection
   */
  std::vector<XYSeries> xySeries;

  /**
   * The Series Collections defined within the 'series' XML collection
   */
  std::vector<SeriesCollection> seriesCollections;

  /**
   * The handler from libxml2
   * manages the tag callbacks
   */
  xmlSAXHandler saxHandler{};

  /**
   * Indicator of what tag we are in.
   * Used for parsing character data within a tag
   */
  Tag currentTag = Tag::None;

  /**
   * Indicator of what section/collection we are currently in
   */
  Section currentSection = Section::None;
};

} // namespace visualization
