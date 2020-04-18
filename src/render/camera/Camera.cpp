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

#include "Camera.h"
#include <cmath>
#include <iostream>

namespace visualization {

Camera::Camera(Camera::KeyMap keyMap) : keyMap(keyMap) {
  update();
}

void Camera::update() {
  glm::vec3 new_front;
  new_front.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
  new_front.y = std::sin(glm::radians(pitch));
  new_front.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));

  // Remove magnitude
  front = glm::normalize(new_front);

  right = glm::normalize(glm::cross(front, world_up));
  up = glm::normalize(glm::cross(right, front));
}

void Camera::handle_keypress(int key) {
  if (key == keyMap.W)
    active.front_back = active_directions::direction::forward;
  else if (key == keyMap.S)
    active.front_back = active_directions::direction::backwards;
  else if (key == keyMap.A)
    active.left_right = active_directions::direction::backwards;
  else if (key == keyMap.D)
    active.left_right = active_directions::direction::forward;
}

void Camera::handle_keyrelease(int key) {
  if (key == keyMap.W || key == keyMap.S)
    active.front_back = active_directions::direction::none;
  else if (key == keyMap.A || key == keyMap.D)
    active.left_right = active_directions::direction::none;
}

glm::mat4 Camera::view_matrix() const {
  return glm::lookAt(position, position + front, up);
}

void Camera::move(float delta_time) {
  auto velocity = move_speed * delta_time;

  if (active.front_back == active_directions::direction::forward)
    position += front * velocity;
  else if (active.front_back == active_directions::direction::backwards)
    position -= front * velocity;

  if (active.left_right == active_directions::direction::backwards)
    position -= right * velocity;
  else if (active.left_right == active_directions::direction::forward)
    position += right * velocity;
}

void Camera::mouse_move(float delta_x, float delta_y) {
  if (mobility == move_state::frozen)
    return;

  delta_x *= turn_speed;
  delta_y *= turn_speed;

  yaw += delta_x;
  pitch += delta_y;

  if (pitch > 89.0f)
    pitch = 89.0f;
  else if (pitch < -89.0f)
    pitch = -89.0f;

  update();
}

Camera::move_state Camera::getMobility() const {
  return mobility;
}

void Camera::setMobility(Camera::move_state state) {
  mobility = state;
}

glm::vec3 Camera::get_position() const {
  return position;
}

void Camera::setPosition(const glm::vec3 &value) {
  position = value;
  update();
}

void Camera::resetRotation() {
  yaw = -90.0f;
  pitch = 0.0f;
  update();
}

} // namespace visualization
