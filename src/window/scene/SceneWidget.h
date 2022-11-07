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

#include "../../group/area/Area.h"
#include "../../group/building/Building.h"
#include "../../group/decoration/Decoration.h"
#include "../../group/node/Node.h"
#include "../../render/Light.h"
#include "../../render/camera/Camera.h"
#include "../../render/helper/Floor.h"
#include "../../render/mesh/Mesh.h"
#include "../../render/model/Model.h"
#include "../../render/model/ModelCache.h"
#include "../../render/renderer/Renderer.h"
#include "../../render/shader/Shader.h"
#include "../../render/texture/TextureCache.h"
#include "../../settings/SettingsManager.h"
#include "../../util/undo-events.h"
#include "src/group/link/WiredLink.h"
#include "src/render/font/FontManager.h"
#include "src/render/framebuffer/PickingFramebuffer.h"
#include "src/render/helper/CoordinateGrid.h"
#include "src/render/helper/SkyBox.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLFunctions>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QTimer>
#include <deque>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <model.h>
#include <unordered_map>
#include <vector>

namespace netsimulyzer {

class SceneWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT
  enum class PlayMode { Paused, Play };

  QOpenGLFunctions_3_3_Core openGl;
  SettingsManager settings;
  Camera camera{glm::vec3{0.0f, 2.0f, 0.0f}};
  QPoint initialCursorPosition{width() / 2, height() / 2};
  QPoint lastCursorPosition{width() / 2, height() / 2};
  bool mousePressed = false;
  bool isInitialMove = true;
  TextureCache textures;
  ModelCache models{textures};
  FontManager fontManager{textures};
  Renderer renderer{models, textures, fontManager};
  QTimer timer{this};
  QElapsedTimer frameTimer;
  bool renderLabels = settings.get<bool>(SettingsManager::Key::RenderShowLabels).value();
  float labelScale = settings.get<float>(SettingsManager::Key::RenderLabelScale).value();
  bool renderSkybox = settings.get<bool>(SettingsManager::Key::RenderSkybox).value();
  bool renderGrid = settings.get<bool>(SettingsManager::Key::RenderGrid).value();
  bool renderBuildingOutlines = settings.get<bool>(SettingsManager::Key::RenderBuildingOutlines).value();
  SettingsManager::MotionTrailRenderMode renderMotionTrails =
      settings.get<SettingsManager::MotionTrailRenderMode>(SettingsManager::Key::RenderMotionTrails).value();

  std::unique_ptr<PickingFramebuffer> pickingFbo;

  DirectionalLight mainLight;
  std::unique_ptr<SkyBox> skyBox;
  std::unique_ptr<Floor> floor;
  std::unique_ptr<CoordinateGrid> coordinateGrid;
  SettingsManager::BuildingRenderMode buildingRenderMode =
      settings.get<SettingsManager::BuildingRenderMode>(SettingsManager::Key::RenderBuildingMode).value();
  std::unique_ptr<Model> transmissionSphere;

  parser::GlobalConfiguration config;

  /**
   * Amount of time to advance/rewind `simulationTime`
   * per frame.
   */
  parser::nanoseconds timeStep =
      settings.get<parser::nanoseconds>(SettingsManager::Key::PlaybackTimeStepPreference).value();

  parser::nanoseconds simulationTime;

  std::vector<Area> areas;
  std::vector<Building> buildings;
  std::unordered_map<unsigned int, Node> nodes;
  std::unordered_map<unsigned int, Decoration> decorations;
  std::vector<WiredLink> wiredLinks;

  std::optional<unsigned int> selectedNode;

  PlayMode playMode = PlayMode::Paused;
  std::deque<parser::SceneEvent> events;
  std::deque<undo::SceneUndoEvent> undoEvents;

#ifndef NDEBUG
  QOpenGLDebugLogger glLogger{this};
#endif

  void handleEvents();
  void handleUndoEvents();

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;
  void contextMenuEvent(QContextMenuEvent *event) override;

public:
  explicit SceneWidget(QWidget *parent = nullptr, const Qt::WindowFlags &f = Qt::WindowFlags());
  ~SceneWidget() override;
  void setConfiguration(parser::GlobalConfiguration configuration);
  void reset();
  void add(const std::vector<parser::Area> &areaModels, const std::vector<parser::Building> &buildingModels,
           const std::vector<parser::Decoration> &decorationModels, const std::vector<parser::WiredLink> &links,
           const std::vector<parser::Node> &nodeModels);

  /**
   * Load an individual model specified by `modelPath`
   * as a `Decoration` in the center of the scene
   *
   * @param modelPath
   * The absolute path to the model to load
   */
  void previewModel(const std::string &modelPath);

  /**
   * Center the specified Node in the view.
   * If the Node with ID nodeId is not found,
   * then the camera is not changed
   *
   * @param nodeId
   * The ID of the Node to focus on
   */
  void focusNode(uint32_t nodeId);

  /**
   * Retrieves a Node from the scene by ID.
   * If the Node is not found, this method
   * will abort.
   *
   * @param nodeId
   * The ID of the Node to retrieve
   *
   * @return
   * The Node from the scene
   */
  const Node &getNode(unsigned int nodeId);

  void enqueueEvents(const std::vector<parser::SceneEvent> &e);
  void resetCamera();

  /**
   * Gets the user controlled camera
   *
   * @return
   * The camera
   */
  [[nodiscard]] Camera &getCamera();

  /**
   * Update the projection matrix
   * for when the FOV or window size changes
   */
  void updatePerspective();

  void setResourcePath(const QString &value);

  void play();
  void pause();

  /**
   * Set the time increment per frame of the simulation
   * when playing/rewinding
   *
   * @param value
   * The time increment, in milliseconds
   */
  void setTime(parser::nanoseconds value);
  void setTimeStep(parser::nanoseconds value);
  QSize sizeHint() const override;

  /**
   *
   * @param enable
   * If true, then the skybox is rendered,
   * if false, no skybox is rendered
   */
  void setSkyboxRenderState(bool enable);

  /**
   * Sets the building render behavior
   * @param mode
   * A mode from SettingsManager::BuildingRenderMode
   */
  void setBuildingRenderMode(SettingsManager::BuildingRenderMode mode);

  /**
   * Set if building outlines should be rendered or not
   * @param enable
   * True: render outlines
   * False: do not render outlines
   */
  void setBuildingRenderOutlines(bool enable);

  /**
   * Enable or disable showing the coordinate grid
   *
   * @param enable
   * True to show the grid, false to hide
   */
  void setRenderGrid(bool enable);

  /**
   * Change the size of the grid squares
   *
   * @param stepSize
   * The new size of each grid square in ns-3 units
   */
  void changeGridStepSize(int stepSize);

  /**
   * Set the motion trail rendering behavior
   *
   * @param value
   * Enum value from SettingsManager::MotionTrailRenderMode
   */
  void setRenderTrails(SettingsManager::MotionTrailRenderMode value);

  /**
   * Enable or disable showing
   * labels above elements
   *
   * @param enable
   * True to show labels, false to hide
   */
  void setRenderLabels(bool enable);

  /**
   * Change the scale of the text labels
   * show above scene elements (like Nodes)
   *
   * @param value
   * The new scale, should be > 0.0f
   */
  void setLabelScale(float value);

  void setSelectedNode(unsigned int nodeId);
  void clearSelectedNode();

signals:
  void timeChanged(parser::nanoseconds simulationTime, parser::nanoseconds increment);
  void paused();
  void playing();
  void selectedItemUpdated();
  void nodeSelected(unsigned int nodeId);
};
} // namespace netsimulyzer
