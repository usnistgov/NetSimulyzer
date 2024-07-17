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

#include "src/render/model/Model.h"
#include "src/util/undo-events.h"
#include <model.h>

namespace netsimulyzer {
class Renderer;

class LogicalLink {
  parser::LogicalLink model;
  Model::ModelLoadInfo linkCylinder;
  /// Cached model max.x - min.x
  float modelWidth;
  /// Cached model max.y - min.y
  float modelHeight;
  float diameter{0.75f}; // TODO: Make configurable
  glm::quat rotate{};
  glm::vec3 color;
  glm::mat4 modelMatrix{1.0f};
  std::pair<glm::vec3, glm::vec3> oldPositions{};

public:
  explicit LogicalLink(parser::LogicalLink model, const Model::ModelLoadInfo &linkCylinder);

  [[nodiscard]] const parser::LogicalLink &getModel() const;

  void update(glm::vec3 node1Position, glm::vec3 node2Position);

  undo::LogicalLinkUpdate handle(const parser::LogicalLinkUpdate &e);

  void handle(const undo::LogicalLinkUpdate &e);

  friend Renderer;
};

} // namespace netsimulyzer
