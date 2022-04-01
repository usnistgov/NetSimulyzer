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

#include "../../render/model/Model.h"
#include "../../util/undo-events.h"
#include "src/group/link/WiredLink.h"
#include "src/group/node/TrailBuffer.h"
#include <QOpenGLFunctions_3_3_Core>
#include <glm/glm.hpp>
#include <model.h>
#include <optional>
#include <vector>

namespace netsimulyzer {

class Node {
public:
  struct TransmitInfo {
    bool isTransmitting{false};
    parser::nanoseconds startTime;
    double targetSize{2.0};
    parser::nanoseconds duration;
    glm::vec3 color;
  };

private:
  Model model;
  parser::Node ns3Node;
  glm::vec3 offset;
  TrailBuffer trailBuffer;
  glm::vec3 trailColor;
  std::vector<WiredLink *> wiredLinks;
  TransmitInfo transmitInfo;

public:
  Node(const Model &model, parser::Node ns3Node, TrailBuffer &&trailBuffer);
  [[nodiscard]] const Model &getModel() const;
  [[nodiscard]] const parser::Node &getNs3Model() const;
  [[nodiscard]] bool visible() const;
  [[nodiscard]] glm::vec3 getCenter() const;
  [[nodiscard]] const TransmitInfo &getTransmitInfo() const;
  [[nodiscard]] const TrailBuffer &getTrailBuffer() const;
  [[nodiscard]] const glm::vec3 &getTrailColor() const;

  void addWiredLink(WiredLink *link);

  undo::MoveEvent handle(const parser::MoveEvent &e);
  undo::TransmitEvent handle(const parser::TransmitEvent &e);
  undo::TransmitEndEvent handle(const parser::TransmitEndEvent &e);
  undo::NodeOrientationChangeEvent handle(const parser::NodeOrientationChangeEvent &e);
  undo::NodeColorChangeEvent handle(const parser::NodeColorChangeEvent &e);

  void handle(const undo::MoveEvent &e);
  void handle(const undo::TransmitEvent &e);
  void handle(const undo::TransmitEndEvent &e);
  void handle(const undo::NodeOrientationChangeEvent &e);
  void handle(const undo::NodeColorChangeEvent &e);
};

} // namespace netsimulyzer
