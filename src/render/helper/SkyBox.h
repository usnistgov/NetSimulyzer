#pragma once

#include "src/render/mesh/Mesh.h"

namespace netsimulyzer {

class SkyBox {
  Mesh mesh;
  unsigned long textureId;

public:
  explicit SkyBox(unsigned long texture);
  Mesh &getMesh();
  [[nodiscard]] unsigned long getTextureId() const;
};

} // namespace netsimulyzer
