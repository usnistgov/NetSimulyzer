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

#include "NodeGroup.h"
#include <osg/BlendFunc>
#include <osg/Material>
#include <osgDB/ReadFile>
#include <type_traits>

namespace visualization {

osg::ref_ptr<NodeGroup> NodeGroup::MakeGroup(const visualization::Node &config) {
  osg::ref_ptr<NodeGroup> node = new NodeGroup();

  // TODO Add an optimization for many nodes using the same model
  osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFile(config.model);

  if (config.opacity < 1) {
    auto stateSet = model->getOrCreateStateSet();

    auto attr = stateSet->getAttribute(osg::StateAttribute::MATERIAL);
    osg::ref_ptr<osg::Material> material;

    if (attr)
      material = dynamic_cast<osg::Material *>(attr);
    else
      material = new osg::Material;

    material->setColorMode(osg::Material::ColorMode::SPECULAR);
    material->setAlpha(osg::Material::FRONT_AND_BACK, static_cast<float>(config.opacity));
    stateSet->setAttributeAndModes(material, osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE);

    stateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    model->setStateSet(stateSet);

    model->getOrCreateStateSet()->setAttributeAndModes(
        new osg::BlendFunc(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA));
  }

  // TODO check if the model actually exists
  node->geode = new osg::Geode();
  node->geode->addChild(model);

  osg::Vec3d scale{config.scale, config.scale, config.scale};
  node->scale = new osg::MatrixTransform(osg::Matrix::scale(scale));
  node->scale->addChild(node->geode);

  node->position = new osg::PositionAttitudeTransform();
  node->position->setPosition(config.position);
  node->position->addChild(node->scale);

  node->visible = new osg::Switch();
  node->visible->addChild(node->position, config.visible);

  node->addChild(node->visible);
  node->setUpdateCallback(new NodeGroupEventCallback());

  return node;
}

void NodeGroupEventCallback::operator()(osg::Node *node, osg::NodeVisitor *nv) {

  auto time = nv->getFrameStamp()->getSimulationTime();

  // We know the type since this callback
  // may only be attached to a NodeGroup
  auto &group = *static_cast<NodeGroup *>(node);
  auto &events = group.events;

  // Returns true after handling an event
  // false otherwise
  auto handleEvent = [&time, &group, &events](auto &&arg) {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(arg)>;

    // All events have a time
    // Make sure we don't handle one in the future
    if (arg.time > time)
      return false;

    if constexpr (std::is_same_v<T, MoveEvent>) {
      group.position->setPosition(arg.targetPosition);
      events.pop_front();
      return true;
    }
  };

  // Handle current/past events
  // Using the return std::visit to keep handling events
  // only works if the events are sorted
  while (!events.empty() && std::visit(handleEvent, events.front())) {
    // Intentionally Blank
  }

  traverse(node, nv);
}

void NodeGroup::enqueueEvent(const Event &event) {
  events.push_back(event);
}

} // namespace visualization
