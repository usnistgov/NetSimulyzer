#include "SkyBox.h"
#include "src/render/mesh/Vertex.h"

namespace {
unsigned int indices[]{0u, 1u, 2u, 2u, 1u, 3u, 2u, 3u, 5u, 5u, 3u, 7u, 5u, 7u, 4u, 4u, 7u, 6u,
                       4u, 6u, 0u, 0u, 6u, 1u, 4u, 0u, 5u, 5u, 0u, 2u, 1u, 6u, 3u, 3u, 6u, 7u};

netsimulyzer::Vertex vertices[]{
    {{-1.0f, 1.0f, -1.0f}}, {{-1.0f, -1.0f, -1.0f}}, {{1.0f, 1.0f, -1.0f}},  {{1.0f, -1.0f, -1.0f}},
    {{-1.0f, 1.0f, 1.0f}},  {{1.0f, 1.0f, 1.0f}},    {{-1.0f, -1.0f, 1.0f}}, {{1.0f, -1.0f, 1.0f}},
};
} // namespace

namespace netsimulyzer {

SkyBox::SkyBox(unsigned long texture) : mesh(vertices, indices, 8u, 36u), textureId(texture) {
}

Mesh &SkyBox::getMesh() {
  return mesh;
}

unsigned long SkyBox::getTextureId() const {
  return textureId;
}

} // namespace netsimulyzer
