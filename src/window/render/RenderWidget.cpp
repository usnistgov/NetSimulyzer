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

#include "RenderWidget.h"
#include "../../render/camera/Camera.h"
#include "../../render/mesh/Mesh.h"
#include "../../render/mesh/Vertex.h"
#include <QFile>
#include <QKeyEvent>
#include <QObject>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLFunctions_4_5_Core>
#include <QTextStream>
#include <Qt>
#include <QtGui/QOpenGLFunctions>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <model.h>
#include <qopengl.h>
#include <vector>

namespace visualization {

void RenderWidget::handleEvents() {
  // Returns true after handling an event
  // false otherwise
  auto handleEvent = [this](auto &&arg) -> bool {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(arg)>;

    // All events have a time
    // Make sure we don't handle one in the future
    if (arg.time > simulationTime)
      return false;

    if constexpr (std::is_same_v<T, parser::MoveEvent> || std::is_same_v<T, parser::NodeOrientationChangeEvent>) {
      auto node = nodes.find(arg.nodeId);
      if (node == nodes.end())
        return false;
      node->second.handle(arg);
      return true;
    } else if constexpr (std::is_same_v<T, parser::DecorationMoveEvent> ||
                         std::is_same_v<T, parser::DecorationOrientationChangeEvent>) {
      auto decoration = decorations.find(arg.decorationId);
      if (decoration == decorations.end())
        return false;
      decoration->second.handle(arg);
      return true;
    }
  };

  while (!events.empty() && std::visit(handleEvent, events.front())) {
    events.pop_front();
  }
}

glm::mat4 RenderWidget::makePerspective() const {
  return glm::perspective(glm::radians(45.0f), static_cast<float>(width()) / static_cast<float>(height()), 0.1f,
                          1000.0f);
}

void RenderWidget::initializeGL() {
  if (!initializeOpenGLFunctions()) {
    std::cerr << "Failed OpenGL functions\n";
    std::abort();
  }
  std::cout << glGetString(GL_VERSION) << '\n';

  if (!textures.init()) {
    std::cerr << "Failed Initializing Texture Cache\n";
    std::abort();
  }
  QImage fallback{":/texture/resources/textures/plain.png"};
  textures.loadFallback(fallback);
  models.init("resources/models/fallback.obj");
  renderer.init();

  std::array<QImage, 6> skyBoxTextures{
      QImage{":/texture/resources/textures/skybox/right.png"}, QImage{":/texture/resources/textures/skybox/left.png"},
      QImage{":/texture/resources/textures/skybox/top.png"},   QImage{":/texture/resources/textures/skybox/bottom.png"},
      QImage{":/texture/resources/textures/skybox/back.png"},  QImage{":/texture/resources/textures/skybox/front.png"}};
  skyBox = std::make_unique<SkyBox>(textures.loadSkyBox(skyBoxTextures));

  floor = std::make_unique<Floor>(renderer.allocateFloor(100.0f, textures.load("resources/textures/grass.png")));
  floor->setPosition({0.0f, -0.5f, 0.0f});

  auto s = size();
  glViewport(0, 0, s.width(), s.height());

  mainLight.set_ambient_intensity(0.5f);
  mainLight.set_color({1.0f, 1.0f, 1.0f});
  mainLight.set_direction({2.0f, -1.0f, -2.0f});
  mainLight.set_diffuse_intensity(0.5f);
  renderer.render(mainLight);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  renderer.setPerspective(makePerspective());

  // Cheap hack to get Qt to repaint at a reasonable rate
  // Seems to only work with the old connect syntax
  QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
  timer.start(1000 / 60); // roughly 60 times per second

  frameTimer.start();
}

void RenderWidget::paintGL() {
  if (!paused)
    handleEvents();

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); // NOLINT(hicpp-signed-bitwise)
  camera.move(static_cast<float>(frameTimer.elapsed()));
  renderer.use(camera);
  renderer.render(*skyBox);

  for (auto &[key, node] : nodes) {
    renderer.render(node.getModel());
  }

  for (auto &[key, decoration] : decorations) {
    renderer.render(decoration.getModel());
  }
  renderer.render(*floor);

  // Keep this last
  renderer.render(buildings);
  frameTimer.restart();
  if (!paused) {
    simulationTime += config.millisecondsPerFrame;
    emit timeAdvanced(simulationTime);
  }
}

