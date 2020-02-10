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
#include <json.hpp>
#include <optional>
#include <osg/Math>
#include <stack>
#include <string>
#include <vector>

namespace visualization {

class FileParser {
  /**
   * Multiply by this constant to convert degrees to radians
   */
  const static inline double TO_RADIANS = osg::PI / 180;

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

  /**
   * Called when the event parser encounters a null value.
   * Used by the parser. Should not be called by user code.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool null();

  /**
   * Called when the event parser encounters a boolean value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool boolean(bool value);

  /**
   * Called when the event parser encounters an integer value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool number_integer(nlohmann::json::number_integer_t value);

  /**
   * Called when the event parser encounters a unsigned integer value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool number_unsigned(nlohmann::json::number_unsigned_t value);

  /**
   * Called when the event parser encounters a floating point value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @param raw
   * The raw string consumed by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool number_float(nlohmann::json::number_float_t value, const nlohmann::json::string_t &raw);

  /**
   * Called when the event parser encounters a string value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool string(nlohmann::json::string_t &value);

  /**
   * Called when the parser encounters the beginning of an object.
   * Used by the parser. Should not be called by user code.
   *
   * @param elements
   * The size of the object. -1 if unknown.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool start_object(std::size_t elements);

  /**
   * Called when the parser encounters the end of an object.
   * Used by the parser. Should not be called by user code.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool end_object();

  /**
   * Called when the parser encounters the beginning of an array.
   * Used by the parser. Should not be called by user code.
   *
   * @param elements
   * The size of the array. -1 if unknown.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool start_array(std::size_t elements);

  /**
   * Called when the parser encounters the end of an array.
   * Used by the parser. Should not be called by user code.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool end_array();

  /**
   * Called when the parser reads a key for a value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value of the encountered key.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool key(nlohmann::json::string_t &value);

  /**
   * Called when the parser encounters an error.
   * Used by the parser. Should not be called by user code.
   *
   * @param position
   * The position in the document
   *
   * @param last_token
   * The last read token
   *
   * @param ex
   * The exception from the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool parse_error(std::size_t position, const std::string &last_token, const nlohmann::detail::exception &ex);

private:
  /**
   * The possible section in the document
   */
  enum class Section { None, Buildings, Configuration, Decorations, Events, Nodes, Series };

  /**
   * Parse a section from a string.
   * Returns Section::None if the string does not match a Section.
   *
   * @param key
   * The key from the parser.
   *
   * @return
   * The parsed Section. Section::None if key does not match a Section.
   */
  static constexpr Section isSection(std::string_view key);

  /**
   * The current section we're in the document.
   */
  Section currentSection = Section::None;

  /**
   * A frame for the JSON stack
   */
  struct JsonFrame {
    std::string key;
    nlohmann::json value;
  };

  /**
   * Stack representing a JSON object
   */
  std::stack<JsonFrame> jsonStack;

  /**
   * The current key we're processing.
   * Not set if we have not encountered a key
   */
  std::optional<std::string> currentKey;

  /**
   * How many levels into a given section we're in.
   */
  int sectionDepth = 0;

  /**
   * Handle a given single value for a key.
   *
   * @tparam T
   * The type of value to store in the currentKey
   *
   * @param value
   * The value to store in currentKey
   */
  template <typename T> void handle(T &&value) {
    if (jsonStack.empty() || !currentKey)
      return;

    auto &top = jsonStack.top();
    if (top.value[*currentKey].type() == nlohmann::json::value_t::array)
      top.value[*currentKey].emplace_back(value);
    else
      top.value[*currentKey] = value;
  }

  /**
   * Parse a JSON object into a model and emplace it.
   *
   * @param section
   * The current Section, used to determine the type of object.
   *
   * @param object
   * The JSON object to parse.
   */
  void do_parse(Section section, const nlohmann::json &object);

  /**
   * Parse and set the configuration.
   *
   * @param object
   * The object from the 'configuration' section
   */
  void parseConfiguration(const nlohmann::json &object);

  /**
   * Parse and emplace a node.
   *
   * @param object
   * The object from the 'nodes' section
   */
  void parseNode(const nlohmann::json &object);

  /**
   * Parse and emplace a building
   *
   * @param object
   * The object from the 'buildings' section
   */
  void parseBuilding(const nlohmann::json &object);

  /**
   * Parse and emplace a decoration
   *
   * @param object
   * The object from the 'decoration' section
   */
  void parseDecoration(const nlohmann::json &object);

  /**
   * Parse and emplace a linear series
   *
   * @param object
   * The object from the 'series' section with the 'xy-series' type
   */
  void parseXYSeries(const nlohmann::json &object);

  /**
   * Parse and emplace a series collection
   *
   * @param object
   * The object from the 'series' section with the 'xy-series' type
   */
  void parseSeriesCollection(const nlohmann::json &object);

  /**
   * Parse and emplace a move event
   *
   * @param object
   * The object from the 'events' section with the 'node-position' type
   */
  void parseMoveEvent(const nlohmann::json &object);

  /**
   * Parse and emplace a DecorationMoveEvent
   *
   * @param object
   * The object from the 'events' section with the 'decoration-position' type
   */
  void parseDecorationMoveEvent(const nlohmann::json &object);

  /**
   * Parse and emplace a NodeOrientationEvent
   *
   * @param object
   * The object from the 'events' section with the 'node-orientation' type
   */
  void parseNodeOrientationEvent(const nlohmann::json &object);

  /**
   * Parse and emplace a DecorationOrientationEvent
   *
   * @param object
   * The object from the 'events' section with the 'decoration-position' type
   */
  void parseDecorationOrientationEvent(const nlohmann::json &object);

  /**
   * Parse and emplace a series append event
   *
   * @param object
   * The object from the 'events' section with the 'xy-series-append' type
   */
  void parseSeriesAppend(const nlohmann::json &object);

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
   * The Events defined by the 'events' JSON collection
   */
  std::vector<Event> events;

  /**
   * The XY Series defined within the 'series' JSON collection
   */
  std::vector<XYSeries> xySeries;

  /**
   * The Series Collections defined within the 'series' JSON collection
   */
  std::vector<SeriesCollection> seriesCollections;
};

} // namespace visualization
