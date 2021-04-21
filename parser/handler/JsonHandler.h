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
#include "../file-parser.h"
#include "Json.h"
#include "model.h"
#include <cassert>
#include <fstream>
#include <iostream>
#include <memory>
#include <optional>
#include <rapidjson/reader.h>
#include <stack>
#include <string>
#include <vector>

class JsonHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, JsonHandler> {
  /**
   * The possible section in the document
   */
  enum class Section { None, Areas, Buildings, Configuration, Decorations, Events, Nodes, Series, Streams };

  parser::FileParser &fileParser;

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
    util::json::JsonValue value;
  };

  /**
   * Stack representing a JSON object
   */
  std::stack<JsonFrame> jsonStack;

  /**
   * Handle a given single value for a key.
   *
   * @tparam T
   * They type of value to store
   *
   * @param value
   * The value to store
   */
  template <typename T>
  void handle(T &&value) {
    // TODO: Probably an error if this happens
    if (jsonStack.empty())
      std::abort();

    // Special case, array of primitives
    // ex: [1, 2, 3]
    if (jsonStack.top().value.isArray()) {
      jsonStack.top().value.array().emplace_back(value);
      return;
    }

    // Copy the old top, since we're about to clear it
    // should contain just a key
    // since `handle()` is for primitives
    auto oldTop = jsonStack.top();

    if (!oldTop.value.isNull()) {
      std::cerr << oldTop.key << '\n';
      std::abort();
    }

    jsonStack.pop();
    auto &currentTop = jsonStack.top();

    if (currentTop.value.isObject()) {
      currentTop.value.object().insert(oldTop.key, value);
      return;
    }

    std::abort();
    // TODO: Error as well...
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
  void do_parse(Section section, const util::json::JsonObject &object);

  /**
   * Parse and set the configuration.
   *
   * @param object
   * The object from the 'configuration' section
   */
  void parseConfiguration(const util::json::JsonObject &object);

  /**
   * Parse and emplace a node.
   *
   * @param object
   * The object from the 'nodes' section
   */
  void parseNode(const util::json::JsonObject &object);

  /**
   * Parse and emplace a building
   *
   * @param object
   * The object from the 'buildings' section
   */
  void parseBuilding(const util::json::JsonObject &object);

  /**
   * Parse and emplace a decoration
   *
   * @param object
   * The object from the 'decoration' section
   */
  void parseDecoration(const util::json::JsonObject &object);

  /**
   * Parse and emplace an area
   *
   * @param object
   * The object from the 'areas' section
   */
  void parseArea(const util::json::JsonObject &object);

  /**
   * Parse and emplace a move event
   *
   * @param object
   * The object from the 'events' section with the 'node-position' type
   */
  void parseMoveEvent(const util::json::JsonObject &object);

  /**
   * Parse and emplace a DecorationMoveEvent
   *
   * @param object
   * The object from the 'events' section with the 'decoration-position' type
   */
  void parseDecorationMoveEvent(const util::json::JsonObject &object);

  /**
   * Parse and emplace a NodeOrientationEvent
   *
   * @param object
   * The object from the 'events' section with the 'node-orientation' type
   */
  void parseNodeOrientationEvent(const util::json::JsonObject &object);

  /**
   * Parse and emplace a DecorationOrientationEvent
   *
   * @param object
   * The object from the 'events' section with the 'decoration-position' type
   */
  void parseDecorationOrientationEvent(const util::json::JsonObject &object);

  /**
   * Parse and emplace a NodeColorChange event
   *
   * @param object
   */
  void parseNodeColorChangeEvent(const util::json::JsonObject &object);

  /**
   * Parse and emplace a series append event
   *
   * @param object
   * The object from the 'events' section with the 'xy-series-append' type
   */
  void parseSeriesAppend(const util::json::JsonObject &object);

  /**
   * Parse and emplace a category value append event
   *
   * @param object
   * The object from the 'events' section with the 'category-series-append' type
   */
  void parseCategorySeriesAppend(const util::json::JsonObject &object);

  /**
   * Parse and emplace a linear series
   *
   * @param object
   * The object from the 'series' section with the 'xy-series' type
   */
  void parseXYSeries(const util::json::JsonObject &object);

  /**
   * Parse and emplace a category value series.
   *
   * @param object
   * The object from the 'series' section with the 'category-value-series' type
   */
  void parseCategoryValueSeries(const util::json::JsonObject &object);

  /**
   * Parse and emplace a series collection
   *
   * @param object
   * The object from the 'series' section with the 'xy-series' type
   */
  void parseSeriesCollection(const util::json::JsonObject &object);

  /**
   * Parse and emplace a Stream for the Scenario Log
   *
   * @param object
   * The object from the 'streams' section
   */
  void parseLogStream(const util::json::JsonObject &object);

  /**
   * Parse and emplace a stream append event
   *
   * @param object
   * The object from the 'events' section with the 'stream-append' type
   */
  void parseStreamAppend(const util::json::JsonObject &object);

  /**
   * Check the min/max bounds against `coordinate` and update accordingly
   *
   * @param coordinate
   * The coordinate to check against the scenario min/max locations
   */
  void updateLocationBounds(const parser::Ns3Coordinate &coordinate);

  /**
   * Update the end of simulation time to `milliseconds`
   * given `milliseconds` is the larger value
   *
   * @param milliseconds
   * The time for the currently being parsed event
   * in milliseconds
   */
  void updateEndTime(double milliseconds);

public:
  explicit JsonHandler(parser::FileParser &parser);

  // Note: do not make the below functions `virtual`
  // or mark them with `override
#pragma clang diagnostic push
#pragma ide diagnostic ignored "HidingNonVirtualFunction"

  /**
   * Called when the event parser encounters a null value.
   * Used by the parser. Should not be called by user code.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool Null();

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
  bool Bool(bool value);

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
  bool Int(int value);

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
  bool Uint(unsigned int value);

  /**
   * Called when the event parser encounters an integer value
   * 64 bits in width.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool Int64(std::int64_t value);

  /**
   * Called when the event parser encounters an unsigned integer value
   * 64 bits in width.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool Uint64(std::uint64_t value);

  /**
   * Called when the event parser encounters a floating point value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool Double(double value);

  /**
   * Called when the event parser encounters a string value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value encountered by the parser
   *
   * @param length
   * Length of the passed string
   *
   * @param copy
   * flag indicating the string should be copied.
   * Will probably always be true for our parsing method.
   * We'll copy anyway...
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool String(const char *value, rapidjson::SizeType length, bool copy);

  /**
   * Called when the parser encounters the beginning of an object.
   * Used by the parser. Should not be called by user code.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool StartObject();

  /**
   * Called when the parser reads a key for a value.
   * Used by the parser. Should not be called by user code.
   *
   * @param value
   * The value of the encountered key.
   *
   * @param length
   * Length of the passed string
   *
   * @param copy
   * flag indicating the string should be copied.
   * Will probably always be true for our parsing method.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool Key(const char *value, rapidjson::SizeType length, bool copy);

  /**
   * Called when the parser encounters the end of an object.
   * Used by the parser. Should not be called by user code.
   *
   * @param memberCount
   * Number of members in the object
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool EndObject(rapidjson::SizeType memberCount);

  /**
   * Called when the parser encounters the beginning of an array.
   * Used by the parser. Should not be called by user code.
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool StartArray();

  /**
   * Called when the parser encounters the end of an array.
   * Used by the parser. Should not be called by user code.
   *
   * @param elementCount
   * Number of elements in the array
   *
   * @return
   * True if the parse result should be used. False otherwise
   */
  bool EndArray(rapidjson::SizeType elementCount);

#pragma clang diagnostic pop
};
