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

#include "Decoration.h"
#include "../../conversion.h"
#include "../../util/undo-events.h"

namespace netsimulyzer {

Decoration::Decoration(const Model &model, const parser::Decoration &ns3Model) : model(model), ns3Model(ns3Model) {
  this->model.setPosition(toRenderCoordinate(ns3Model.position));
  this->model.setRotate(ns3Model.orientation[0], ns3Model.orientation[2], ns3Model.orientation[1]);

  if (ns3Model.height) {
    const auto bounds = model.getBounds();
    auto height = std::abs(bounds.max.y - bounds.min.y);

    this->model.setTargetHeightScale(*ns3Model.height / height);
  }

  this->model.setScale(ns3Model.scale);
}

const Model &Decoration::getModel() const {
  return model;
}

undo::DecorationMoveEvent Decoration::handle(const parser::DecorationMoveEvent &e) {
  undo::DecorationMoveEvent undo;
  undo.position = model.getPosition();
  undo.event = e;

  this->model.setPosition(toRenderCoordinate(e.targetPosition));

  return undo;
}

undo::DecorationOrientationChangeEvent Decoration::handle(const parser::DecorationOrientationChangeEvent &e) {
  undo::DecorationOrientationChangeEvent undo;
  undo.orientation = model.getRotate();
  undo.event = e;

  this->model.setRotate(e.targetOrientation[0], e.targetOrientation[2], e.targetOrientation[1]);

  return undo;
}

void Decoration::handle(const undo::DecorationMoveEvent &e) {
  model.setPosition(e.position);
}

void Decoration::handle(const undo::DecorationOrientationChangeEvent &e) {
  model.setRotate(e.orientation[0], e.orientation[2], e.orientation[1]);
}

} // namespace netsimulyzer
