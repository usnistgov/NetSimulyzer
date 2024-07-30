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

#include "SceneWidget.h"
#include "../../render/camera/Camera.h"
#include "../../render/mesh/Mesh.h"
#include "../../render/mesh/Vertex.h"
#include "src/conversion.h"
#include "src/util/palette.h"
#include <QByteArray>
#include <QFileDialog>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QObject>
#include <QOpenGLDebugMessage>
#include <QOpenGLFunctions_3_3_Core>
#include <QPixmap>
#include <QSettings>
#include <QTextStream>
#include <Qt>
#include <QtGui/QOpenGLFunctions>
#include <algorithm>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <ios>
#include <iostream>
#include <model.h>
#include <qopengl.h>
#include <vector>

#ifndef NDEBUG
static void logGlDebugMessage(const QOpenGLDebugMessage &message) {
  std::clog << message.message().toStdString() << '\n';
}
#endif

namespace netsimulyzer {

void SceneWidget::handleEvents() {
  // Flag to indicate the selected Node has been updated
  // Use a flag instead of emitting a signal from the
  // handler, just in case the Node is updated several times
  // this event period
  bool selectedNodeUpdated = false;

  // Returns true after handling an event
  // false otherwise
  auto handleEvent = [this, &selectedNodeUpdated](auto &&arg) -> bool {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(arg)>;

    // All events have a time
    // Make sure we don't handle one in the future
    if (arg.time > simulationTime)
      return false;

    if constexpr (std::is_same_v<T, parser::MoveEvent> || std::is_same_v<T, parser::NodeModelChangeEvent> ||
                  std::is_same_v<T, parser::NodeOrientationChangeEvent> ||
                  std::is_same_v<T, parser::NodeColorChangeEvent> || std::is_same_v<T, parser::TransmitEvent> ||
                  std::is_same_v<T, parser::TransmitEndEvent>) {
      auto node = nodes.find(arg.nodeId);
      if (node == nodes.end())
        return false;

      if constexpr (std::is_same_v<T, parser::NodeModelChangeEvent>)
        undoEvents.emplace_back(node->second.handle(arg, models));
      else
        undoEvents.emplace_back(node->second.handle(arg));

      if (selectedNode.has_value() && node->second.getNs3Model().id == selectedNode.value())
        selectedNodeUpdated = true;

      return true;
    } else if constexpr (std::is_same_v<T, parser::DecorationMoveEvent> ||
                         std::is_same_v<T, parser::DecorationOrientationChangeEvent>) {
      auto decoration = decorations.find(arg.decorationId);
      if (decoration == decorations.end())
        return false;
      undoEvents.emplace_back(decoration->second.handle(arg));
      return true;
    } else if constexpr (std::is_same_v<T, parser::LogicalLinkCreate>) {
      logicalLinks.insert_or_assign(arg.model.id, LogicalLink{arg.model, linkCylinderInfo});
      undoEvents.emplace_back(undo::LogicalLinkCreate{arg});
      return true;
    } else if constexpr (std::is_same_v<T, parser::LogicalLinkUpdate>) {
      auto link = logicalLinks.find(arg.id);
      if (link == logicalLinks.end()) {
        std::cerr << "Logical link update event references Logical Link which does not exist: ID [" << arg.id
                  << "] discarding event\n";
        return true;
      }

      undoEvents.emplace_back(link->second.handle(arg));
      return true;
    }

    return false;
  };

  while (!events.empty() && std::visit(handleEvent, events.front())) {
    events.pop_front();
  }

  if (selectedNodeUpdated)
    emit selectedItemUpdated();
}

void SceneWidget::handleUndoEvents() {
  // Flag to indicate the selected Node has been updated
  // Use a flag instead of emitting a signal from the
  // handler, just in case the Node is updated several times
  // this event period
  bool selectedNodeUpdated = false;

  auto handleUndoEvent = [this, &selectedNodeUpdated](auto &&arg) -> bool {
    // Strip off qualifiers, etc
    // so T holds just the type
    // so we can more easily match it
    using T = std::decay_t<decltype(arg)>;

    // All events have a time
    // Make sure we don't handle one
    // Before it was originally applied
    if (simulationTime > arg.event.time)
      return false;

    if constexpr (std::is_same_v<T, undo::MoveEvent> || std::is_same_v<T, undo::NodeModelChangeEvent> ||
                  std::is_same_v<T, undo::NodeOrientationChangeEvent> || std::is_same_v<T, undo::TransmitEvent> ||
                  std::is_same_v<T, undo::TransmitEndEvent> || std::is_same_v<T, undo::NodeColorChangeEvent>) {
      auto node = nodes.find(arg.event.nodeId);
      if (node == nodes.end())
        return false;

      if constexpr (std::is_same_v<T, undo::NodeModelChangeEvent>)
        node->second.handle(arg, models);
      else
        node->second.handle(arg);

      if (selectedNode.has_value() && node->second.getNs3Model().id == selectedNode.value())
        selectedNodeUpdated = true;

      events.emplace_front(arg.event);
      return true;
    }

    if constexpr (std::is_same_v<T, undo::DecorationMoveEvent> ||
                  std::is_same_v<T, undo::DecorationOrientationChangeEvent>) {
      auto decoration = decorations.find(arg.event.decorationId);
      if (decoration == decorations.end())
        return false;
      decoration->second.handle(arg);

      events.emplace_front(arg.event);
      return true;
    }

    if constexpr (std::is_same_v<T, undo::LogicalLinkCreate>) {
      logicalLinks.erase(arg.event.model.id);
      events.emplace_front(arg.event);
      return true;
    }
    if constexpr (std::is_same_v<T, undo::LogicalLinkUpdate>) {
      auto link = logicalLinks.find(arg.event.id);
      if (link == logicalLinks.end()) {
        std::cerr << "Logical link update undo event references Logical Link which does not exist: ID [" << arg.event.id
                  << "] discarding event\n";
        return true;
      }

      link->second.handle(arg);
      events.emplace_front(arg.event);
      return true;
    }

    return false;
  };

  while (!undoEvents.empty() && std::visit(handleUndoEvent, undoEvents.back())) {
    undoEvents.pop_back();
  }

  if (selectedNodeUpdated)
    emit selectedItemUpdated();
}

void SceneWidget::initializeGL() {
  if (!initializeOpenGLFunctions()) {
    std::cerr << "Failed OpenGL functions\n";
    std::abort();
  }

  if (!openGl.initializeOpenGLFunctions())
    std::cerr << "Failed to initialize passable OpenGL functions!\n";
  std::cout << glGetString(GL_VERSION) << ' ' << openGl.glGetString(GL_VERSION) << '\n';

#ifndef NDEBUG
  const auto hasKhrDebug = context()->hasExtension(QByteArrayLiteral("GL_KHR_debug"));
  std::cout << std::boolalpha << "GL_KHR_debug: " << hasKhrDebug << '\n';
  if (hasKhrDebug && glLogger.initialize()) {
    QObject::connect(&glLogger, &QOpenGLDebugLogger::messageLogged, &logGlDebugMessage);
    glLogger.startLogging(QOpenGLDebugLogger::LoggingMode::SynchronousLogging);
  } else
    std::clog << "Failed to initialize OpenGL debug log\n";
#endif

  if (!textures.init()) {
    std::cerr << "Failed Initializing Texture Cache\n";
    std::abort();
  }

  models.init("models/fallback.obj");
  fontManager.init(":/texture/resources/textures/undefined-medium.png");
  renderer.init();

  transmissionSphere = std::make_unique<Model>(models.load("models/transmission_sphere.obj"));
  linkCylinderInfo = models.load("models/link-cylinder.obj");

  TextureCache::CubeMap cubeMap;
  cubeMap.right = QImage{":/texture/resources/textures/skybox/right.png"};
  cubeMap.left = QImage{":/texture/resources/textures/skybox/left.png"};
  cubeMap.top = QImage{":/texture/resources/textures/skybox/top.png"};
  cubeMap.bottom = QImage{":/texture/resources/textures/skybox/bottom.png"};
  cubeMap.back = QImage{":/texture/resources/textures/skybox/back.png"};
  cubeMap.front = QImage{":/texture/resources/textures/skybox/front.png"};
  skyBox = std::make_unique<SkyBox>(textures.load(cubeMap));

  floor = std::make_unique<Floor>(renderer.allocateFloor(100.0f));
  floor->setPosition({0.0f, -0.5f, 0.0f});

  coordinateGrid = std::make_unique<CoordinateGrid>(
      renderer.allocateCoordinateGrid(100.0f, settings.get<int>(SettingsManager::Key::RenderGridStep).value()));
  coordinateGrid->setHeight(-0.1f);

  auto s = size();
  glViewport(0, 0, s.width(), s.height());

  mainLight.ambientIntensity = 0.9f;
  mainLight.color = {1.0f, 1.0f, 1.0f};
  mainLight.direction = {-2.0f, -1.0f, -2.0f};
  mainLight.diffuseIntensity = 0.5f;
  renderer.render(mainLight);

  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glEnable(GL_DEPTH_TEST);

  updatePerspective();

  // picking FBO
  pickingFbo = std::make_unique<PickingFramebuffer>(openGl, width(), height());
  pickingFbo->unbind(GL_FRAMEBUFFER, defaultFramebufferObject());

  // Cheap hack to get Qt to repaint at a reasonable rate
  // Seems to only work with the old connect syntax
  QObject::connect(&timer, SIGNAL(timeout()), this, SLOT(update()));
  timer.start(1000 / 60); // roughly 60 times per second

  frameTimer.start();
}

void SceneWidget::paintGL() {
  if (playMode == PlayMode::Play) {
    if (timeStep > 0LL)
      handleEvents();
    else
      handleUndoEvents();
  }

  // Picking
  pickingFbo->bind(GL_FRAMEBUFFER);
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  for (auto &[key, node] : nodes) {
    if (!node.visible())
      continue;
    renderer.renderPickingNode(node.getNs3Model().id, node.getModel());
  }

  glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
  // end Picking

  switch (cameraType) {
  case SettingsManager::CameraType::FirstPerson:
    camera.move(static_cast<float>(frameTimer.elapsed()));
    renderer.use(camera);
    break;
  case SettingsManager::CameraType::ArcBall:
    arcCamera.move(static_cast<float>(frameTimer.elapsed()));
    renderer.use(arcCamera);
    break;
  }

  glClearColor(clearColorGl[0], clearColorGl[1], clearColorGl[2], 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  if (renderSkybox)
    renderer.render(*skyBox);

  for (auto &[key, node] : nodes) {
    if (!node.visible())
      continue;
    if (selectedNode.has_value())
      renderer.render(node, key == selectedNode.value());
    else
      renderer.render(node, false);

    using MotionTrailRenderMode = SettingsManager::MotionTrailRenderMode;
    if (renderMotionTrails == MotionTrailRenderMode::Always ||
        (renderMotionTrails == MotionTrailRenderMode::EnabledOnly && node.getNs3Model().trailEnabled))
      renderer.renderTrail(node.getTrailBuffer(), node.getTrailColor());
  }

  for (auto &[_, logicalLink] : logicalLinks) {
    const auto &model = logicalLink.getModel();

    if (!model.active)
      continue;

    const auto &node1It = nodes.find(model.nodes.first);
    if (node1It == nodes.end()) {
      std::cerr << "Node: " << model.nodes.first << " not found when rendering Logical Link: " << model.id
                << " skipping!\n";
      continue;
    }

    const auto &node2It = nodes.find(model.nodes.second);
    if (node2It == nodes.end()) {
      std::cerr << "Node: " << model.nodes.second << " not found when rendering Logical Link: " << model.id
                << " skipping!\n";
      continue;
    }
    const auto &node1 = node1It->second;
    const auto &node2 = node2It->second;

    // TODO: find a way to make a component-wise offset
    // TODO: Cache offset and calculate on location/scale change
    const auto offset = std::max(node1.getModel().getLinkOffset(), node2.getModel().getLinkOffset());

    logicalLink.update(node1It->second.getCenter(), node2It->second.getCenter(), offset);
    renderer.render(logicalLink);
  }

  for (auto &[key, decoration] : decorations) {
    renderer.render(decoration.getModel());
  }

  if (renderFloor)
    renderer.render(*floor);

  renderer.render(areas);

  if (buildingRenderMode == SettingsManager::BuildingRenderMode::Opaque)
    renderer.render(buildings);
  // else in the transparent section

  if (renderBuildingOutlines) {
    // Black outlines for opaque buildings
    // White for transparent
    if (buildingRenderMode == SettingsManager::BuildingRenderMode::Opaque)
      renderer.renderOutlines(buildings, glm::vec3{0.0f, 0.0f, 0.0f});
    else
      renderer.renderOutlines(buildings, glm::vec3{1.0f, 1.0f, 1.0f});
  }

  renderer.render(wiredLinks);

  // Keep this next to `startTransparent()`
  // has it's own transparency implementation
  if (renderGrid)
    renderer.render(*coordinateGrid);
  // Keep this after all opaque items
  renderer.startTransparentDark();

  // Other condition in opaque section
  if (buildingRenderMode == SettingsManager::BuildingRenderMode::Transparent)
    renderer.render(buildings);

  for (const auto &[_, node] : nodes) {
    if (!node.visible())
      continue;

    const auto &nodeModel = node.getModel();
    renderer.renderTransparent(nodeModel);

    // Name Banner
    using LabelRenderMode = SettingsManager::LabelRenderMode;
    if (renderLabels == LabelRenderMode::Always ||
        (renderLabels == LabelRenderMode::EnabledOnly && node.getNs3Model().labelEnabled))
      renderer.renderFont(node.getBannerRenderInfo(), node.getTop(), labelScale);
    // `renderFont` ends with us in light transparent mode,
    // so make sure we're back in dark mode, since other transparent
    // items assume that mode
    renderer.startTransparentDark();

    const auto &transmit = node.getTransmitInfo();
    if (transmit.isTransmitting && transmit.startTime <= simulationTime &&
        transmit.startTime + transmit.duration >= simulationTime) {
      const auto delta = static_cast<double>(simulationTime - transmit.startTime) /
                         static_cast<double>(transmit.duration) * transmit.targetSize;
      transmissionSphere->setPosition(nodeModel.getPosition());
      transmissionSphere->setTargetHeightScale(static_cast<float>(delta));
      transmissionSphere->setBaseColor(transmit.color);
      renderer.render(*transmissionSphere, Renderer::LightingMode::LightingDisabled);
    }
  }

  renderer.startTransparentDark();
  for (auto &[key, decoration] : decorations) {
    renderer.renderTransparent(decoration.getModel());
  }
  renderer.endTransparent();
  frameTimer.restart();

  if (playMode == PlayMode::Paused)
    return;

  simulationTime += timeStep;
  emit timeChanged(simulationTime, timeStep);

  const auto pastEnd = timeStep > 0LL && simulationTime >= config.endTime;
  const auto pastBeginning = timeStep < 0LL && simulationTime < 0LL;
  if ((pastEnd || pastBeginning) && playMode == PlayMode::Play) {
    pause();

    // Correct times so they match up with the end/beginning
    // Useful if the increment does not match up with
    // the end time
    if (pastEnd)
      setTime(config.endTime);
    else
      setTime(0LL);
  }
}

void SceneWidget::resizeGL(int w, int h) {
  updatePerspective();
  glViewport(0, 0, w, h);
  pickingFbo->resize(w, h);
}

void SceneWidget::keyPressEvent(QKeyEvent *event) {
  QWidget::keyPressEvent(event);
  camera.handle_keypress(event->key());
  arcCamera.handleKeyPress(event->key());
}

void SceneWidget::keyReleaseEvent(QKeyEvent *event) {
  QWidget::keyReleaseEvent(event);
  camera.handle_keyrelease(event->key());
  arcCamera.handleKeyRelease(event->key());
}

void SceneWidget::mousePressEvent(QMouseEvent *event) {
  QWidget::mousePressEvent(event);

  makeCurrent();

  // OpenGL starts from the bottom left,
  // Qt Starts at the top left,
  // so adjust the Y coordinate accordingly
  const auto selected = pickingFbo->read(event->x(), height() - event->y());
  pickingFbo->unbind(GL_READ_FRAMEBUFFER, defaultFramebufferObject());
  doneCurrent();

  if (selected.object && selected.type == 1u) {
    emit nodeSelected(selected.id);
    selectedNode = selected.id;
    return;
  }

  if (cameraType == SettingsManager::CameraType::FirstPerson) {
    mousePressed = true;
    camera.setMobility(Camera::move_state::mobile);
  } else /* ArcBall */ {
    if (event->buttons() & Qt::LeftButton) {
      arcCamera.mousePressed = true;
    }
  }

  setCursor(Qt::BlankCursor);
  initialCursorPosition = {event->x(), event->y()};
}

void SceneWidget::mouseReleaseEvent(QMouseEvent *event) {
  QWidget::mouseReleaseEvent(event);

  if (!(event->buttons() & Qt::LeftButton)) {
    if (cameraType == SettingsManager::CameraType::FirstPerson) {
      mousePressed = false;
      camera.setMobility(Camera::move_state::frozen);
    } else /* Arcball */ {
      arcCamera.mousePressed = false;
    }
  }

  if (mousePressed || arcCamera.mousePressed)
    return;

  unsetCursor();
  // Put the cursor back where it was when we started
  QCursor::setPos(mapToGlobal(initialCursorPosition));
}

void SceneWidget::wheelEvent(QWheelEvent *event) {
  if (cameraType == SettingsManager::CameraType::FirstPerson) {
    QOpenGLWidget::wheelEvent(event);
    return;
  }

  int delta{};
  const auto numPixels = event->pixelDelta();
  const auto numDegrees = event->angleDelta() / 8;

  if (!numPixels.isNull())
    delta = numPixels.y();
  else if (!numDegrees.isNull())
    delta = numDegrees.y();

  arcCamera.wheel(delta);
  QOpenGLWidget::wheelEvent(event);
}

void SceneWidget::mouseMoveEvent(QMouseEvent *event) {
  QWidget::mouseMoveEvent(event);

  if (cameraType == SettingsManager::CameraType::ArcBall && !arcCamera.mousePressed)
    return;

  if (cameraType == SettingsManager::CameraType::FirstPerson && !mousePressed)
    return;
  auto dx = event->x() - initialCursorPosition.x();
  // Prevent inverting the camera
  auto dy = initialCursorPosition.y() - event->y();

  if (cameraType == SettingsManager::CameraType::FirstPerson) {
    camera.mouse_move(dx, dy);
  } else /* ArcBall */ {
    arcCamera.mouseMove(dx, dy);
  }

  QCursor::setPos(mapToGlobal(initialCursorPosition));
}

void SceneWidget::contextMenuEvent(QContextMenuEvent *event) {
  QMenu menu;
  menu.addAction("Save as Image", [this]() {
    const auto image = grab();
    const auto fileName = QFileDialog::getSaveFileName(this, "Save as Image", "", "Images (*.png *.jpeg)");
    if (fileName.isEmpty()) {
      return;
    }
    image.save(fileName);
  });

  menu.exec(event->globalPos());
}

SceneWidget::SceneWidget(QWidget *parent, const Qt::WindowFlags &f) : QOpenGLWidget(parent, f) {
  // Make sure we get keyboard events
  setFocusPolicy(Qt::StrongFocus);

  auto resourceDirSetting = settings.get<QString>(SettingsManager::Key::ResourcePath);

  // Shouldn't happen, but just in case
  if (!resourceDirSetting) {
    QMessageBox::critical(this, "No 'resources/' Directory Set",
                          "No 'resources/' directory was set "
                          "rerun the application and set one");
    std::abort();
  }

  setResourcePath(resourceDirSetting.value());
}

SceneWidget::~SceneWidget() {
#ifndef NDEBUG
  // Silence the warning for trying to close a logger
  // without the right OpenGL context
  makeCurrent();
  glLogger.stopLogging();
  doneCurrent();
#endif
}

void SceneWidget::setConfiguration(parser::GlobalConfiguration configuration) {
  config = configuration;

  // Resize ground plane to the farthest away item/event
  auto newSize = std::max({std::abs(config.minLocation.x), std::abs(config.maxLocation.x),
                           std::abs(config.minLocation.y), std::abs(config.maxLocation.y)});

  // Don't resize beneath the default
  if (newSize > 100.0f) {
    renderer.resize(*floor, newSize + 50.0f); // Give the new size a bit of extra overrun
    renderer.resize(*coordinateGrid, newSize + 50.0f, settings.get<int>(SettingsManager::Key::RenderGridStep).value());
  }

  // time step handled by the MainWindow
}

void SceneWidget::reset() {
  areas.clear();
  buildings.clear();
  nodes.clear();
  decorations.clear();
  wiredLinks.clear();
  logicalLinks.clear();
  events.clear();
  undoEvents.clear();
  selectedNode.reset();
  fontManager.reset();
  simulationTime = 0.0;
}

void SceneWidget::add(const std::vector<parser::Area> &areaModels, const std::vector<parser::Building> &buildingModels,
                      const std::vector<parser::Decoration> &decorationModels,
                      const std::vector<parser::WiredLink> &links,
                      const std::vector<parser::LogicalLink> &parserLogicalLinks,
                      const std::vector<parser::Node> &nodeModels) {

  // We need a current context for the initial construction of most models
  makeCurrent();

  areas.reserve(areaModels.size());
  for (const auto &area : areaModels) {
    areas.emplace_back(renderer.allocate(area), area);
  }

  buildings.reserve(buildingModels.size());
  for (const auto &building : buildingModels) {
    buildings.emplace_back(renderer.allocate(building), building);
  }

  decorations.reserve(decorationModels.size());
  for (const auto &decoration : decorationModels) {
    decorations.try_emplace(decoration.id, Model{models.load(decoration.model)}, decoration);
  }

  nodes.reserve(nodeModels.size());
  const auto functions = context()->versionFunctions<QOpenGLFunctions_3_3_Core>();
  const auto trailLength = settings.get<int>(SettingsManager::Key::RenderMotionTrailLength).value();
  for (const auto &node : nodeModels) {
    nodes.try_emplace(node.id, Model{models.load(node.model)}, node,
                      renderer.allocateTrailBuffer(functions, trailLength), fontManager.allocate(node.name));
  }

  wiredLinks.reserve(links.size());
  for (const auto &link : links) {
    auto &newLink = wiredLinks.emplace_back(renderer.allocate(link), link);

    // Flag to ignore links with non-configured nodes
    // should be picked up by the ns-3 module, but just in case
    bool ignoreLink = false;
    for (const auto nodeId : link.nodes) {
      const auto &node = nodes.find(nodeId);

      if (node == nodes.end()) {
        std::cerr << "A wired link references an unknown Node with ID: " << nodeId << " ignoring link\n";
        ignoreLink = true;
        continue;
      }

      node->second.addWiredLink(&newLink);
    }

    if (ignoreLink)
      wiredLinks.erase(wiredLinks.end() - 1);
  }

  logicalLinks.reserve(parserLogicalLinks.size());
  for (const auto &parsedLink : parserLogicalLinks) {
    if (nodes.find(parsedLink.nodes.first) == nodes.end()) {
      std::cerr << "Node ID: " << parsedLink.nodes.first << " not found, ignoring link: " << parsedLink.id << '\n';
      continue;
    }

    if (nodes.find(parsedLink.nodes.second) == nodes.end()) {
      std::cerr << "Node ID: " << parsedLink.nodes.second << " not found, ignoring link: " << parsedLink.id << '\n';
      continue;
    }

    logicalLinks.insert_or_assign(parsedLink.id, LogicalLink{parsedLink, linkCylinderInfo});
  }

  doneCurrent();
}

void SceneWidget::previewModel(const std::string &modelPath) {
  makeCurrent();

  // Reset the model cache, since it's not out of the question
  // that a model has changed since the last load
  models.reset();

  // The scene should only have our previewed model in it
  // so, remove everything else
  reset();

  const Model previewedModel{models.loadAbsolute(modelPath)};

  // If we get the fallback model ID, then the model failed to load
  // (Unless someone is trying to load the fallback model itself...)
  if (previewedModel.getModelId() == models.getFallbackModelId()) {
    QMessageBox::warning(this, "Failed to Load Model", "Failed to load the model. Check the console for more info");
    models.reset();
    reset();
    doneCurrent();
    return;
  }

  decorations.try_emplace(0u, previewedModel, parser::Decoration{});

  // Put the camera slightly away from the loaded model
  // accounting for how large the model is
  const auto bounds = previewedModel.getBounds();
  auto position = previewedModel.getPosition();
  position.z += bounds.max.z + 5.0f;

  // Put us at the middle of the model (height wise)
  position.y = (bounds.max.y - bounds.min.y) / 2.0f;

  camera.setPosition(position);
  camera.resetRotation();
  doneCurrent();
}

void SceneWidget::focusNode(uint32_t nodeId) {
  auto iter = nodes.find(nodeId);
  if (iter == nodes.end()) {
    std::cerr << "Error: Node with ID: " << nodeId << " not found\n";
    return;
  }

  const auto &node = iter->second;
  const auto &ns3Model = node.getNs3Model();

  const auto &bounds = node.getModel().getBounds();
  auto position = node.getModel().getPosition();

  // Put us slightly away from the model
  position.z += 5.0f;

  // Put us at the middle of the model (height wise)
  // Use the provided height or one calculated based on
  // the model bounds
  position.y += ns3Model.height.value_or(bounds.max.y - bounds.min.y) * ns3Model.scale[2] / 2.0f;

  if (cameraType == SettingsManager::CameraType::FirstPerson) {
    camera.setPosition(position);
    camera.resetRotation();
  } else {
    arcCamera.target = node.getModel().getPosition();
  }
}

const Node &SceneWidget::getNode(unsigned int nodeId) {
  const auto iter = nodes.find(nodeId);

  if (iter == nodes.end()) {
    std::cerr << "Error: Node with ID: " << nodeId << " not found\n";
    std::abort();
  }

  return iter->second;
}

void SceneWidget::enqueueEvents(const std::vector<parser::SceneEvent> &e) {
  events.insert(events.end(), e.begin(), e.end());
}

void SceneWidget::resetCamera() {
  if (cameraType == SettingsManager::CameraType::FirstPerson) {
    camera.setPosition({0.0f, 0.0f, 0.0f});
    camera.resetRotation();
  } else {
    arcCamera.reset();
  }
}

Camera &SceneWidget::getCamera() {
  return camera;
}

ArcCamera &SceneWidget::getArcCamera() {
  return arcCamera;
}

void SceneWidget::updatePerspective() {
  float fov;

  if (cameraType == SettingsManager::CameraType::FirstPerson)
    fov = camera.getFieldOfView();
  else
    fov = arcCamera.fieldOfView;

  renderer.setPerspective(
      glm::perspective(glm::radians(fov), static_cast<float>(width()) / static_cast<float>(height()), 0.1f, 1000.0f));
}

void SceneWidget::setResourcePath(const QString &value) {
  textures.setResourceDirectory(QDir{value});
  models.setBasePath(value.toStdString());
}

void SceneWidget::play() {
  playMode = PlayMode::Play;

  emit playing();
}

void SceneWidget::pause() {
  playMode = PlayMode::Paused;

  emit paused();
}

void SceneWidget::setTime(parser::nanoseconds value) {
  const auto oldTime = simulationTime;

  simulationTime = value;
  const auto diff = simulationTime - oldTime;

  if (diff > 0LL)
    handleEvents();
  else
    handleUndoEvents();

  emit timeChanged(simulationTime, diff);
}

void SceneWidget::setTimeStep(parser::nanoseconds value) {
  timeStep = value;
}

QSize SceneWidget::sizeHint() const {
  return {640, 480};
}

void SceneWidget::setCameraType(const SettingsManager::CameraType value) {
  cameraType = value;
}

void SceneWidget::setSkyboxRenderState(bool enable) {
  renderSkybox = enable;
}

void SceneWidget::setFloorRenderState(bool enable) {
  renderFloor = enable;
}

void SceneWidget::setClearColor(const QColor &value) {
  clearColor = value;

  clearColorGl[0] = static_cast<float>(clearColor.redF());
  clearColorGl[1] = static_cast<float>(clearColor.greenF());
  clearColorGl[2] = static_cast<float>(clearColor.blueF());
}

void SceneWidget::setBuildingRenderMode(SettingsManager::BuildingRenderMode mode) {
  buildingRenderMode = mode;
}

void SceneWidget::setBuildingRenderOutlines(bool enable) {
  renderBuildingOutlines = enable;
}

void SceneWidget::setRenderGrid(bool enable) {
  renderGrid = enable;
}
void SceneWidget::changeGridStepSize(int stepSize) {
  makeCurrent();
  // Keep the same square size, but change the grid step
  renderer.resize(*coordinateGrid, coordinateGrid->getRenderInfo().squareSize, stepSize);
  doneCurrent();
}

void SceneWidget::setRenderTrails(SettingsManager::MotionTrailRenderMode value) {
  renderMotionTrails = value;
}

void SceneWidget::setRenderLabels(SettingsManager::LabelRenderMode value) {
  renderLabels = value;
}

void SceneWidget::setLabelScale(float value) {
  labelScale = value;
}

void SceneWidget::setSelectedNode(unsigned int nodeId) {
  if (nodes.find(nodeId) == nodes.end()) {
    std::cerr << "Node with ID: " << nodeId << " selected, but not found in `nodes`, ignoring!\n";
    return;
  }

  selectedNode = nodeId;
}

void SceneWidget::clearSelectedNode() {
  selectedNode.reset();
}

} // namespace netsimulyzer