void RenderWidget::resizeGL(int w, int h) {
  renderer.setPerspective(makePerspective());
  glViewport(0, 0, w, h);
}

void RenderWidget::keyPressEvent(QKeyEvent *event) {
  QWidget::keyPressEvent(event);
  camera.handle_keypress(event->key());

  if (event->key() == pauseKey)
    paused = !paused;
}

void RenderWidget::keyReleaseEvent(QKeyEvent *event) {
  QWidget::keyReleaseEvent(event);
  camera.handle_keyrelease(event->key());
}

void RenderWidget::mousePressEvent(QMouseEvent *event) {
  QWidget::mousePressEvent(event);
  if (event->buttons() & Qt::LeftButton) {
    setCursor(Qt::BlankCursor);

    // Keep this position since we're about to move it
    initialCursorPosition = {event->x(), event->y()};

    isInitialMove = true;
    QCursor::setPos(mapToGlobal({width() / 2, height() / 2}));
    camera.setMobility(Camera::move_state::mobile);
  }
}

void RenderWidget::mouseReleaseEvent(QMouseEvent *event) {
  QWidget::mouseReleaseEvent(event);
  if (!(event->buttons() & Qt::LeftButton)) {
    unsetCursor();
    camera.setMobility(Camera::move_state::frozen);

    // Put the cursor back where it was when we started
    QCursor::setPos(mapToGlobal(initialCursorPosition));
    isInitialMove = true;
  }
}

void RenderWidget::mouseMoveEvent(QMouseEvent *event) {
  QWidget::mouseMoveEvent(event);
  if (!(event->buttons() & Qt::LeftButton))
    return;

  const QPoint widgetCenter{width() / 2, height() / 2};

  if (isInitialMove) {
    isInitialMove = false;
    lastCursorPosition = widgetCenter;
    return;
  }

  auto dx = event->x() - lastCursorPosition.x();
  // Prevent inverting the camera
  auto dy = lastCursorPosition.y() - event->y();

  camera.mouse_move(dx, dy);

  lastCursorPosition = widgetCenter;
  QCursor::setPos(mapToGlobal(widgetCenter));
}

RenderWidget::RenderWidget(QWidget *parent, const Qt::WindowFlags &f) : QOpenGLWidget(parent, f) {
  // Make sure we get keyboard events
  setFocusPolicy(Qt::StrongFocus);

  setMinimumSize({640, 480});
}

void RenderWidget::setConfiguration(parser::GlobalConfiguration configuration) {
  config = configuration;

  // Resize ground plane to the farthest away item/event
  auto newSize = std::max({std::abs(config.minLocation.x), std::abs(config.maxLocation.x),
                           std::abs(config.minLocation.y), std::abs(config.maxLocation.y)});

  // Don't resize beneath the default
  if (newSize > 100.0f)
    renderer.resize(*floor, newSize + 50.0f); // Give the new size a bit of extra overrun
}

void RenderWidget::reset() {
  buildings.clear();
  nodes.clear();
  decorations.clear();
  events.clear();
}

void RenderWidget::add(const std::vector<parser::Building> &buildingModels,
                       const std::vector<parser::Decoration> &decorationModels,
                       const std::vector<parser::Node> &nodeModels) {

  // We need a current context for the initial construction of most models
  makeCurrent();
  buildings.reserve(buildingModels.size());
  for (const auto &building : buildingModels) {
    buildings.emplace_back(renderer.allocate(building), building);
  }

  decorations.reserve(decorationModels.size());
  for (const auto &decoration : decorationModels) {
    decorations.try_emplace(decoration.id, Model{models.load("resources/models/" + decoration.model)}, decoration);
  }

  nodes.reserve(nodeModels.size());
  for (const auto &node : nodeModels) {
    nodes.try_emplace(node.id, Model{models.load("resources/models/" + node.model)}, node);
  }

  doneCurrent();
}

void RenderWidget::focusNode(uint32_t nodeId) {
  auto iter = nodes.find(nodeId);
  if (iter == nodes.end()) {
    std::cerr << "Error: Node with ID: " << nodeId << " not found\n";
    return;
  }

  auto position = iter->second.getModel().getPosition();
  position.z += 5.0f;
  camera.setPosition(position);
  camera.resetRotation();
}

void RenderWidget::enqueueEvents(const std::vector<parser::SceneEvent> &e) {
  events.insert(events.end(), e.begin(), e.end());
}

} // namespace visualization
