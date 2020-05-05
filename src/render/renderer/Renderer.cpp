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

#include "Renderer.h"
#include "../../coordinate.h"
#include "../material/material.h"
#include <QFile>
#include <QString>
#include <QTextStream>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

static std::string qFileToString(QFile &f) {
  if (!f.open(QFile::ReadOnly | QFile::Text))
    abort();
  QTextStream stream{&f};
  return stream.readAll().toStdString();
}

namespace visualization {

Renderer::Renderer(ModelCache &modelCache, TextureCache &textureCache)
    : modelCache(modelCache), textureCache(textureCache) {
}

void Renderer::init() {
  initializeOpenGLFunctions();

  QFile modelVertex{":/shader/shaders/model.vert"};
  auto modelVertexSrc = qFileToString(modelVertex);

  QFile modelFragment{":/shader/shaders/model.frag"};
  auto modelFragmentSrc = qFileToString(modelFragment);

  modelShader.init(modelVertexSrc, modelFragmentSrc);

  QFile buildingVertex{":shader/shaders/building.vert"};
  auto buildingVertexSrc = qFileToString(buildingVertex);

  QFile buildingFragment{":shader/shaders/building.frag"};
  auto buildingFragmentSrc = qFileToString(buildingFragment);

  buildingShader.init(buildingVertexSrc, buildingFragmentSrc);

  QFile skyBoxVertex{":/shader/shaders/skybox.vert"};
  auto skyBoxVertexSrc = qFileToString(skyBoxVertex);

  QFile skyBoxFragment{":/shader/shaders/skybox.frag"};
  auto skyBoxFragmentSrc = qFileToString(skyBoxFragment);

  skyBoxShader.init(skyBoxVertexSrc, skyBoxFragmentSrc);
}

void Renderer::setPerspective(const glm::mat4 &perspective) {
  modelShader.uniform("projection", perspective);
  buildingShader.uniform("projection", perspective);
  skyBoxShader.uniform("projection", perspective);
}

void Renderer::setPointLightCount(unsigned int count) {
  if (count > maxPointLights)
    assert(!"Point light count set higher than defined max");
  modelShader.uniform("pointLightCount", count);
}

void Renderer::setSpotLightCount(unsigned int count) {
  if (count > maxSpotLights)
    assert(!"Spot light count set higher than defined max");
  modelShader.uniform("spotLightCount", count);
}

Building::RenderInfo Renderer::allocate(const parser::Building &building) {
  Building::RenderInfo info;

  auto min = toRenderCoordinate(building.min);
  auto max = toRenderCoordinate(building.max);

  // clang-format off
  std::vector<float> vertices{
      min.x, min.y, min.z, // 0
      max.x, min.y, min.z, // 1
      max.x, min.y, max.z, // 2
      min.x, min.y, max.z, // 3
      min.x, max.y, min.z, // 4
      max.x, max.y, min.z, // 5
      max.x, max.y, max.z, // 6
      min.x, max.y, max.z  // 7
  };
  std::vector<unsigned int> indices{
      0u, 1u, 2u,
      3u, 0u, 2u,
      1u, 5u, 6u,
      2u, 1u, 6u,
      4u, 5u, 6u,
      7u, 4u, 6u,
      0u, 4u, 7u,
      3u, 0u, 7u,
      0u, 1u, 5u,
      4u, 0u, 5u,
      3u, 2u, 6u,
      7u, 3u, 6u
  };
  // clang-format on

  auto last_index = 7u;

  glGenVertexArrays(1, &info.vao);
  glBindVertexArray(info.vao);

  // Floors
  //   All floors are exactly the same height
  //   abs() just in case our coordinates are negative
  const auto floor_height = (std::abs(max.y) - std::abs(min.y)) / static_cast<float>(building.floors);
  for (auto currentFloor = 1; currentFloor < building.floors; currentFloor++) {
    const auto currentHeight = floor_height * currentFloor + min.y;

    vertices.insert(vertices.end(), {min.x, currentHeight, min.z});
    vertices.insert(vertices.end(), {max.x, currentHeight, min.z});
    vertices.insert(vertices.end(), {max.x, currentHeight, max.z});
    vertices.insert(vertices.end(), {min.x, currentHeight, max.z});

    // 0, 1, 2, 3, 0, 2
    indices.insert(indices.end(),
                   {last_index + 1, last_index + 2, last_index + 3, last_index + 4, last_index + 1, last_index + 3});
    last_index += 4;
  }

  // Walls
  // X

  // Find the size of each room
  auto roomLengthX = (max.x - min.x) / static_cast<float>(building.roomsX);

  for (auto currentRoom = 1; currentRoom < building.roomsX; currentRoom++) {
    auto currentWallPosition = roomLengthX * currentRoom + min.x;

    vertices.insert(vertices.end(), {currentWallPosition, min.y, min.z});
    vertices.insert(vertices.end(), {currentWallPosition, max.y, min.z});
    vertices.insert(vertices.end(), {currentWallPosition, max.y, max.z});
    vertices.insert(vertices.end(), {currentWallPosition, min.y, max.z});

    // 0, 1, 2, 3, 0, 2
    indices.insert(indices.end(),
                   {last_index + 1, last_index + 2, last_index + 3, last_index + 4, last_index + 1, last_index + 3});
    last_index += 4;
  }

  // Y (Z in OpenGl coordinates)
  auto roomLengthY = (max.z - min.z) / static_cast<float>(building.roomsY);

  for (auto currentRoom = 1; currentRoom < building.roomsY; currentRoom++) {
    auto currentWallPosition = roomLengthY * currentRoom + min.z;

    vertices.insert(vertices.end(), {min.x, min.y, currentWallPosition});
    vertices.insert(vertices.end(), {max.x, min.y, currentWallPosition});
    vertices.insert(vertices.end(), {max.x, max.y, currentWallPosition});
    vertices.insert(vertices.end(), {min.x, max.y, currentWallPosition});

    // 0, 1, 2, 3, 0, 2
    indices.insert(indices.end(),
                   {last_index + 1, last_index + 2, last_index + 3, last_index + 4, last_index + 1, last_index + 3});
    last_index += 4;
  }

  info.ibo_size = indices.size();

  glGenBuffers(1, &info.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * indices.size(), indices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &info.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, info.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

  // Location
  glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
  glEnableVertexAttribArray(0u);

  return info;
}

Mesh Renderer::allocateFloor(float size, unsigned int textureId) {
  unsigned int floorIndices[]{0u, 2u, 1u, 1u, 2u, 3u};
  std::array<float, 3> normal{0.0f, -1.0f, 1.0f};

  // Make sure this isn't negative
  size = std::abs(size);

  Vertex floorVertices[]{
      Vertex{{-size, 0.0f, -size}, normal, {0.0f, 0.0f}}, Vertex{{size, 0.0f, -size}, normal, {size, 0.0f}},
      Vertex{{-size, 0.0f, size}, normal, {0.0f, size}}, Vertex{{size, 0.0f, size}, normal, {size, size}}};

  Material floorMaterial;
  floorMaterial.shininess = 4.0f;
  floorMaterial.specularIntensity = 0.03f;
  floorMaterial.textureId = textureId;

  Mesh m{floorVertices, floorIndices, 4u, 6};
  m.setMaterial(floorMaterial);
  return m;
}

void Renderer::resize(Floor &f, float size) {
  // Make sure this isn't negative
  size = std::abs(size);

  std::array<float, 3> normal{0.0f, -1.0f, 1.0f};
  Vertex floorVertices[]{
      Vertex{{-size, 0.0f, -size}, normal, {0.0f, 0.0f}}, Vertex{{size, 0.0f, -size}, normal, {size, 0.0f}},
      Vertex{{-size, 0.0f, size}, normal, {0.0f, size}}, Vertex{{size, 0.0f, size}, normal, {size, size}}};

  const auto &renderInfo = f.getMesh().getRenderInfo();

  glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
  glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(floorVertices), reinterpret_cast<const void *>(floorVertices));
}

void Renderer::use(const Camera &cam) {
  modelShader.uniform("view", cam.view_matrix());
  modelShader.uniform("eye_position", cam.get_position());

  buildingShader.uniform("view", cam.view_matrix());

  // Drop the translation so we cannot move out of the sky box
  auto noTranslationView = cam.view_matrix();
  noTranslationView[3] = {0.0f, 0.0f, 0.0f, 1.0f};
  skyBoxShader.uniform("view", noTranslationView);
}

void Renderer::render(const DirectionalLight &light) {
  modelShader.uniform("directional_light.base.color", light.color);
  modelShader.uniform("directional_light.base.ambient_intensity", light.ambientIntensity);
  modelShader.uniform("directional_light.base.diffuse_intensity", light.diffuseIntensity);

  modelShader.uniform("directional_light.direction", light.direction);
}

void Renderer::render(const PointLight &light) {
  modelShader.uniform(light.prefix + "base.color", light.color);
  modelShader.uniform(light.prefix + "base.ambient_intensity", light.ambientIntensity);
  modelShader.uniform(light.prefix + "base.diffuse_intensity", light.diffuseIntensity);

  modelShader.uniform(light.prefix + "position", light.position);

  modelShader.uniform(light.prefix + "constant", light.constant);
  modelShader.uniform(light.prefix + "linear", light.linear);
  modelShader.uniform(light.prefix + "exponent", light.exponent);
}

void Renderer::render(const SpotLight &light) {
  modelShader.uniform(light.prefix + "pointLight.base.color", light.color);
  modelShader.uniform(light.prefix + "pointLight.base.ambient_intensity", light.ambientIntensity);
  modelShader.uniform(light.prefix + "pointLight.base.diffuse_intensity", light.diffuseIntensity);

  modelShader.uniform(light.prefix + "pointLight.position", light.position);
  modelShader.uniform(light.prefix + "direction", light.direction);

  modelShader.uniform(light.prefix + "pointLight.constant", light.constant);
  modelShader.uniform(light.prefix + "pointLight.linear", light.linear);
  modelShader.uniform(light.prefix + "pointLight.exponent", light.exponent);

  modelShader.uniform(light.prefix + "edge", light.processedEdge);
}

void Renderer::render(std::vector<Building> &buildings) {
  buildingShader.bind();
  glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  glBlendEquation(GL_FUNC_ADD);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  for (const auto &building : buildings) {
    const auto &renderInfo = building.getRenderInfo();
    buildingShader.uniform("color", building.getColor());

    glBindVertexArray(renderInfo.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderInfo.ibo);
    glDrawElements(GL_TRIANGLES, renderInfo.ibo_size, GL_UNSIGNED_INT, nullptr);
  }

  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Renderer::render(const Model &m) {
  modelShader.bind();
  modelShader.uniform("model", m.getModelMatrix());
  modelCache.get(m.getModelId()).render(modelShader);
}

void Renderer::render(Floor &f) {
  modelShader.bind();
  modelShader.uniform("model", f.getModelMatrix());
  textureCache.use(f.getTextureId());
  f.render();
}

void Renderer::render(SkyBox &skyBox) {
  glDepthMask(GL_FALSE);
  skyBoxShader.bind();

  textureCache.useCubeMap(skyBox.getTextureId());
  skyBox.getMesh().render();
  glDepthMask(GL_TRUE);
}

} // namespace visualization
