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
#include "../../conversion.h"
#include "../material/material.h"
#include <QFile>
#include <QMessageBox>
#include <QString>
#include <QTextStream>
#include <array>
#include <cassert>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>

namespace netsimulyzer {

void Renderer::initShader(Shader &s, const QString &vertexPath, const QString &fragmentPath) {
  QFile vertexFile{vertexPath};
  if (!vertexFile.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::critical(nullptr, "Failed to open shader file", "Failed to open shader file " + vertexPath);
    std::abort();
  }
  auto vertexSrc = QTextStream{&vertexFile}.readAll().toStdString();

  QFile fragmentFile{fragmentPath};
  if (!fragmentFile.open(QFile::ReadOnly | QFile::Text)) {
    QMessageBox::critical(nullptr, "Failed to open shader file", "Failed to open shader file " + fragmentPath);
    std::abort();
  }
  auto fragmentSrc = QTextStream{&fragmentFile}.readAll().toStdString();

  s.init(vertexSrc, fragmentSrc);
}

Renderer::Renderer(ModelCache &modelCache, TextureCache &textureCache, FontManager &fontManager)
    : modelCache(modelCache), textureCache(textureCache), fontManager(fontManager) {
}

void Renderer::init() {
  initializeOpenGLFunctions();

  initShader(areaShader, ":shader/shaders/area.vert", ":shader/shaders/area.frag");
  initShader(buildingShader, ":shader/shaders/building.vert", ":shader/shaders/building.frag");

  initShader(gridShader, ":shader/shaders/grid.vert", ":shader/shaders/grid.frag");
  gridShader.uniform("discard_distance", 250.0f);
  gridShader.uniform("height", -0.001f);

  initShader(modelShader, ":shader/shaders/model.vert", ":shader/shaders/model.frag");
  initShader(skyBoxShader, ":shader/shaders/skybox.vert", ":shader/shaders/skybox.frag");
  initShader(pickingShader, ":/shader/shaders/picking.vert", ":/shader/shaders/picking.frag");
  initShader(fontShader, ":/shader/shaders/font.vert", ":/shader/shaders/font.frag");
  initShader(fontBackgroundShader, ":/shader/shaders/font_bg.vert", ":/shader/shaders/font_bg.frag");
}

void Renderer::setPerspective(const glm::mat4 &perspective) {
  areaShader.uniform("projection", perspective);
  buildingShader.uniform("projection", perspective);
  gridShader.uniform("projection", perspective);
  modelShader.uniform("projection", perspective);
  skyBoxShader.uniform("projection", perspective);
  pickingShader.uniform("projection", perspective);
  fontShader.uniform("projection", perspective);
  fontBackgroundShader.uniform("projection", perspective);
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

TrailBuffer Renderer::allocateTrailBuffer(QOpenGLFunctions_3_3_Core *openGl, int size) {
  const auto vertexSize = sizeof(float) * 3;

  // Vao
  unsigned int vao;
  openGl->glGenVertexArrays(1, &vao);
  openGl->glBindVertexArray(vao);

  // Vbo
  unsigned int vbo;
  openGl->glGenBuffers(1, &vbo);
  openGl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
  openGl->glBufferData(GL_ARRAY_BUFFER, vertexSize * size, nullptr, GL_DYNAMIC_DRAW);

  // Location
  openGl->glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
  openGl->glEnableVertexAttribArray(0u);

  TrailBuffer buffer{openGl, vao, vbo, size, vertexSize};
  return buffer;
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

  // Border Lines

  // add a very slight offset
  // so lines do not directly
  // intersect the walls
  const float offset = 0.01f;

  // clang-format off
  const std::array<float, 24> borderVertices {
      min.x - offset, min.y - offset, min.z - offset, // 0
      max.x + offset, min.y - offset, min.z - offset, // 1
      max.x + offset, min.y - offset, max.z + offset, // 2
      min.x - offset, min.y - offset, max.z + offset, // 3
      min.x - offset, max.y + offset, min.z - offset, // 4
      max.x + offset, max.y + offset, min.z - offset, // 5
      max.x + offset, max.y + offset, max.z + offset, // 6
      min.x - offset, max.y + offset, max.z + offset  // 7
  };
  const std::array<unsigned int, 24> lineIndices {
      0u, 1u, // Bottom
      1u, 2u,
      2u, 3u,
      3u, 0u,
      4u, 5u, // Top
      5u, 6u,
      6u, 7u,
      7u, 4u,
      0u, 4u, // Sides
      1u, 5u,
      2u, 6u,
      3u, 7u,
  };
  // clang-format on

  glGenVertexArrays(1, &info.lineVao);
  glBindVertexArray(info.lineVao);

  info.lineIboSize = lineIndices.size();

  glGenBuffers(1, &info.lineIbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, info.lineIbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * lineIndices.size(), lineIndices.data(), GL_STATIC_DRAW);

  glGenBuffers(1, &info.lineVbo);
  glBindBuffer(GL_ARRAY_BUFFER, info.lineVbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * borderVertices.size(), borderVertices.data(), GL_STATIC_DRAW);

  // Location
  glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
  glEnableVertexAttribArray(0u);

  return info;
}

Area::RenderInfo Renderer::allocate(const parser::Area &area) {
  Area::RenderInfo info;

  // Convert to OpenGl coordinates
  // for easier reading later
  // Also handle the fill
  std::vector<glm::vec3> convertedPoints;
  convertedPoints.reserve(area.points.size());
  for (const auto &point : area.points) {
    auto converted = toRenderCoordinate(point);
    convertedPoints.emplace_back(converted);

    // Handle the fill vertexes
    // since we're already looping through all the points
  }

  using DrawMode = parser::Area::DrawMode;

  // Fill
  info.renderFill = area.fillMode == DrawMode::Solid;
  if (info.renderFill) {
    std::vector<float> fillVertices;
    fillVertices.reserve(area.points.size() * 3uL);

    for (const auto &point : convertedPoints) {
      // It's easiest to pass a flat array of floats
      // with only one vertex attribute
      // (Location in this case)
      fillVertices.insert(fillVertices.end(), {point.x, point.y, point.z});
    }

    glGenVertexArrays(1, &info.fillVao);
    glBindVertexArray(info.fillVao);

    // Size is in number of three component vertices
    // not raw number of floats
    // so the ns-3 or converted point count would work
    info.fillVbo_size = area.points.size();

    glGenBuffers(1, &info.fillVbo);
    glBindBuffer(GL_ARRAY_BUFFER, info.fillVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * fillVertices.size(), fillVertices.data(), GL_STATIC_DRAW);

    // Location Attribute
    glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
    glEnableVertexAttribArray(0u);
  }

  // Border
  info.renderBorder = area.borderMode == DrawMode::Solid;
  if (info.renderBorder) {
    const auto borderWidth = 0.5f; // TODO: Make configurable?

    // TODO: Filled Corners?
    float borderPoints[] = {
        // Top Left
        convertedPoints[0].x, convertedPoints[0].y, convertedPoints[0].z,               // 0
        convertedPoints[0].x - borderWidth, convertedPoints[0].y, convertedPoints[0].z, // 1

        // Bottom Left
        convertedPoints[1].x, convertedPoints[1].y, convertedPoints[1].z,               // 2
        convertedPoints[1].x - borderWidth, convertedPoints[1].y, convertedPoints[1].z, // 3
        convertedPoints[1].x, convertedPoints[1].y, convertedPoints[1].z + borderWidth, // 4

        // Bottom Right
        convertedPoints[2].x, convertedPoints[2].y, convertedPoints[2].z,               // 5
        convertedPoints[2].x, convertedPoints[2].y, convertedPoints[2].z + borderWidth, // 6
        convertedPoints[2].x + borderWidth, convertedPoints[2].y, convertedPoints[2].z, // 7

        // Top Right
        convertedPoints[3].x, convertedPoints[3].y, convertedPoints[3].z,               // 8
        convertedPoints[3].x + borderWidth, convertedPoints[3].y, convertedPoints[3].z, // 9
        convertedPoints[3].x, convertedPoints[3].y, convertedPoints[3].z - borderWidth, // 10

        // Top Left (Again)
        convertedPoints[0].x, convertedPoints[0].y, convertedPoints[0].z,               // 11 (same as 0)
        convertedPoints[0].x, convertedPoints[0].y, convertedPoints[0].z - borderWidth, // 12
        convertedPoints[0].x - borderWidth, convertedPoints[0].y, convertedPoints[0].z, // 13 (same as 1)
    };

    info.borderVbo_size = 14u;
    info.borderColor = toRenderColor(area.borderColor);

    glGenVertexArrays(1, &info.borderVao);
    glBindVertexArray(info.borderVao);

    glGenBuffers(1, &info.borderVbo);
    glBindBuffer(GL_ARRAY_BUFFER, info.borderVbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(borderPoints), borderPoints, GL_STATIC_DRAW);

    // Location
    glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
    glEnableVertexAttribArray(0u);
  }

  return info;
}

WiredLink::RenderInfo Renderer::allocate(const parser::WiredLink &link) {
  WiredLink::RenderInfo info;

  info.size = static_cast<int>(link.nodes.size());

  glGenVertexArrays(1, &info.vao);
  glBindVertexArray(info.vao);

  glGenBuffers(1, &info.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, info.vbo);

  // Location data is set when this link is added to each node
  glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * link.nodes.size(), nullptr, GL_DYNAMIC_DRAW);

  // Location
  glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, nullptr);
  glEnableVertexAttribArray(0u);

