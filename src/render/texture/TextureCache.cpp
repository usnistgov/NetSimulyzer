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

#include "TextureCache.h"
#include "../mesh/Mesh.h"
#include <QImage>
#include <cstdio>
#include <iostream>
#include <stb_image.h>

namespace visualization {

unsigned int TextureCache::loadFallback(QImage &texture) {
  Texture t;
  t.width = texture.width();
  t.height = texture.height();

  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, texture.width(), texture.height(), 0, GL_BGRA, GL_UNSIGNED_BYTE,
               texture.constBits());

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0u);

  textures.emplace_back(t);
  const auto fallbackIndex = textures.size() - 1;
  indexMap.emplace("fallback", fallbackIndex);

  fallbackTexture = fallbackIndex;
  return fallbackIndex;
}

TextureCache::~TextureCache() {
  clear();
}

std::size_t TextureCache::load(const std::string &path) {
  // If we've already loaded the texture, use that ID
  auto existing = indexMap.find(path);
  if (existing != indexMap.end()) {
    return existing->second;
  }

  Texture t;
  int depth;
  auto *data = stbi_load(path.c_str(), &t.width, &t.height, &depth, 0);
  if (!data) {
    std::cerr << "Failed to load " << path << '\n';
    if (fallbackTexture)
      return *fallbackTexture;
    std::cerr << "No fallback texture loaded! Nothing to fallback on\n";
    std::abort();
  }

  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  QImage test{path.c_str()};

  unsigned int format;
  if (depth == 4)
    format = GL_RGBA;
  else if (depth == 3)
    format = GL_RGB;
  else {
    std::cerr << "Unsupported texture depth in texture: " << path << '\n' << "Depth: " << depth << '\n';
    if (fallbackTexture)
      return *fallbackTexture;
    std::cerr << "No fallback texture!\n";
    std::abort();
  }
  glTexImage2D(GL_TEXTURE_2D, 0, format, t.width, t.height, 0, format, GL_UNSIGNED_BYTE, data);

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0u);
  stbi_image_free(data);

  textures.emplace_back(t);
  const auto newIndex = textures.size() - 1;
  indexMap.emplace(path, newIndex);
  return newIndex;
}

unsigned long TextureCache::loadSkyBox(const std::array<QImage, 6> &images) {
  unsigned int id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);

  for (auto i = 0u; i < images.size(); i++) {
    const auto &image = images[i];
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, image.width(), image.height(), 0, GL_BGRA,
                 GL_UNSIGNED_BYTE, image.constBits());
  }

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return id;
}

const Texture &TextureCache::get(std::size_t index) {
  return textures[index];
}

void TextureCache::clear() {
  for (const auto &t : textures) {
    glDeleteTextures(1, &t.id);
  }

  textures.clear();
  indexMap.clear();
  fallbackTexture.reset();
}

void TextureCache::use(std::size_t index) {
  const auto &t = textures[index];
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, t.id);
}

void TextureCache::useSkyBox(unsigned int id) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

const std::optional<unsigned long> &TextureCache::getFallbackTexture() const {
  return fallbackTexture;
}

} // namespace visualization
