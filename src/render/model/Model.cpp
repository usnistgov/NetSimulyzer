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

#include "Model.h"
#include "../mesh/Vertex.h"
#include <algorithm>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <iostream>
#include <utility>
#include <glm/gtx/norm.hpp>

namespace netsimulyzer {

void Model::rebuildScaleMatrix() {
  scaleMatrix = glm::mat4{1.0f};

  if (!targetHeightScale && !targetWidthScale && !targetDepthScale) {
    scaleMatrix = glm::scale(scaleMatrix, scale);
    return;
  }

  if (keepRatio) {
    // At least one of these will be set, so we won't end up with 0
    const auto maxTargetScale =
        std::max({targetHeightScale.value_or(0.0f), targetWidthScale.value_or(0.0f), targetDepthScale.value_or(0.0f)});
    scaleMatrix = glm::scale(scaleMatrix, {maxTargetScale, maxTargetScale, maxTargetScale});
  } else {
    scaleMatrix = glm::scale(scaleMatrix, {targetWidthScale.value_or(1.0f), targetHeightScale.value_or(1.0f),
                                           targetDepthScale.value_or(1.0f)});
  }

  scaleMatrix = glm::scale(scaleMatrix, scale);
}

Model::Model(const Model::ModelLoadInfo &info) : Model(info.id, info.min, info.max) {
}

Model::Model(model_id modelId, const glm::vec3 &min, const glm::vec3 &max) : modelId(modelId), min(min), max(max) {
}

glm::quat RotationBetweenVectors(glm::vec3 start, glm::vec3 dest){
  start = glm::normalize(start);
  dest = glm::normalize(dest);

  const float cosTheta = dot(start, dest);
  glm::vec3 rotationAxis;

  if (cosTheta < -1 + 0.001f){
    // special case when vectors in opposite directions:
    // there is no "ideal" rotation axis
    // So guess one; any will do as long as it's perpendicular to start
    rotationAxis = cross(glm::vec3(0.0f, 0.0f, 1.0f), start);
    if (glm::length2(rotationAxis) < 0.01 ) // bad luck, they were parallel, try again!
      rotationAxis = cross(glm::vec3(1.0f, 0.0f, 0.0f), start);

    rotationAxis = normalize(rotationAxis);
    return glm::angleAxis(glm::radians(180.0f), rotationAxis);
  }

  rotationAxis = cross(start, dest);

  const auto s = std::sqrt( (1+cosTheta)*2 );
  const auto invs = 1.0f / s;

  return {
          s * 0.5f,
          rotationAxis.x * invs,
          rotationAxis.y * invs,
          rotationAxis.z * invs
  };

}

void Model::rebuildModelMatrix() {
  modelMatrix = glm::mat4{1.0f};
  modelMatrix = glm::translate(modelMatrix, position);


  if (rotateQuat)
    modelMatrix *= glm::toMat4(rotateQuat.value());
  else {
    glm::mat4 rotateMatrix{1.0f};
    rotateMatrix = glm::rotate(rotateMatrix, glm::radians(rotate[0]), {1, 0, 0});
    rotateMatrix = glm::rotate(rotateMatrix, glm::radians(rotate[1]), {0, 1, 0});
    rotateMatrix = glm::rotate(rotateMatrix, glm::radians(rotate[2]), {0, 0, 1});

    modelMatrix *= rotateMatrix;
  }


  modelMatrix *= scaleMatrix;
}
void Model::face(const glm::vec3 front, const glm::vec3 direction, glm::vec3 up) {
  // Find the rotation between the front of the object (that we assume towards +Z,
  // but this depends on your model) and the desired direction
  const auto rot1 = RotationBetweenVectors(front, direction);

  // Recompute desiredUp so that it's perpendicular to the direction
  // You can skip that part if you really want to force desiredUp
  const auto right = cross(direction, up);
  up = cross(right, direction);

  // Because of the 1rst rotation, the up is probably completely screwed up.
  // Find the rotation between the "up" of the rotated object, and the desired up
  const auto  newUp = rot1 * glm::vec3(0.0f, 1.0f, 0.0f);
  const auto rot2 = RotationBetweenVectors(newUp, up);

  rotateQuat = rot2 * rot1;
}

void Model::setPosition(const glm::vec3 &value) {
  position = value;
  rebuildModelMatrix();
}

void Model::setKeepRatio(bool value) {
  keepRatio = value;
  rebuildScaleMatrix();
  rebuildModelMatrix();
}

bool Model::getKeepRatio() const {
  return keepRatio;
}

void Model::setTargetHeightScale(float value) {
  targetHeightScale = value;
  rebuildScaleMatrix();
  rebuildModelMatrix();
}

std::optional<float> Model::getTargetHeightScale() const {
  return targetHeightScale;
}

void Model::setTargetWidthScale(float value) {
  targetWidthScale = value;
  rebuildScaleMatrix();
  rebuildModelMatrix();
}

std::optional<float> Model::getTargetWidthScale() const {
  return targetWidthScale;
}

void Model::setTargetDepthScale(float value) {
  targetDepthScale = value;
  rebuildScaleMatrix();
  rebuildModelMatrix();
}

std::optional<float> Model::getTargetDepthScale() const {
  return targetDepthScale;
}

void Model::setScale(glm::vec3 value) {
  scale = value;
  rebuildScaleMatrix();
  rebuildModelMatrix();
}

const glm::mat4 &Model::getModelMatrix() const {
  return modelMatrix;
}

const glm::vec3 &Model::getPosition() const {
  return position;
}

glm::vec3 Model::getCenter() const {
  // Set-up of the scale matrix
  // [x 0 0 0]
  // [0 y 0 0]
  // [0 0 z 0]
  // [0 0 0 1]
  const auto xScale = scaleMatrix[0].x;
  const auto yScale = scaleMatrix[1].y;
  const auto zScale = scaleMatrix[2].z;

  glm::vec3 result;
  result.x = (max.x + min.x) * xScale / 2.0f;
  result.y = (max.y + min.y) * yScale / 2.0f;
  result.z = (max.z + min.z) * zScale / 2.0f;

  return result;
}

glm::vec3 Model::getTop() const {
  // Pull the final scale from the
  // built scale matrix
  // [x 0 0 0]
  // [0 y 0 0]
  // [0 0 z 0]
  // [0 0 0 1]
  const auto xScale = scaleMatrix[0].x;
  const auto yScale = scaleMatrix[1].y;
  const auto zScale = scaleMatrix[2].z;

  glm::vec3 result;
  result.x = (max.x + min.x) * xScale / 2.0f;
  result.y = (max.y + min.y) * yScale;
  result.z = (max.z + min.z) * zScale / 2.0f;

  return result;
}
float Model::getLinkOffset() const {
  // Pull the final scale from the
  // built scale matrix
  // [x 0 0 0]
  // [0 y 0 0]
  // [0 0 z 0]
  // [0 0 0 1]
  const auto xScale = scaleMatrix[0].x;
  const auto yScale = scaleMatrix[1].y;
  const auto zScale = scaleMatrix[2].z;


  return std::max({
    std::abs(max.x * xScale),
    std::abs(min.x * xScale),
    std::abs(max.y * yScale),
    std::abs(min.y * yScale),
    std::abs(max.z * zScale),
    std::abs(min.z * zScale),
  });
}

glm::vec3 Model::getScale() const {
  return scale;
}

void Model::setRotate(float x, float y, float z) {
  rotate[0] = x;
  rotate[1] = y;
  rotate[2] = z;
  rebuildModelMatrix();
}

glm::vec3 Model::getRotate() const {
  return rotate;
}

model_id Model::getModelId() const {
  return modelId;
}

Model::ModelBounds Model::getBounds() const {
  return {min, max};
}

void Model::setBaseColor(const glm::vec3 &value) {
  baseColor.emplace(value);
}

void Model::unsetBaseColor() {
  baseColor.reset();
}

const std::optional<glm::vec3> &Model::getBaseColor() const {
  return baseColor;
}

void Model::setHighlightColor(const glm::vec3 &value) {
  highlightColor.emplace(value);
}

void Model::unsetHighlightColor() {
  highlightColor.reset();
}

const std::optional<glm::vec3> &Model::getHighlightColor() const {
  return highlightColor;
}

} // namespace netsimulyzer
