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

#include "LogicalLink.h"
#include "src/conversion.h"
#include <glm/gtc/type_ptr.hpp>
#include <utility>

static constexpr auto vertexSize = static_cast<long>(sizeof(float) * 3);

namespace netsimulyzer {

LogicalLink::LogicalLink(parser::LogicalLink model, QOpenGLFunctions_3_3_Core *gl) : model(std::move(model)), gl{gl} {
  renderInfo.color = toRenderColor(this->model.color);

  this->gl->glGenVertexArrays(1, &renderInfo.vao);
  this->gl->glBindVertexArray(renderInfo.vao);

  this->gl->glGenBuffers(1, &renderInfo.vbo);
  this->gl->glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);

  this->gl->glBufferData(GL_ARRAY_BUFFER, vertexSize * renderInfo.size, nullptr, GL_DYNAMIC_DRAW);

  // Location
  this->gl->glVertexAttribPointer(0u, 3, GL_FLOAT, GL_FALSE, vertexSize, nullptr);
  this->gl->glEnableVertexAttribArray(0u);
}

LogicalLink::~LogicalLink() {
  gl->glDeleteBuffers(1, &renderInfo.vbo);
  gl->glDeleteVertexArrays(1, &renderInfo.vao);
}

LogicalLink::LogicalLink(LogicalLink &&other) noexcept
    : model{std::move(other.model)}, renderInfo{other.renderInfo}, gl{other.gl} {
  // Clear the other RenderInfo so the destructor
  // doesn't delete the OpenGL buffers
  other.renderInfo.vao = 0;
  other.renderInfo.vbo = 0;
  other.renderInfo.size = 0;
}

LogicalLink &LogicalLink::operator=(LogicalLink &&other) noexcept {
  if (this == &other)
    return *this;
  model = std::move(other.model);
  renderInfo = other.renderInfo;
  gl = other.gl;

  // Clear the other RenderInfo so the destructor
  // doesn't delete the OpenGL buffers
  other.renderInfo.vao = 0;
  other.renderInfo.vbo = 0;
  other.renderInfo.size = 0;

  return *this;
}

void LogicalLink::notifyNodeMoved(unsigned nodeId, glm::vec3 position) {
  auto offset{0l};
  if (nodeId == model.nodes.second)
    offset = vertexSize;

  gl->glBindBuffer(GL_ARRAY_BUFFER, renderInfo.vbo);
  gl->glBufferSubData(GL_ARRAY_BUFFER, offset, vertexSize, glm::value_ptr(position));
}

const LogicalLink::RenderInfo &LogicalLink::getRenderInfo() const {
  return renderInfo;
}

} // namespace netsimulyzer