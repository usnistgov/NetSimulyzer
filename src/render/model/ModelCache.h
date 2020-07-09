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
#include "../mesh/Mesh.h"
#include "../shader/Shader.h"
#include "../texture/TextureCache.h"
#include "../texture/texture.h"
#include "Model.h"
#include <QOpenGLFunctions_3_3_Core>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <glm/glm.hpp>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace visualization {

class ModelRenderInfo : protected QOpenGLFunctions_3_3_Core {
public:
  struct ModelRenderBounds {
    glm::vec3 min{0.0f};
    glm::vec3 max{0.0f};
  };

private:
  std::vector<Mesh> meshes;
  std::vector<Mesh> transparentMeshes;
  TextureCache &textureCache;
  std::vector<Material> materials;
  ModelRenderBounds bounds;

  void updateBounds();

  void loadNode(aiNode const *node, aiScene const *scene);
  void loadMesh(aiMesh const *m, aiScene const *scene);
  void loadMaterials(aiScene const *scene);

public:
  ~ModelRenderInfo() override;
  ModelRenderInfo(aiScene const *scene, TextureCache &textureCache);
  ModelRenderInfo(std::vector<Mesh> meshes, TextureCache &textureCache);

  // Allow Moves
  ModelRenderInfo(ModelRenderInfo &&other) noexcept
      : meshes(std::move(other.meshes)), transparentMeshes(std::move(other.transparentMeshes)),
        textureCache(other.textureCache), materials(std::move(other.materials)) {
    bounds = other.bounds;
  };

  // We cannot allow the assignment operator since we have a reference member
  ModelRenderInfo &operator=(ModelRenderInfo &&other) = delete;

  // Disallow copies
  ModelRenderInfo(const ModelRenderInfo &) = delete;
  ModelRenderInfo &operator=(const ModelRenderInfo &) = delete;

  [[nodiscard]] const ModelRenderBounds &getBounds() const;
  [[nodiscard]] bool hasTransparentMeshes() const;

  void render(Shader &s);
  void renderTransparent(Shader &s);
  void clear();
};

class ModelCache : protected QOpenGLFunctions_3_3_Core {
  std::unordered_map<std::string, std::size_t> indexMap;
  std::vector<ModelRenderInfo> models;
  TextureCache &textureCache;
  std::string basePath;

public:
  const model_id fallbackModel = 0u;
  explicit ModelCache(TextureCache &textureCache);
  ~ModelCache() override;

  void setBasePath(std::string value);
  void init(const std::string &fallbackModelPath);
  Model::ModelLoadInfo load(const std::string &path);
  ModelRenderInfo &get(model_id index);

  ModelRenderInfo &operator[](model_id index) {
    return get(index);
  }
  void clear();
  void render(model_id index, Shader &s);
};

} // namespace visualization
