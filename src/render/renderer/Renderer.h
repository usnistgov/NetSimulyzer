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
#include "../camera/Camera.h"
#include "../helper/Floor.h"
#include "../light/DirectionalLight.h"
#include "../light/PointLight.h"
#include "../light/SpotLight.h"
#include "../model/Model.h"
#include "../model/ModelCache.h"
#include "../shader/Shader.h"
#include "../texture/TextureCache.h"
#include <QOpenGLFunctions_3_3_Core>
#include <glm/glm.hpp>
#include <sstream>
#include <vector>

namespace visualization {

class Renderer : protected QOpenGLFunctions_3_3_Core {
  ModelCache &modelCache;

  Shader buildingShader;
  Shader modelShader;

public:
  const unsigned int maxPointLights = 5u;
  const unsigned int maxSpotLights = 5u;

  explicit Renderer(ModelCache &modelCache);
  void init();
  void setPerspective(const glm::mat4 &perspective);

  void setPointLightCount(unsigned int count);
  void setSpotLightCount(unsigned int count);

  Building::RenderInfo allocate(const parser::Building &building);
  ModelRenderInfo allocateFloor(float size, unsigned int textureId, TextureCache &textureCache);

  void use(const Camera &cam);
  void render(const directional_light &light);
  void render(const point_light &light);
  void render(const SpotLight &light);
  void render(std::vector<Building> &buildings);
  void render(const Model &m);
  void render(Floor &f);
};

} // namespace visualization