  return info;
}

Mesh Renderer::allocateFloor(float size) {
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
  floorMaterial.color = {0.56f, 0.79f, 0.39f}; // Light Green
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

CoordinateGrid::RenderInfo Renderer::allocateCoordinateGrid(float size, int stepSize) {
  using CVertex = CoordinateGrid::Vertex;
  // Make sure this isn't negative
  size = std::abs(size);
  // Align grid to integer coordinates
  size = std::floor(size);

  // Allocate points
  std::vector<CoordinateGrid::Vertex> gridVertices;
  gridVertices.reserve(size * 4uL);

  for (auto i = -size; i <= size; i += static_cast<float>(stepSize)) { // NOLINT(cert-flp30-c)
    // Vertical Line
    gridVertices.emplace_back(CVertex{{i, size}});
    gridVertices.emplace_back(CVertex{{i, -size}});

    // Horizontal Line
    gridVertices.emplace_back(CVertex{{-size, i}});
    gridVertices.emplace_back(CVertex{{size, i}});
  }

  // Setup VAO & VBO
  // No IBO, since no points are repeated
  CoordinateGrid::RenderInfo renderInfo;

  glGenVertexArrays(1, &renderInfo.vao);
  glBindVertexArray(renderInfo.vao);

  glGenBuffers(1, &renderInfo.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(CVertex) * gridVertices.size(), gridVertices.data(), GL_STATIC_DRAW);

  // Location
  glVertexAttribPointer(0u, 2, GL_FLOAT, GL_FALSE, sizeof(CVertex),
                        reinterpret_cast<void *>(offsetof(CVertex, position)));
  glEnableVertexAttribArray(0u);

  glBindVertexArray(0u);

  renderInfo.size = gridVertices.size();
  renderInfo.squareSize = size;
  return renderInfo;
}

void Renderer::resize(CoordinateGrid &grid, float size, int stepSize) {
  using CVertex = CoordinateGrid::Vertex;
  const auto &renderInfo = grid.getRenderInfo();

  // Regenerate the grid using the same steps as allocation
  size = std::abs(size);
  size = std::floor(size);

  std::vector<CoordinateGrid::Vertex> gridVertices;
  gridVertices.reserve(size * 4uL);

  for (auto i = -size; i <= size; i += static_cast<float>(stepSize)) { // NOLINT(cert-flp30-c)
    // Vertical Line
    gridVertices.emplace_back(CVertex{{i, size}});
    gridVertices.emplace_back(CVertex{{i, -size}});

    // Horizontal Line
    gridVertices.emplace_back(CVertex{{-size, i}});
    gridVertices.emplace_back(CVertex{{size, i}});
  }

  glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(CVertex) * gridVertices.size(), gridVertices.data(), GL_STATIC_DRAW);

  // Notify the grid of the resize
  grid.resized(gridVertices.size(), size);
}

