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

#include "event/model.h"
#include "group/building/BuildingGroup.h"
#include "group/decoration/DecorationGroup.h"
#include "group/node/NodeGroup.h"
#include "hud/hud.h"
#include "parser/file-parser.h"
#include "util/CoordinateGrid.h"
#include "window/mainWindow.h"
#include "window/osgWidget.h"
#include <QApplication>
#include <QSurfaceFormat>
#include <deque>
#include <iostream>
#include <osgGA/OrbitManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/config/SingleWindow>
#include <unordered_map>

// Compile time checking for a member nodeId
template <typename T, typename = int> struct hasNodeId : std::false_type {}; // Default case

template <typename T>
struct hasNodeId<T, decltype((void)T::nodeId, int()) // Checking if ::nodeId exists, and its of type uint32_t. If
                                                     // it doesn't exist then this specialization is ignored (SFINAE)
                 > : std::true_type {};

template <typename T, typename = int> struct hasSeriesId : std::false_type {};
template <typename T> struct hasSeriesId<T, decltype((void)T::seriesId, int())> : std::true_type {};

template <typename T, typename = int> struct hasDecorationId : std::false_type {};
template <typename T> struct hasDecorationId<T, decltype((void)T::decorationId, int())> : std::true_type {};

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Error: file argument required\n"
              << "Usage: " << argv[0] << " (output_file.xml)\n";

    return 1;
  }

  visualization::FileParser parser;
  parser.parse(argv[1]);

  const auto &config = parser.getConfiguration();
  const auto &nodes = parser.getNodes();

  osg::ref_ptr<osg::Group> root = new osg::Group();

  std::unordered_map<uint32_t, osg::ref_ptr<visualization::NodeGroup>> nodeGroups;
  nodeGroups.reserve(nodes.size());
  for (auto &node : nodes) {
    auto nodeGroup = visualization::NodeGroup::MakeGroup(node);
    nodeGroups.insert({node.id, nodeGroup});
    root->addChild(nodeGroup);
  }

  const auto &buildings = parser.getBuildings();
  for (auto &building : buildings) {
    auto group = visualization::BuildingGroup::makeGroup(building);
    root->addChild(group);
  }

  const auto &decorations = parser.getDecorations();
  std::unordered_map<uint32_t, osg::ref_ptr<visualization::DecorationGroup>> decorationGroups;
  decorationGroups.reserve(decorations.size());
  for (const auto &decoration : decorations) {
    auto group = new visualization::DecorationGroup(decoration);
    decorationGroups.insert({decoration.id, group});
    root->addChild(group);
  }

  std::deque<visualization::ChartEvent> chartEvents;
  const auto &events = parser.getEvents();
  for (auto &event : events) {
    std::visit(
        [&nodeGroups, &decorationGroups, &chartEvents](auto &&arg) {
          // Strip off qualifiers, etc
          // so T holds just the type
          // so we can more easily match it
          using T = std::decay_t<decltype(arg)>;

          // Only enqueue events with nodeIds's
          if constexpr (hasNodeId<T>::value)
            nodeGroups[arg.nodeId]->enqueueEvent(arg);
          else if constexpr (hasSeriesId<T>::value)
            chartEvents.emplace_back(arg);
          else if constexpr (hasDecorationId<T>::value)
            decorationGroups[arg.decorationId]->enqueueEvent(arg);
        },
        event);
  }

  root->addChild(new visualization::CoordinateGrid(100));

  QApplication application(argc, argv);

  QSurfaceFormat format;
  format.setVersion(2, 1);
  format.setProfile(QSurfaceFormat::CompatibilityProfile);

  QSurfaceFormat::setDefaultFormat(format);

  visualization::MainWindow mainWindow(config, chartEvents, parser, root);
  mainWindow.show();
  return QApplication::exec();
}
