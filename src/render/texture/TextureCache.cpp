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
#include <QColor>
#include <QDebug>
#include <QDir>
#include <QImage>
#include <QString>
#include <Qt>
#include <iostream>
#include <utility>

namespace {
std::optional<QFileInfo> findTexture(const QDir &base, const QString &fileName, unsigned int max = 25u) {
  // Cut us off after `max` levels
  if (max == 0u)
    return {};

  QDir searchFile{base};
  searchFile.setFilter(QDir::Files | QDir::Readable);
  searchFile.setNameFilters({fileName});

  auto fileResults = searchFile.entryInfoList();
  if (fileResults.count() > 0)
    return {fileResults[0]};

  QDir subDirectories{base};
  subDirectories.setFilter(QDir::Dirs | QDir::Readable | QDir::NoDotAndDotDot);
  // no name filter

  auto directoryResults = subDirectories.entryInfoList();
  for (const auto &subDirectory : directoryResults) {
    auto result = findTexture({subDirectory.canonicalFilePath()}, fileName, max - 1);
    if (result)
      return result;
  }

  return {};
}

} // namespace

namespace netsimulyzer {

void TextureCache::setResourceDirectory(const QDir &value) {
  resourceDirectory = value;
}

TextureCache::~TextureCache() {
  clear();
}

bool TextureCache::init() {
  if (!initializeOpenGLFunctions())
    return false;

  // Generate a fallback texture
  QImage fallback{64, 64, QImage::Format::Format_ARGB32};
  fallback.fill(Qt::GlobalColor::magenta);

  Texture t;
  t.width = fallback.width();
  t.height = fallback.height();

  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, t.width, t.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, fallback.constBits());

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0u);

  textures.emplace_back(t);
  fallbackTexture = textures.size() - 1;
  indexMap.emplace("fallback", fallbackTexture);

  return true;
}

texture_id TextureCache::load(const std::string &filename) {

  // If we've already loaded the texture, use that ID
  auto existing = indexMap.find(filename);
  if (existing != indexMap.end()) {
    return existing->second;
  }

  auto result = findTexture(resourceDirectory, QString::fromStdString(filename));
  if (!result)
    return fallbackTexture;

  QImage image{result->canonicalFilePath()};
  if (image.isNull())
    return fallbackTexture;

  unsigned int glFormat;
  switch (image.format()) {
  case QImage::Format::Format_RGB32:
    glFormat = GL_RGB;
    break;
  case QImage::Format::Format_ARGB32:
    glFormat = GL_RGBA;
    break;
  default:
    qDebug() << "Unsupported format: " << image.format() << "\nAttempting conversion...";
    image = image.convertToFormat(QImage::Format_ARGB32);
    glFormat = GL_RGBA;

    if (image.isNull())
      return fallbackTexture;
  }

  Texture t;
  t.height = image.height();
  t.width = image.width();

  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  // QImage keeps BGRA format, event without an alpha channel
  glTexImage2D(GL_TEXTURE_2D, 0, glFormat, t.width, t.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.constBits());

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0u);

  textures.emplace_back(t);
  const auto newIndex = textures.size() - 1;
  indexMap.emplace(result->canonicalFilePath().toStdString(), newIndex);
  return newIndex;
}

unsigned int TextureCache::load(const CubeMap &cubeMap) {
  unsigned int id;
  glGenTextures(1, &id);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);

  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, cubeMap.right.width(), cubeMap.right.height(), 0, GL_BGRA,
               GL_UNSIGNED_BYTE, cubeMap.right.constBits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, cubeMap.left.width(), cubeMap.left.height(), 0, GL_BGRA,
               GL_UNSIGNED_BYTE, cubeMap.left.constBits());

  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, cubeMap.top.width(), cubeMap.top.height(), 0, GL_BGRA,
               GL_UNSIGNED_BYTE, cubeMap.top.constBits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, cubeMap.bottom.width(), cubeMap.bottom.height(), 0, GL_BGRA,
               GL_UNSIGNED_BYTE, cubeMap.bottom.constBits());

  glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, cubeMap.back.width(), cubeMap.back.height(), 0, GL_BGRA,
               GL_UNSIGNED_BYTE, cubeMap.back.constBits());
  glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, cubeMap.front.width(), cubeMap.front.height(), 0, GL_BGRA,
               GL_UNSIGNED_BYTE, cubeMap.front.constBits());

  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  return id;
}

texture_id TextureCache::loadInternal(const std::string &path, GLint filter, GLint repeat) {
  QImage image{QString::fromStdString(path)};
  if (image.isNull())
    return fallbackTexture;

  unsigned int glFormat;
  switch (image.format()) {
  case QImage::Format::Format_RGB32:
    glFormat = GL_RGB;
    break;
  case QImage::Format::Format_ARGB32:
    glFormat = GL_RGBA;
    break;
  default:
    qDebug() << "Unsupported format: " << image.format() << "\nAttempting conversion...";
    image = image.convertToFormat(QImage::Format_ARGB32);
    glFormat = GL_RGBA;

    if (image.isNull())
      return fallbackTexture;
  }

  Texture t;
  t.height = image.height();
  t.width = image.width();

  glGenTextures(1, &t.id);
  glBindTexture(GL_TEXTURE_2D, t.id);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);

  // QImage keeps BGRA format, event without an alpha channel
  glTexImage2D(GL_TEXTURE_2D, 0, glFormat, t.width, t.height, 0, GL_BGRA, GL_UNSIGNED_BYTE, image.constBits());

  glGenerateMipmap(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, 0u);

  textures.emplace_back(t);
  return textures.size() - 1u;
}

const Texture &TextureCache::get(texture_id index) {
  return textures[index];
}

void TextureCache::clear() {
  for (const auto &t : textures) {
    glDeleteTextures(1, &t.id);
  }

  textures.clear();
  indexMap.clear();
}

void TextureCache::use(texture_id index) {
  const auto &t = textures[index];
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, t.id);
}

void TextureCache::useCubeMap(unsigned int id) {
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_CUBE_MAP, id);
}

texture_id TextureCache::getFallbackTexture() const {
  return fallbackTexture;
}

} // namespace netsimulyzer
