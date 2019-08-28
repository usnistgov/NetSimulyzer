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

#include <osg/Callback>
#include <osg/Camera>
#include <osg/ref_ptr>
#include <osgGA/GUIEventHandler>

namespace visualization {

/**
 * Callback to update the current simulation timestamp
 */
class HudCallback : public osg::Callback {
public:
  bool run(osg::Object *object, osg::Object *data) override;
};

/**
 * A camera with an osg::Text element in the bottom left of the screen
 * and an attached callback to update the text
 */
class HudCamera : public osg::Camera {
public:
  /**
   * Make a HUD camera showing the current timestamp
   * in the bottom left of the screen
   *
   * \param xResolution
   * The width of the current window in pixels
   *
   * \param yResolution
   * The height of the current window in pixels
   */
  HudCamera(double xResolution, double yResolution);
};

/**
 * Window event handler to update the HUD projection matrix
 */
class HudResizeHandler : public osgGA::GUIEventHandler {
  osg::ref_ptr<HudCamera> camera;

public:
  /**
   * @param camera
   * Pointer to the HUD camera to manage
   */
  explicit HudResizeHandler(const osg::ref_ptr<HudCamera> &camera);

  /**
   * @param event
   * GUI Event Adapter holding the new window size
   *
   * @param object
   * Unused
   *
   * @param nv
   * Unused
   *
   * @return true if the event wash handled, false otherwise
   */
  bool handle(osgGA::Event *event, osg::Object *object, osg::NodeVisitor *nv) override;
};

} // namespace visualization
