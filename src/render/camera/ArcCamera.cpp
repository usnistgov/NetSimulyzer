#include "ArcCamera.h"
#include "src/settings/SettingsManager.h"

namespace netsimulyzer {

ArcCamera::ArcCamera(const glm::vec3 &target) : target{target}, position{target + defaultDistance} {
  SettingsManager settings;
  using Key = SettingsManager::Key;
  keyForward = *settings.get<int>(Key::CameraKeyForward);
  keyBackward = *settings.get<int>(Key::CameraKeyBackwards);
  keyLeft = *settings.get<int>(Key::CameraKeyLeft);
  keyRight = *settings.get<int>(Key::CameraKeyRight);
  keyTurnLeft = *settings.get<int>(Key::CameraKeyLeftTurn);
  keyTurnRight = *settings.get<int>(Key::CameraKeyRightTurn);
  keyUp = *settings.get<int>(Key::CameraKeyUp);
  keyDown = *settings.get<int>(Key::CameraKeyDown);

  moveSpeed = *settings.get<float>(Key::MoveSpeed);
  keyboardTurnSpeed = *settings.get<float>(Key::KeyboardTurnSpeed);
  fieldOfView = *settings.get<float>(Key::FieldOfView);
  mouseTurnSpeed = *settings.get<float>(Key::MouseTurnSpeed);
}

void ArcCamera::mouseMove(float dx, float dy) {
  if (!mousePressed)
    return;
  rotate(-dx * mouseTurnSpeed, dy * (mouseTurnSpeed / 4.0f));
}

void ArcCamera::rotate(const float yaw, const float pitch) {
  const auto cameraDirection = glm::normalize(position - target);

  const glm::quat quaternionYaw{glm::angleAxis(glm::radians(yaw), world_up)};
  const glm::quat quaternionPitch{glm::angleAxis(glm::radians(pitch), glm::cross(world_up, cameraDirection))};
  const glm::vec4 rotatedDirection =
      quaternionYaw * quaternionPitch * glm::vec4{cameraDirection.x, cameraDirection.y, cameraDirection.z, 1.0f};

  position = target + glm::vec3(rotatedDirection) * distance;
}

void ArcCamera::handleKeyPress(const int key) {

  if (key == keyForward) {
    forwardPressed = true;
  } else if (key == keyBackward) {
    backwardPressed = true;
  } else if (key == keyLeft) {
    leftPressed = true;
  } else if (key == keyRight) {
    rightPressed = true;
  } else if (key == keyTurnRight) {
    turnRightPressed = true;
  } else if (key == keyTurnLeft) {
    turnLeftPressed = true;
  } else if (key == keyUp) {
    upPressed = true;
  } else if (key == keyDown) {
    downPressed = true;
  }

  // Keyboard zoom
  if (key == Qt::Key::Key_Plus || key == Qt::Key::Key_Equal)
    zoomIn = true;
  else if (key == Qt::Key::Key_Minus)
    zoomOut = true;
}

void ArcCamera::handleKeyRelease(const int key) {
  if (key == keyForward) {
    forwardPressed = false;
  } else if (key == keyBackward) {
    backwardPressed = false;
  } else if (key == keyLeft) {
    leftPressed = false;
  } else if (key == keyRight) {
    rightPressed = false;
  } else if (key == keyTurnRight) {
    turnRightPressed = false;
  } else if (key == keyTurnLeft) {
    turnLeftPressed = false;
  } else if (key == keyUp) {
    upPressed = false;
  } else if (key == keyDown) {
    downPressed = false;
  }

  // Keyboard zoom
  if (key == Qt::Key::Key_Plus || key == Qt::Key::Key_Equal)
    zoomIn = false;
  else if (key == Qt::Key::Key_Minus)
    zoomOut = false;
}

void ArcCamera::move(const float deltaTime) {
  const auto velocity = moveSpeed * moveSpeedSizeScale * deltaTime;
  const auto cameraDirection = glm::normalize(position - target);
  const auto right = glm::normalize(glm::cross(cameraDirection, world_up));
  const auto up = glm::normalize(glm::cross(right, cameraDirection));

  if (forwardPressed) {
    target -= cameraDirection * velocity;
  } else if (backwardPressed) {
    target += cameraDirection * velocity;
  }

  if (rightPressed) {
    target -= right * velocity;
  } else if (leftPressed) {
    target += right * velocity;
  }

  if (upPressed) {
    target += up * velocity;
  } else if (downPressed) {
    target -= up * velocity;
  }

  if (zoomIn)
    zoom(-zoomSpeed);
  else if (zoomOut)
    zoom(zoomSpeed);

  position = target + distance * cameraDirection;

  // Rotate overwrites position
  if (turnRightPressed) {
    rotate(velocity, 0.0f);
  } else if (turnLeftPressed) {
    rotate(-velocity, 0.0f);
  }
}

void ArcCamera::reset() {
  target = glm::vec3{0.0f};
  distance = defaultDistance;
  position = target + distance;
}

void ArcCamera::wheel(const int delta) {
  if (delta == 0)
    return;

  if (delta > 0)
    zoom(zoomSpeed);
  else
    zoom(-zoomSpeed);
}

void ArcCamera::zoom(const float delta) {
  distance += delta;
  updatePosition();
}

glm::mat4 ArcCamera::viewMatrix() const {
  return glm::lookAt(position, target, world_up);
}

void ArcCamera::updatePosition() {
  const auto cameraDirection = glm::normalize(position - target);
  position = target + distance * cameraDirection;
}

} // namespace netsimulyzer
