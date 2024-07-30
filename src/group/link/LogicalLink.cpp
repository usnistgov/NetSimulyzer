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

#include "LogicalLink.h"
#include "src/conversion.h"
#include "src/render/model/Model.h"
#include "src/util/undo-events.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/norm.hpp>
#include <glm/gtx/quaternion.hpp>
#include <model.h>
#include <utility>

namespace {
glm::quat rotationBetweenVectors(glm::vec3 start, glm::vec3 dest) {
  start = glm::normalize(start);
  dest = glm::normalize(dest);

  const float cosTheta = glm::dot(start, dest);

  glm::vec3 rotationAxis;
  if (cosTheta < -1 + 0.001f) {
    // special case when vectors in opposite directions:
    // there is no "ideal" rotation axis
    // So guess one any will do as long as it's perpendicular to start
    rotationAxis = glm::cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
    if (glm::length2(rotationAxis) < 0.01) // parallel, it's the other axis
      rotationAxis = glm::cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

    rotationAxis = normalize(rotationAxis);
    return glm::angleAxis(glm::radians(180.0f), rotationAxis);
  }

  rotationAxis = glm::cross(start, dest);

  const auto s = std::sqrt((1 + cosTheta) * 2);
  const auto sInverse = 1.0f / s;

  return {s * 0.5f, rotationAxis.x * sInverse, rotationAxis.y * sInverse, rotationAxis.z * sInverse};
}

} // namespace

namespace netsimulyzer {

LogicalLink::LogicalLink(parser::LogicalLink model, const Model::ModelLoadInfo &linkCylinder)
    : model{std::move(model)}, linkCylinder{linkCylinder}, modelWidth{linkCylinder.max.x - linkCylinder.min.x},
      modelHeight{linkCylinder.max.y - linkCylinder.min.y}, color{toRenderColor(this->model.color)} {
}

const parser::LogicalLink &LogicalLink::getModel() const {
  return model;
}

void LogicalLink::update(const glm::vec3 node1Position, const glm::vec3 node2Position) {

  // TODO: move to event processing
  if (node1Position == oldPositions.first && node2Position == oldPositions.second)
    return;
  oldPositions = {node1Position, node2Position};
  updateModelMatrix(node1Position, node2Position);
}

void LogicalLink::update() {
  updateModelMatrix(oldPositions.first, oldPositions.second);
}

void LogicalLink::updateModelMatrix(const glm::vec3 node1Position, const glm::vec3 node2Position) {
  const auto direction = node1Position - node2Position;

  // Find the rotation between the front of the object (+X) and the desired direction
  const auto rot1 = rotationBetweenVectors({1.0f, 0.0f, 0.0f}, direction);

  // Recompute desiredUp so that it's perpendicular to the direction
  // You can skip that part if you really want to force desiredUp
  const auto right = cross(direction, {0.0f, 0.0f, 1.0f});
  const auto up = cross(right, direction);

  // Because of the 1st rotation, the up is probably completely screwed up.
  // Find the rotation between the "up" of the rotated object, and the desired up
  const auto newUp = rot1 * glm::vec3(0.0f, 1.0f, 0.0f);
  const auto rot2 = rotationBetweenVectors(newUp, up);

  // Build model matrix
  modelMatrix = glm::mat4{1.0f};

  const auto position = glm::vec3{(node1Position.x + node2Position.x) / 2, (node1Position.y + node2Position.y) / 2,
                                  (node1Position.z + node2Position.z) / 2};

  const auto distance = std::hypot(std::hypot(node1Position.x - node2Position.x, node1Position.y - node2Position.y),
                                   node1Position.z - node2Position.z);
  const auto lengthScale = distance / modelWidth;
  const auto diameterScale = model.diameter / modelHeight;

  modelMatrix = glm::translate(modelMatrix, position);
  // Rotate
  modelMatrix *= glm::toMat4(rot2 * rot1);
  modelMatrix = glm::scale(modelMatrix, glm::vec3{lengthScale, diameterScale, diameterScale});
}

undo::LogicalLinkUpdate LogicalLink::handle(const parser::LogicalLinkUpdate &e) {
  undo::LogicalLinkUpdate undo;
  undo.event = e;
  undo.nodes = model.nodes;
  undo.active = model.active;
  undo.color = model.color;
  undo.diameter = model.diameter;

  model.nodes = e.nodes;
  model.active = e.active;
  model.color = e.color;
  model.diameter = e.diameter;

  color = toRenderColor(model.color);

  // If the nodes match, then we can
  // update using the old positions,
  // if not, then the `update()` call
  // in the `SceneWidget` with the
  // positions will update this
  if (undo.nodes == model.nodes)
    update();

  return undo;
}

void LogicalLink::handle(const undo::LogicalLinkUpdate &e) {
  bool doUpdate = model.nodes == e.nodes;

  model.nodes = e.nodes;
  model.active = e.active;
  model.color = e.color;
  model.diameter = e.diameter;

  color = toRenderColor(model.color);

  // Same deal as `handle` for the
  // non-undo version of this event
  if (doUpdate)
    update();
}

} // namespace netsimulyzer
