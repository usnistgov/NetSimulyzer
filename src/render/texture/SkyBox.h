#pragma once

#include "../mesh/Mesh.h"

namespace visualization {

class SkyBox {
  Mesh mesh;
  unsigned int textureId;

public:
  explicit SkyBox(unsigned long texture);
  Mesh &getMesh();
  [[nodiscard]] unsigned long getTextureId() const;
};

} // namespace visualization
