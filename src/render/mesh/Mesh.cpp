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

#include "Mesh.h"
#include "Vertex.h"
#include <algorithm>

namespace netsimulyzer {

void Mesh::move(Mesh &&other) noexcept {
  initializeOpenGLFunctions();
  renderInfo = other.renderInfo;
  material = other.material;
  bounds = other.bounds;

  // Clear the other one
  // so it doesn't delete the mesh
  other.renderInfo.vao = 0u;
  other.renderInfo.vbo = 0u;
  other.renderInfo.ibo = 0u;
  other.renderInfo.indexCount = 0u;
}

const Material &Mesh::getMaterial() const {
  return material;
}

void Mesh::setMaterial(const Material &value) {
  material = value;
}

Mesh::Mesh(const Vertex vertices[], unsigned int indices[], unsigned int vertexCount, int indexCount) {
  initializeOpenGLFunctions();
  renderInfo.indexCount = indexCount;

  if (vertexCount > 0u) {
    const auto &v = vertices[0];
    bounds.min = {v.position[0], v.position[1], v.position[2]};
    bounds.max = bounds.min;
  }

  for (auto i = 0u; i < vertexCount; i++) {
    const auto &position = vertices[i].position;
    bounds.max.x = std::max(bounds.max.x, position[0]);
    bounds.min.x = std::min(bounds.min.x, position[0]);

    bounds.max.y = std::max(bounds.max.y, position[1]);
    bounds.min.y = std::min(bounds.min.y, position[1]);

    bounds.max.z = std::max(bounds.max.z, position[2]);
    bounds.min.z = std::min(bounds.min.z, position[2]);
  }

  glGenVertexArrays(1, &renderInfo.vao);
  glBindVertexArray(renderInfo.vao);

  glGenBuffers(1, &renderInfo.ibo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderInfo.ibo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned int) * renderInfo.indexCount, indices, GL_STATIC_DRAW);

  glGenBuffers(1, &renderInfo.vbo);
  glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertexCount, vertices, GL_STATIC_DRAW);

  // Location
  glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, position)));
  glEnableVertexAttribArray(0u);

  // Normal
  glVertexAttribPointer(1u, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), reinterpret_cast<void *>(offsetof(Vertex, normal)));
  glEnableVertexAttribArray(1u);

  // Texture
  glVertexAttribPointer(2u, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        reinterpret_cast<void *>(offsetof(Vertex, textureCoordinate)));
  glEnableVertexAttribArray(2u);

  glBindVertexArray(0u);
}

const Mesh::MeshRenderInfo &Mesh::getRenderInfo() const {
  return renderInfo;
}

const Mesh::MeshBounds &Mesh::getBounds() const {
  return bounds;
}

void Mesh::render() {
  glBindVertexArray(renderInfo.vao);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, renderInfo.ibo);
  glDrawElements(GL_TRIANGLES, renderInfo.indexCount, GL_UNSIGNED_INT, nullptr);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

Mesh::~Mesh() {
  glDeleteBuffers(1, &renderInfo.ibo);
  renderInfo.ibo = 0;

  glDeleteBuffers(1, &renderInfo.vbo);
  renderInfo.vbo = 0;

  glDeleteVertexArrays(1, &renderInfo.vao);
  renderInfo.vao = 0;

  renderInfo.indexCount = 0;
}

} // namespace netsimulyzer
