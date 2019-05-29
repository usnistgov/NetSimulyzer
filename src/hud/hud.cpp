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

#include "hud.h"
#include <osgText/Font>
#include <osgText/Text>

namespace visualization {

bool HudCallback::run(osg::Object *object, osg::Object *data) {
  auto text = dynamic_cast<osgText::Text *>(object);
  auto nodeVisitor = dynamic_cast<osg::NodeVisitor *>(data);
  auto time = nodeVisitor->getFrameStamp()->getSimulationTime();

  text->setText(std::to_string(time) + "ms");

  return traverse(object, data);
}

HudCamera::HudCamera(double xResolution, double yResolution) {
  // This camera will rewrite all current data, regardless of depth
  // so we clear the depth buffer
  setClearMask(GL_DEPTH_BUFFER_BIT);

  // Render this camera after the normal scene
  setRenderOrder(osg::Camera::POST_RENDER);

  // The coordinates for this camera are not changed by anything else
  setReferenceFrame(osg::Camera::ABSOLUTE_RF);

  // We don't want the HUD camera to grab focus from the normal camera
  setAllowEventFocus(false);

  // Project over the whole window
  setProjectionMatrix(osg::Matrix::ortho2D(0, xResolution, 0, yResolution));

  auto currentTimeLabel = new osgText::Text;
  currentTimeLabel->setCharacterSize(12.0f);
  currentTimeLabel->setAxisAlignment(osgText::TextBase::XY_PLANE);
  currentTimeLabel->setPosition({0, 0, 0});

  currentTimeLabel->setUpdateCallback(new HudCallback);
  addChild(currentTimeLabel);
}

} // namespace visualization
