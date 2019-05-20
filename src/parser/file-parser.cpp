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
#include <utility>
#include <cassert>
#include <cassert>
#include <memory>

/**
 * namespace containing wrappers for Libxml2 functions returning smart pointers
 * with the appropriate deleter functions
 */
namespace {

using smartXmlString = std::unique_ptr<xmlChar, decltype(xmlFree)>;

std::unique_ptr<xmlXPathContext, decltype(&xmlXPathFreeContext)> makeUniqueContext(xmlDocPtr doc) {
  return {xmlXPathNewContext(doc), xmlXPathFreeContext};
}

std::unique_ptr<xmlXPathObject, decltype(&xmlXPathFreeObject)> xPathEvalUnique(const char *path,
                                                                               xmlXPathContextPtr context) {
  return {xmlXPathEvalExpression(BAD_CAST path, context), xmlXPathFreeObject};
}

smartXmlString xmlGetStringUnique(xmlDocPtr doc, const xmlNode *nodeList, int inLine = 1) {
  return {xmlNodeListGetString(doc, nodeList, inLine), xmlFree};
}

smartXmlString xmlGetPropUnique(const xmlNode *node, const char *name) {
  return {xmlGetProp(node, BAD_CAST name), xmlFree};
}

}

namespace visualization {

FileParser::FileParser(std::string path) : path(std::move(path)) {
  LIBXML_TEST_VERSION
  // We have to explicitly use this->path since the parameter was moved
  doc = xmlReadFile(this->path.c_str(), nullptr, 0);
  assert(doc != nullptr);
}

FileParser::~FileParser() {
  xmlFreeDoc(doc);
  xmlCleanupParser();
}

GlobalConfiguration FileParser::readGlobalConfiguration() {
  xmlNodePtr cursor = xmlDocGetRootElement(doc);
  assert(cursor != nullptr);
  GlobalConfiguration config{};

  auto context = makeUniqueContext(doc);
  auto result = xPathEvalUnique("/visualizer3d/configuration/*", context.get());

  auto nodeSet = result->nodesetval;
  for (auto i = 0; i < nodeSet->nodeNr; i++) {
    auto node = nodeSet->nodeTab[i];
    if (xmlStrEqual(node->name, BAD_CAST "ms-per-frame")) {
      // The value of an XML node (i.e. <element>value</element>) is considered its child
      // so we pass node->children to xmlNodeListGetString to parse it
      auto value = xmlGetStringUnique(doc, node->children);
      assert(value != nullptr); // No configuration element may be valueless

      config.millisecondsPerFrame = std::stod(reinterpret_cast<const char *> (value.get()));
    }
  }

  return config;
}

std::vector<Node> FileParser::readNodes() {
  xmlNodePtr cursor = xmlDocGetRootElement(doc);
  assert(cursor != nullptr);

  auto context = makeUniqueContext(doc);
  auto result = xPathEvalUnique("/visualizer3d/nodes/*", context.get());

  auto nodeSet = result->nodesetval;

  std::vector<Node> nodes;
  nodes.reserve(nodeSet->nodeNr);
  for (auto i = 0; i < nodeSet->nodeNr; i++) {
    nodes.push_back(parseNode(nodeSet->nodeTab[i]));
  }

  return nodes;
}

Node FileParser::parseNode(xmlNodePtr cursor) {
  Node node{};

  auto value = xmlGetPropUnique(cursor, "id");
  if (value != nullptr) {
    node.id = std::stoul(reinterpret_cast<const char *>(value.get()));
  }

  value = xmlGetPropUnique(cursor, "model");
  if (value != nullptr) {
    node.model = reinterpret_cast<const char *>(value.get());
  }

  value = xmlGetPropUnique(cursor, "scale");
  if (value != nullptr) {
    node.scale = std::stod(reinterpret_cast<const char *>(value.get()));
  }

  value = xmlGetPropUnique(cursor, "opacity");
  if (value != nullptr) {
    node.opacity = std::stod(reinterpret_cast<const char *>(value.get()));
  }

  value = xmlGetPropUnique(cursor, "visible");
  if (value != nullptr) {
    node.visible = xmlStrEqual(value.get(), BAD_CAST "1");
  }

  value = xmlGetPropUnique(cursor, "x");
  if (value != nullptr) {
    node.position[0] = std::stod(reinterpret_cast<const char *>(value.get()));
  }

  value = xmlGetPropUnique(cursor, "y");
  if (value != nullptr) {
    node.position[1] = std::stod(reinterpret_cast<const char *>(value.get()));
  }

  value = xmlGetPropUnique(cursor, "z");
  if (value != nullptr) {
    node.position[2] = std::stod(reinterpret_cast<const char *>(value.get()));
  }

  return node;
}

} // namespace visualization
