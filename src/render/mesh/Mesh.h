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

#include "../material/material.h"
#include "Vertex.h"
#include <QOpenGLFunctions_3_3_Core>
#include <utility>

namespace visualization {

class Mesh : protected QOpenGLFunctions_3_3_Core {
public:
  struct MeshRenderInfo {
    unsigned int vao = 0u;
    unsigned int vbo = 0u;
    unsigned int ibo = 0u;
    int indexCount = 0;
  };

private:
  MeshRenderInfo renderInfo;
  Material material;

  void move(Mesh &&other) noexcept;

public:
  Mesh(const Vertex vertices[], unsigned int indices[], unsigned int vertexCount, int indexCount);

  // Allow Moves
  Mesh(Mesh &&other) noexcept {
    move(std::forward<Mesh &&>(other));
  };

  Mesh &operator=(Mesh &&other) noexcept {
    move(std::forward<Mesh &&>(other));
    return *this;
  };

  // Disallow copies
  Mesh(const Mesh &) = delete;
  Mesh &operator=(const Mesh &) = delete;

  [[nodiscard]] const Material &getMaterial() const;
  void setMaterial(const Material &value);

  [[nodiscard]] const MeshRenderInfo &getRenderInfo() const;

  void render();

  ~Mesh() override;
};

} // namespace visualization
