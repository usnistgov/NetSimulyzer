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

#include "../../group/building/Building.h"
#include "../../group/decoration/Decoration.h"
#include "../../group/node/Node.h"
#include "../../render/camera/Camera.h"
#include "../../render/helper/Floor.h"
#include "../../render/light/DirectionalLight.h"
#include "../../render/mesh/Mesh.h"
#include "../../render/model/Model.h"
#include "../../render/model/ModelCache.h"
#include "../../render/renderer/Renderer.h"
#include "../../render/shader/Shader.h"
#include "../../render/texture/TextureCache.h"
#include <QApplication>
#include <QElapsedTimer>
#include <QFile>
#include <QKeyEvent>
#include <QMainWindow>
#include <QMouseEvent>
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

namespace visualization {

class RenderWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core {
  Q_OBJECT
  Camera camera{{Qt::Key::Key_W, Qt::Key::Key_A, Qt::Key::Key_S, Qt::Key::Key_D}};
  QPoint initialCursorPosition{width() / 2, height() / 2};
  QPoint lastCursorPosition{width() / 2, height() / 2};
  bool isInitialMove = true;
  TextureCache textures;
  ModelCache models{textures};
  Renderer renderer{models};
  QTimer timer{this};
  QElapsedTimer frameTimer;
  Qt::Key pauseKey = Qt::Key::Key_P;

  directional_light mainLight;

  std::unique_ptr<Floor> floor;

  parser::GlobalConfiguration config;
  double simulationTime = 0.0;

  std::vector<Building> buildings;
  std::unordered_map<unsigned int, Node> nodes;
  std::unordered_map<unsigned int, Decoration> decorations;

  bool paused = true;
  std::deque<parser::SceneEvent> events;

  void handleEvents();
  glm::mat4 makePerspective() const;

protected:
  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int w, int h) override;
  void keyPressEvent(QKeyEvent *event) override;
  void keyReleaseEvent(QKeyEvent *event) override;
  void mousePressEvent(QMouseEvent *event) override;
  void mouseReleaseEvent(QMouseEvent *event) override;
  void mouseMoveEvent(QMouseEvent *event) override;

public:
  explicit RenderWidget(QWidget *parent = nullptr, const Qt::WindowFlags &f = Qt::WindowFlags());
  void setConfiguration(parser::GlobalConfiguration configuration);
  void reset();
  void add(const std::vector<parser::Building> &buildingModels, const std::vector<parser::Decoration> &decorationModels,
           const std::vector<parser::Node> &nodeModels);
  /**
   * Center the specified Node in the view.
   * If the Node with ID nodeId is not found,
   * then the camera is not changed
   *
   * @param nodeId
   * The ID of the Node to focus on
   */
  void focusNode(uint32_t nodeId);

  void enqueueEvents(const std::vector<parser::SceneEvent> &e);

signals:
  void timeAdvanced(double simulationTime);
};
} // namespace visualization
