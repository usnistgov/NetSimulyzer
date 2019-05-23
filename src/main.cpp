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

#include "group/node/NodeGroup.h"
#include "parser/file-parser.h"
#include <iostream>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <unordered_map>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    std::cerr << "Error: file argument required\n"
              << "Usage: " << argv[1] << " (output_file.xml)\n";

    return 1;
  }

  visualization::FileParser parser{argv[1]};
  auto config = parser.readGlobalConfiguration();
  auto nodes = parser.readNodes();

  osg::ref_ptr<osg::Group> root = new osg::Group();

  std::unordered_map<uint32_t, osg::ref_ptr<visualization::NodeGroup>> nodeGroups;
  nodeGroups.reserve(nodes.size());
  for (auto &node : nodes) {
    auto nodeGroup = visualization::NodeGroup::MakeGroup(node);
    nodeGroups.insert({node.id, nodeGroup});
    root->addChild(nodeGroup);
  }

  auto building = parser.readBuildings();
  auto events = parser.readEvents();
  for (auto &event : events) {
    std::visit([&nodeGroups](auto &&arg) { nodeGroups[arg.nodeId]->enqueueEvent(arg); }, event);
  }

  osgViewer::Viewer viewer;
  viewer.setUpViewInWindow(0, 0, 640, 480);
  viewer.setSceneData(root);
  viewer.setCameraManipulator(new osgGA::TrackballManipulator());
  viewer.realize();

  double currentTime = 0.0;
  while (!viewer.done()) {
    viewer.frame(currentTime += config.millisecondsPerFrame);
  }
}
