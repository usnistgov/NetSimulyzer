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
#include "../../event/model.h"
#include "../../parser/model.h"
#include <deque>
#include <osg/Geode>
#include <osg/Group>
#include <osg/MatrixTransform>
#include <osg/NodeCallback>
#include <osg/PositionAttitudeTransform>
#include <osg/Switch>
#include <osg/ref_ptr>

namespace visualization {

class NodeGroupEventCallback : public osg::NodeCallback {
public:
  void operator()(osg::Node *node, osg::NodeVisitor *nv) override;
};

/**
 * Top level Group Node to manage all components
 * of a Node From ns-3
 */
class NodeGroup : public osg::Group {
  friend NodeGroupEventCallback;
  NodeGroup() = default;

  META_Node(osg, NodeGroup);
  std::deque<Event> events;
  osg::ref_ptr<osg::Switch> visible;
  osg::ref_ptr<osg::PositionAttitudeTransform> position;
  osg::ref_ptr<osg::MatrixTransform> scale;
  osg::ref_ptr<osg::Geode> geode;

public:
  static osg::ref_ptr<NodeGroup> MakeGroup(const visualization::Node &config);
  explicit NodeGroup(const Group &Group, const osg::CopyOp &Copyop = osg::CopyOp::SHALLOW_COPY)
      : osg::Group(Group, Copyop) {
  }

  void enqueueEvent(const Event &event);
};

} // namespace visualization