void Renderer::startTransparentDark() {
  glBlendFunc(GL_ZERO, GL_SRC_COLOR);
  glBlendEquation(GL_FUNC_ADD);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
}

void Renderer::startTransparentLight() {
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);
}

void Renderer::endTransparent() {
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Renderer::use(const Camera &cam) {
  areaShader.uniform("view", cam.view_matrix());

  modelShader.uniform("view", cam.view_matrix());
  modelShader.uniform("eye_position", cam.get_position());

  buildingShader.uniform("view", cam.view_matrix());

  gridShader.uniform("view", cam.view_matrix());
  gridShader.uniform("eye_position", cam.get_position());

  // Drop the translation so we cannot move out of the sky box
  auto noTranslationView = cam.view_matrix();
  noTranslationView[3] = {0.0f, 0.0f, 0.0f, 1.0f};
  skyBoxShader.uniform("view", noTranslationView);

  pickingShader.uniform("view", cam.view_matrix());

  // Convert to 3x3 since that's the rotation section of the model matrix (the top left 3x3)
  // then invert that.
  cameraRotateInverse = glm::inverse(glm::mat3x3(cam.view_matrix()));

  fontShader.uniform("view", cam.view_matrix());
  fontBackgroundShader.uniform("view", cam.view_matrix());
}

void Renderer::use(const ArcCamera &cam) {
  areaShader.uniform("view", cam.viewMatrix());

  modelShader.uniform("view", cam.viewMatrix());
  modelShader.uniform("eye_position", cam.position);

  buildingShader.uniform("view", cam.viewMatrix());

  gridShader.uniform("view", cam.viewMatrix());
  gridShader.uniform("eye_position", cam.position);

  // Drop the translation so we cannot move out of the sky box
  auto noTranslationView = cam.viewMatrix();
  noTranslationView[3] = {0.0f, 0.0f, 0.0f, 1.0f};
  skyBoxShader.uniform("view", noTranslationView);

  pickingShader.uniform("view", cam.viewMatrix());

  // Convert to 3x3 since that's the rotation section of the model matrix (the top left 3x3)
  // then invert that.
  cameraRotateInverse = glm::inverse(glm::mat3x3(cam.viewMatrix()));

  fontShader.uniform("view", cam.viewMatrix());
  fontBackgroundShader.uniform("view", cam.viewMatrix());
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

void Renderer::render(const std::vector<Area> &areas) {
  areaShader.bind();

  for (const auto &area : areas) {
    const auto &renderInfo = area.getRenderInfo();
    if (renderInfo.renderFill) {
      areaShader.uniform("color", renderInfo.fillColor);
      glBindVertexArray(renderInfo.fillVao);
      glDrawArrays(GL_TRIANGLE_FAN, 0, renderInfo.fillVbo_size);
    }

    if (renderInfo.renderBorder) {
      areaShader.uniform("color", renderInfo.borderColor);
      glBindVertexArray(renderInfo.borderVao);
      glDrawArrays(GL_TRIANGLE_STRIP, 0, renderInfo.borderVbo_size);
    }
  }
}

void Renderer::render(const std::vector<Building> &buildings) {
  buildingShader.bind();
  for (const auto &building : buildings) {
    if (!building.visible())
      continue;
    const auto &renderInfo = building.getRenderInfo();
    buildingShader.uniform("color", building.getColor());

    glBindVertexArray(renderInfo.vao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderInfo.ibo);
    glDrawElements(GL_TRIANGLES, renderInfo.ibo_size, GL_UNSIGNED_INT, nullptr);
  }
}

void Renderer::renderOutlines(const std::vector<Building> &buildings, const glm::vec3 &color) {
  buildingShader.bind();
  for (const auto &building : buildings) {
    if (!building.visible())
      continue;
    const auto &renderInfo = building.getRenderInfo();

    glBindVertexArray(renderInfo.lineVao);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderInfo.lineIbo);
    buildingShader.uniform("color", color);
    glDrawElements(GL_LINES, renderInfo.lineIboSize, GL_UNSIGNED_INT, nullptr);
  }
}

void Renderer::renderTrail(const TrailBuffer &buffer, const glm::vec3 &color) {
  glEnable(GL_LINE_SMOOTH);

  buildingShader.bind();
  buildingShader.uniform("color", color);
  buffer.render();

  glDisable(GL_LINE_SMOOTH);
}

void Renderer::render(const Node &node, bool isSelected, LightingMode lightingMode) {
  const auto &m = node.getModel();

  modelShader.bind();
  modelShader.uniform("is_selected", isSelected);
  modelShader.uniform("model", m.getModelMatrix());
  modelShader.uniform("useLighting", lightingMode == LightingMode::LightingEnabled);
  modelCache.get(m.getModelId()).render(modelShader, m);

  modelShader.uniform("is_selected", false);
}

void Renderer::render(const Model &m, LightingMode lightingMode) {
  modelShader.bind();
  modelShader.uniform("is_selected", false);
  modelShader.uniform("model", m.getModelMatrix());
  modelShader.uniform("useLighting", lightingMode == LightingMode::LightingEnabled);
  modelCache.get(m.getModelId()).render(modelShader, m);
}

void Renderer::renderTransparent(const Model &m, LightingMode lightingMode) {
  auto &renderInfo = modelCache.get(m.getModelId());

  if (!renderInfo.hasTransparentMeshes())
    return;

  modelShader.bind();
  modelShader.uniform("model", m.getModelMatrix());
  modelShader.uniform("useLighting", lightingMode == LightingMode::LightingEnabled);
  modelShader.uniform("is_selected", false);
  renderInfo.renderTransparent(modelShader, m);
}

void Renderer::render(Floor &f) {
  modelShader.bind();
  modelShader.uniform("model", f.getModelMatrix());
  modelShader.uniform("useTexture", false);
  modelShader.uniform("material_color", f.getMesh().getMaterial().color.value());
  modelShader.uniform("is_selected", false);
  f.render();
}

void Renderer::render(SkyBox &skyBox) {
  glDepthMask(GL_FALSE);
  skyBoxShader.bind();

  textureCache.useCubeMap(skyBox.getTextureId());
  skyBox.getMesh().render();
  glDepthMask(GL_TRUE);
}

void Renderer::render(CoordinateGrid &coordinateGrid) {
  const auto &renderInfo = coordinateGrid.getRenderInfo();
  glEnable(GL_LINE_SMOOTH);
  glBlendFunc(GL_ONE, GL_ONE);
  glBlendEquation(GL_FUNC_ADD);

  glDepthMask(GL_FALSE);
  glEnable(GL_BLEND);

  gridShader.bind();

  // TODO: Make configurable
  gridShader.uniform("intensity", 0.3f);

  glBindVertexArray(renderInfo.vao);
  glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
  glDrawArrays(GL_LINES, 0, renderInfo.size);

  glBindVertexArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glDisable(GL_LINE_SMOOTH);
  glDisable(GL_BLEND);
  glDepthMask(GL_TRUE);
}

void Renderer::render(const std::vector<WiredLink> &wiredLinks) {
  glEnable(GL_LINE_SMOOTH);

  buildingShader.bind();
  // TODO: Make configurable
  buildingShader.uniform("color", {0.0f, 0.0f, 0.0f});

  for (const auto &wiredLink : wiredLinks) {
    const auto &renderInfo = wiredLink.getRenderInfo();
    glBindVertexArray(renderInfo.vao);
    glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
    glDrawArrays(GL_LINES, 0, renderInfo.size);
  }

  glDisable(GL_LINE_SMOOTH);
}

void Renderer::renderPickingNode(unsigned int nodeId, const Model &m) {
  auto &model = modelCache.get(m.getModelId());

  pickingShader.uniform("model", m.getModelMatrix());
  pickingShader.uniform("object_id", nodeId);
  pickingShader.uniform("object_type", 1u);

  pickingShader.bind();
  auto &meshes = model.getMeshes();
  for (auto &mesh : meshes) {
    mesh.render();
  }
  auto &transparentMeshes = model.getTransparentMeshes();
  for (auto &mesh : transparentMeshes) {
    mesh.render();
  }
}

void Renderer::renderFont(const FontManager::FontBannerRenderInfo &info, const glm::vec3 &location, float scale) {
  // TODO: Maybe make this configurable?
  const glm::vec3 offset{0.0f, 2.0f, 0.0f};

  auto modelMatrix = glm::translate(glm::identity<glm::mat4>(), location + offset);
  modelMatrix = glm::scale(modelMatrix, glm::vec3{scale});
  modelMatrix *= cameraRotateInverse;

  // ----- Background -----
  startTransparentDark();
  fontBackgroundShader.bind();
  fontBackgroundShader.uniform("model", glm::translate(modelMatrix, {0.0, 0.0, -0.01f}));

  glBindVertexArray(info.backgroundVao);
  glBindBuffer(GL_ARRAY_BUFFER, info.backgroundVbo);
  glDrawArrays(GL_TRIANGLES, 0, info.backgroundVboSize);

  // ----- Glyphs -----
  startTransparentLight();

  textureCache.use(fontManager.getAtlasTexture());

  fontShader.bind();
  fontShader.uniform("model", modelMatrix);

  glBindVertexArray(info.glyphVao);
  glBindBuffer(GL_ARRAY_BUFFER, info.glyphVbo);
  glDrawArrays(GL_TRIANGLES, 0, info.glyphVboSize);
}

} // namespace netsimulyzer
