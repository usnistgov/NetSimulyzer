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
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <utility>

namespace visualization {

Model::Model(const Model::ModelLoadInfo &info) : Model(info.id, info.min, info.max) {
}

Model::Model(model_id modelId, const glm::vec3 &min, const glm::vec3 &max) : modelId(modelId), min(min), max(max) {
}

void Model::rebuildModelMatrix() {
  modelMatrix = glm::mat4{1.0f};
  modelMatrix = glm::translate(modelMatrix, position);

  glm::mat4 rotateMatrix{1.0f};
  rotateMatrix = glm::rotate(rotateMatrix, glm::radians(rotate[0]), {1, 0, 0});
  rotateMatrix = glm::rotate(rotateMatrix, glm::radians(rotate[1]), {0, 1, 0});
  rotateMatrix = glm::rotate(rotateMatrix, glm::radians(rotate[2]), {0, 0, 1});
  modelMatrix *= rotateMatrix;

  modelMatrix = glm::scale(modelMatrix, {targetHeightScale, targetHeightScale, targetHeightScale});
  modelMatrix = glm::scale(modelMatrix, {scale, scale, scale});
}

void Model::setPosition(const glm::vec3 &value) {
  position = value;
  rebuildModelMatrix();
}

void Model::setTargetHeightScale(float value) {
  targetHeightScale = value;
  rebuildModelMatrix();
}

float Model::getTargetHeightScale() const {
  return targetHeightScale;
}

void Model::setScale(float value) {
  scale = value;
  rebuildModelMatrix();
}

const glm::mat4 &Model::getModelMatrix() const {
  return modelMatrix;
}

const glm::vec3 &Model::getPosition() const {
  return position;
}
float Model::getScale() const {
  return scale;
}

void Model::setRotate(float x, float y, float z) {
  rotate[0] = x;
  rotate[1] = y;
  rotate[2] = z;
  rebuildModelMatrix();
}

model_id Model::getModelId() const {
  return modelId;
}

Model::ModelBounds Model::getBounds() const {
  return {min, max};
}

} // namespace visualization
