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

#include "TrailBuffer.h"
#include <algorithm>
#include <utility>

namespace netsimulyzer {

TrailBuffer::TrailBuffer(QOpenGLFunctions_3_3_Core *openGl, unsigned int vao, unsigned int vbo, int initialSize,
                         int vertexSize) noexcept
    : openGl{openGl}, vao{vao}, vbo{vbo}, bufferSize{initialSize}, vertexSize{vertexSize}, buffer(bufferSize) {
}

TrailBuffer::TrailBuffer(TrailBuffer &&other) noexcept
    : openGl{other.openGl}, vao{other.vao}, vbo{other.vbo}, bufferSize{other.bufferSize},
      vertexSize{other.vertexSize}, index{other.index}, _empty{other._empty}, buffer{std::move(other.buffer)} {
  // Clear these, so the `other` deconstructor doesn't delete our moved buffers
  other.vao = 0u;
  other.vbo = 0u;
}

TrailBuffer::~TrailBuffer() {
  openGl->glDeleteBuffers(1, &vbo);
  openGl->glDeleteVertexArrays(1, &vao);
}

void TrailBuffer::bind() const {
  openGl->glBindVertexArray(vao);
  openGl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
}

void TrailBuffer::render() const {
  if (empty())
    return;

  bind();
  openGl->glDrawArrays(GL_LINE_STRIP, 0, index);
}

void TrailBuffer::append(float x, float y, float z) {
  if (index == bufferSize - 1) {
    std::rotate(buffer.begin(), buffer.begin() + 1, buffer.end());

    buffer[index].x = x;
    buffer[index].y = y;
    buffer[index].z = z;

  } else {
    if (_empty)
      _empty = false;
    else
      index++;

    buffer[index].x = x;
    buffer[index].y = y;
    buffer[index].z = z;
  }

  openGl->glBindVertexArray(vao);
  openGl->glBindBuffer(GL_ARRAY_BUFFER, vbo);
  openGl->glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize * (index + 1), buffer.data());
}

void TrailBuffer::pop() {
  if (_empty)
    return;

  index--;
  if (index == -1) {
    _empty = true;
    index = 0;
  }
}

bool TrailBuffer::empty() const noexcept {
  return _empty;
}

} // namespace netsimulyzer
