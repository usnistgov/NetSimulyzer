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
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/matrix_decompose.hpp>

namespace netsimulyzer {
/**
 * 'Trackball' style camera that orbits around `target`
 */
class ArcCamera {
public:
  /**
   * The point the camera is circling
   */
  glm::vec3 target{0.0f};
  /**
   * The position of the camera in orbit
   */
  glm::vec3 position;

  static constexpr float defaultDistance{10.0f};

  /**
   * The distance from the view of the camera
   * to `target`
   */
  float distance{defaultDistance};
  bool mousePressed{false};
  bool zoomIn{false};
  bool zoomOut{false};
  const glm::vec3 world_up{0.0f, 1.0f, 0.0f};
  float mouseTurnSpeed;
  float moveSpeed;
  float moveSpeedSizeScale{1.0f};
  float keyboardTurnSpeed;
  float fieldOfView;
  float zoomSpeed{0.5f}; // TODO: Make configurable

  int keyForward;
  int keyBackward;
  int keyLeft;
  int keyRight;
  int keyTurnLeft;
  int keyTurnRight;
  int keyUp;
  int keyDown;

  explicit ArcCamera(const glm::vec3 &target = {0, 0, -10});

  void mouseMove(float dx, float dy);
  void rotate(float yaw, float pitch);

  void handleKeyPress(int key);
  void handleKeyRelease(int key);
  void move(float deltaTime);
  void reset();
  void wheel(int delta);
  void zoom(float delta);

  [[nodiscard]] glm::mat4 viewMatrix() const;

private:
  bool forwardPressed{false};
  bool backwardPressed{false};
  bool leftPressed{false};
  bool rightPressed{false};
  bool turnLeftPressed{false};
  bool turnRightPressed{false};
  bool upPressed{false};
  bool downPressed{false};

  void updatePosition();
};
} // namespace netsimulyzer
