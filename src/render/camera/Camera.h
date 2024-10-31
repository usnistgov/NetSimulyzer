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

#include <Qt>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace netsimulyzer {

class Camera {
public:
  enum class move_state { mobile, frozen };

private:
  struct active_directions {
    enum class direction { none, forward, backwards };
    enum class verticalDirection { none, up, down };
    enum class side { none, left, right };
    direction front_back = direction::none;
    side left_right = side::none;
    verticalDirection upDown = verticalDirection::none;
    side turn = side::none;
  };

  int keyForward;
  int keyBackward;
  int keyLeft;
  int keyRight;
  int keyTurnLeft;
  int keyTurnRight;
  int keyUp;
  int keyDown;

  glm::vec3 position{0.0f};
  glm::vec3 front{0.0f, 0.0f, -1.0f};
  glm::vec3 up{0.0f};
  glm::vec3 right{0.0f};
  glm::vec3 world_up{0.0f, 1.0f, 0.0f};

  float fieldOfView = 45.0f;
  const float minHeight = 0.0f;

  float yaw{-90.0f};
  float pitch{0.0f};

  float moveSpeedSizeScale{1.0f};
  float move_speed{0.05f};
  float turnSpeed{0.1};
  float mouseTurnSpeed{0.5f};

  active_directions active;

  move_state mobility = move_state::frozen;

public:
  Camera(const glm::vec3 &initialPosition = {0.0f, 0.0f, 0.0f});

  void update();
  [[nodiscard]] glm::mat4 view_matrix() const;
  void handle_keypress(int key);
  void handle_keyrelease(int key);

  [[nodiscard]] float getFieldOfView() const;
  void setFieldOfView(float value);

  [[nodiscard]] float getMoveSpeed() const;
  void setMoveSpeed(float value);

  [[nodiscard]] float getTurnSpeed() const;
  void setTurnSpeed(float value);

  [[nodiscard]] float getMouseTurnSpeed() const;
  void setMouseTurnSpeed(float value);

  [[nodiscard]] int getKeyForward() const;
  void setKeyForward(int value);

  [[nodiscard]] int getKeyBackward() const;
  void setKeyBackward(int value);

  [[nodiscard]] int getKeyLeft() const;
  void setKeyLeft(int value);

  [[nodiscard]] int getKeyRight() const;
  void setKeyRight(int value);

  [[nodiscard]] int getKeyTurnLeft() const;
  void setKeyTurnLeft(int value);

  [[nodiscard]] int getKeyTurnRight() const;
  void setKeyTurnRight(int value);

  [[nodiscard]] int getKeyUp() const;
  void setKeyUp(int value);

  [[nodiscard]] int getKeyDown() const;
  void setKeyDown(int value);

  [[nodiscard]] move_state getMobility() const;
  void setMobility(move_state state);

  void move(float delta_time);
  void mouse_move(float delta_x, float delta_y);

  [[nodiscard]] float getMoveSpeedSizeScale() const;
  void setMoveSpeedSizeScale(float value);

  [[nodiscard]] glm::vec3 get_position() const;
  void setPosition(const glm::vec3 &value);

  void resetRotation();
};

} // namespace netsimulyzer
