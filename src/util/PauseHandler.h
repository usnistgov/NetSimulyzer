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

#include <atomic>
#include <osgGA/GUIEventHandler>

namespace visualization {

/**
 * Handler that listens for a key to pause/play the simulation
 */
class PauseHandler : public osgGA::GUIEventHandler {

  /**
   * Character key to toggle the 'paused' flag
   * This character should require no modifiers (ctrl, shift, etc)
   */
  char pauseKey;

  /**
   * Flag indicating the scenario should play or not
   */
  std::atomic_bool paused = true;

public:
  /**
   * Construct a PauseHandler with the pause/play key set
   *
   * @param PauseKey
   * The character to listen for from the keyboard
   */
  explicit PauseHandler(char PauseKey = 'p');

  /**
   *
   * @param event
   * GUI Event Adapter holding the pressed key
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

  /**
   * @return The flag indicating if the scenario should be paused
   */
  bool isPaused() const;
};

} // namespace visualization
