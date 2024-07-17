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
#include "../Light.h"
#include "../camera/Camera.h"
#include "../helper/Floor.h"
#include "../mesh/Mesh.h"
#include "../model/Model.h"
#include "../model/ModelCache.h"
#include "../shader/Shader.h"
#include "../texture/TextureCache.h"
#include "src/group/link/LogicalLink.h"
#include "src/group/link/WiredLink.h"
#include "src/group/node/Node.h"
#include "src/group/node/TrailBuffer.h"
#include "src/render/camera/ArcCamera.h"
#include "src/render/font/FontManager.h"
#include "src/render/font/character.h"
#include "src/render/helper/CoordinateGrid.h"
#include "src/render/helper/SkyBox.h"
#include <QOpenGLFunctions_3_3_Core>
#include <glm/glm.hpp>
#include <sstream>
#include <vector>

namespace netsimulyzer {

class Renderer : protected QOpenGLFunctions_3_3_Core {
  ModelCache &modelCache;
  TextureCache &textureCache;
  FontManager &fontManager;

  /**
   * Camera view matrix with the
   * rotation inverted.
   *
   * Apply this to a model matrix
   * to have it always facing the
   * camera
   */
  glm::mat4 cameraRotateInverse;

  Shader areaShader;
  Shader buildingShader;
  Shader gridShader;
  Shader modelShader;
  Shader skyBoxShader;
  Shader pickingShader;
  Shader fontShader;
  Shader fontBackgroundShader;

  void initShader(Shader &s, const QString &vertexPath, const QString &fragmentPath);

public:
  enum class LightingMode { LightingEnabled, LightingDisabled };
  const unsigned int maxPointLights = 5u;
  const unsigned int maxSpotLights = 5u;

  Renderer(ModelCache &modelCache, TextureCache &textureCache, FontManager &fontManager);
  void init();
  void setPerspective(const glm::mat4 &perspective);

  void setPointLightCount(unsigned int count);
  void setSpotLightCount(unsigned int count);

  TrailBuffer allocateTrailBuffer(QOpenGLFunctions_3_3_Core *openGl, int size);
  Building::RenderInfo allocate(const parser::Building &building);
  Area::RenderInfo allocate(const parser::Area &area);
  WiredLink::RenderInfo allocate(const parser::WiredLink &link);
  Mesh allocateFloor(float size);
  void resize(Floor &f, float size);
  CoordinateGrid::RenderInfo allocateCoordinateGrid(float size, int stepSize);
  void resize(CoordinateGrid &grid, float size, int stepSize);

  void startTransparentDark();
  void startTransparentLight();
  void endTransparent();

  void renderPickingNode(unsigned int nodeId, const Model &m);

  void use(const Camera &cam);
  void use(const ArcCamera &cam);
  void render(const DirectionalLight &light);
  void render(const PointLight &light);
  void render(const SpotLight &light);
  void render(const std::vector<Area> &areas);
  void render(const std::vector<Building> &buildings);
  void renderOutlines(const std::vector<Building> &buildings, const glm::vec3 &color);
  void renderTrail(const TrailBuffer &buffer, const glm::vec3 &color);
  void render(const Node &node, bool isSelected, LightingMode lightingMode = LightingMode::LightingEnabled);
  void render(const Model &m, LightingMode lightingMode = LightingMode::LightingEnabled);
  void renderTransparent(const Model &m, LightingMode lightingMode = LightingMode::LightingEnabled);
  void render(Floor &f);
  void render(SkyBox &skyBox);
  void render(CoordinateGrid &coordinateGrid);
  void render(const std::vector<WiredLink> &wiredLinks);
  void render(const LogicalLink &link);
  void renderFont(const FontManager::FontBannerRenderInfo &info, const glm::vec3 &location, float scale);
};

} // namespace netsimulyzer
