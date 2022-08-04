#ifndef CORE_HORIZON_MODEL_H
#define CORE_HORIZON_MODEL_H

#include "core/horizon_mesh.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <string>

namespace horizon {

class Model {
public:
    Model() = default;
    ~Model();

    void init(gfx::Device& deviceRef, std::string path);
    void free();

    void draw(VkCommandBuffer commandBuffer);

private:
    void loadModelFromPath(std::string& path);
    void processNode(aiNode *node, const aiScene *scene, aiMatrix4x4 &transform);
    std::unique_ptr<Mesh> processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 &transform);
    Material processMaterial(aiMaterial *assimpMaterial);
    std::shared_ptr<gfx::Texture2D> loadTextures(aiMaterial *mat, aiTextureType type, std::string typeName);

private:
    std::vector<std::unique_ptr<Mesh>> mMeshes;
    std::string mFilePath, mDirectory;
    gfx::Device *mDevice;
};

} // namespace horizon

#endif