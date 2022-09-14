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

#include "FontManager.h"
#include "src/render/font/undefined-medium-font.h"
#include <algorithm>
#include <iostream>

namespace netsimulyzer {

// Atlas texture/width/height initialized in `init`
// where we actually can load them
FontManager::FontManager(TextureCache &textureCache) // NOLINT(cppcoreguidelines-pro-type-member-init)
    : textureCache(textureCache) {
}

FontManager::~FontManager() {
  reset();
}

void FontManager::init(const std::string &atlasFilePath) {
  if (!gl.initializeOpenGLFunctions()) {
    std::cerr << "Failed to initialize FontManager OpenGL Functions!\n";
    std::abort();
  }

  // Use `GL_NEAREST` filtering, since this is a
  // pixelated font
  atlasTexture = textureCache.loadInternal(atlasFilePath, GL_NEAREST);
  const auto &t = textureCache.get(atlasTexture);

  atlasWidth = static_cast<float>(t.width);
  atlasHeight = static_cast<float>(t.height);
}

void FontManager::reset() {
  for (const auto &font : createdFontMeshes) {
    gl.glDeleteBuffers(1, &font.glyphVbo);
    gl.glDeleteBuffers(1, &font.backgroundVbo);

    gl.glDeleteVertexArrays(1, &font.glyphVao);
    gl.glDeleteVertexArrays(1, &font.backgroundVao);
  }

  createdFontMeshes.clear();
}

FontManager::FontBannerRenderInfo FontManager::allocate(std::string_view text) {
  textureCache.use(atlasTexture);
  FontBannerRenderInfo renderInfo; // NOLINT(cppcoreguidelines-pro-type-member-init)

  renderInfo.size = static_cast<int>(text.size());
  renderInfo.glyphVboSize = renderInfo.size * 6; // 3 Vertices per triangle, 2 triangles per glyph
  renderInfo.backgroundVboSize = 3 * 2;          // 3 Vertices per triangle, 2 triangles in the rectangle

  // ----- Glyphs -----
  // TODO: Make configurable
  // Scale factor for the font + background
  // smaller, since we render the font for the
  // texture at a large size, so it looks okay
  // even if it gets big
  float scale = 0.05f;

  // All ASCII characters use the same advance value,
  // so we can use one value to produce an estimate for the
  // whole string
  // TODO: This seems slightly off center, maybe adjust
  const auto estimatedAdvance = 32.0f * scale;
  const auto startX = -1.0f * (estimatedAdvance * text.size()) / 2.0f;

  gl.glGenVertexArrays(1, &renderInfo.glyphVao);
  gl.glGenBuffers(1, &renderInfo.glyphVbo);

  gl.glBindVertexArray(renderInfo.glyphVao);
  gl.glBindBuffer(GL_ARRAY_BUFFER, renderInfo.glyphVbo);

  gl.glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4 * text.size(), nullptr, GL_STATIC_DRAW);

  // Location
  gl.glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
  gl.glEnableVertexAttribArray(0);

  // Loop through each character in the string,
  // calculate the size, offsets, etc. for each glyph
  // then, add the mesh for the glyph to `renderInfo.glyphVbo`
  float maxX = 0.0f; // Max X/Y for the borders of the background
  float maxY = 0.0f;
  float minY = 0.0f;

  // Running location of the next glyph
  // Starting with `startX`, since we want to start
  // Halfway to the left, to center the text
  float x = startX;
  float y = 0.0f;
  // Index of the glyph
  auto i = 0;
  for (const auto &c : text) {
    // use `at()` since there's no const `[]`
    const auto &ch = undefined_medium::fontGlyphs.at(c);

    // Starting position of the character
    const float positionX = x + ch.offset.x * scale;
    const float positionY = y + ch.offset.y * scale;

    float characterWidth = ch.size.x * scale;
    float characterHeight = ch.size.y * scale;

    maxX = std::max(maxX, positionX + characterWidth);
    maxY = std::max(maxY, positionY + characterHeight);
    minY = std::min(minY, positionY);

    // texture coordinates of the character
    // on the atlas.
    // The coordinates in the character are in pixels,
    // so divide by the `atlasWidth` to get them in
    // texture coordinates
    const auto lowX = ch.x / atlasWidth;
    const auto lowY = ch.y / atlasHeight;
    const auto highX = (ch.x + ch.size.x) / atlasWidth;
    const auto highY = (ch.y + ch.size.y) / atlasHeight;

    // clang-format off
    float vertices[6][4] ={
         {positionX,                  positionY + characterHeight,   lowX, lowY},
         {positionX,                  positionY,                     lowX, highY},
         {positionX + characterWidth, positionY,                     highX, highY},

         {positionX,                  positionY + characterHeight,  lowX, lowY},
         {positionX + characterWidth, positionY,                    highX, highY},
         {positionX + characterWidth, positionY + characterHeight,  highX, lowY}
        };
    // clang-format on
    x += estimatedAdvance;

    gl.glBufferSubData(GL_ARRAY_BUFFER, sizeof(vertices) * i, sizeof(vertices), vertices);
    i++;
  }

  // ----- Background -----
  // The background is one quad, rendered in black/grey
  // behind the glyphs
  gl.glGenVertexArrays(1, &renderInfo.backgroundVao);
  gl.glGenBuffers(1, &renderInfo.backgroundVbo);
  gl.glBindVertexArray(renderInfo.backgroundVao);
  gl.glBindBuffer(GL_ARRAY_BUFFER, renderInfo.backgroundVbo);

  // Grab the offset for the last character,
  // so we may get the correct right border for the background
  const auto endOffset = undefined_medium::fontGlyphs.at(*(text.end() - 1)).offset.x * scale;

  // Add/Subtract `estimatedAdvance` to give some extra
  // overhang to the background
  // clang-format off
  float bgVertices[6][2] = {
      {startX           - estimatedAdvance, maxY},
      {startX           - estimatedAdvance, minY},
      {maxX + endOffset + estimatedAdvance, minY},

      {startX           - estimatedAdvance, maxY},
      {maxX + endOffset + estimatedAdvance, minY},
      {maxX + endOffset + estimatedAdvance, maxY},
  };
  // clang-format on

  gl.glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 2 * 6, bgVertices, GL_STATIC_DRAW);

  // Location
  gl.glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
  gl.glEnableVertexAttribArray(0);

  gl.glBindBuffer(GL_ARRAY_BUFFER, 0);
  gl.glBindVertexArray(0);

  return renderInfo;
}

texture_id FontManager::getAtlasTexture() const {
  return atlasTexture;
}

} // namespace netsimulyzer
