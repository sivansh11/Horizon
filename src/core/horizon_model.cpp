#include "core/horizon_model.h"


namespace horizon {

Model::~Model() {
    
} 

void Model::init(gfx::Device& deviceRef, std::string path) {
    mDevice = &deviceRef;
    loadModelFromPath(path);
}

void Model::free() {
    for (auto& mesh: mMeshes) {
        mesh->free();
    }
}

void Model::loadModelFromPath(std::string& path) {
    mFilePath = path;

    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(path,
                                            //  aiProcess_Triangulate |
                                             aiProcess_FlipUVs     |
                                             aiProcess_GenNormals );
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        throw std::runtime_error(std::string("Assimp Error!\n") + importer.GetErrorString());
    }

    mDirectory = path.substr(0, path.find_last_of('/'));
    aiMatrix4x4 transform{};
    processNode(scene->mRootNode, scene, transform);
}

void Model::processNode(aiNode *node, const aiScene *scene, aiMatrix4x4 &transform) {
    aiMatrix4x4 localTransform = transform * node->mTransformation;
    for (uint32_t i = 0; i < node->mNumChildren; i++) {
        processNode(node->mChildren[i], scene, localTransform);
    }
    for (uint32_t i = 0; i < node->mNumMeshes; i++) {
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];
        mMeshes.push_back(processMesh(mesh, scene, localTransform));
    }
}

std::unique_ptr<Mesh> Model::processMesh(aiMesh *mesh, const aiScene *scene, aiMatrix4x4 &transform) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;

    vertices.reserve(mesh->mNumVertices);
    for (uint32_t i = 0; i < mesh->mNumVertices; i++) {
        Vertex vertex;

        vertex.position.x = mesh->mVertices[i].x;
        vertex.position.y = mesh->mVertices[i].y;
        vertex.position.z = mesh->mVertices[i].z;

        if (mesh->HasNormals()) {
            vertex.normal.x = mesh->mNormals[i].x;
            vertex.normal.y = mesh->mNormals[i].y;
            vertex.normal.z = mesh->mNormals[i].z;
        }
        if (mesh->mTextureCoords[0]) {
            vertex.uv.x = mesh->mTextureCoords[0][i].x;
            vertex.uv.y = mesh->mTextureCoords[0][i].y;
        } else {
            vertex.uv = {0, 0};
        }

        vertex.color = {1, 1, 1};

        vertices.push_back(vertex); 
    }

    for (uint32_t i = 0; i < mesh->mNumFaces; i++) {
        aiFace &face = mesh->mFaces[i];
        for (uint32_t j = 0; j < face.mNumIndices; j++) {
            indices.push_back(face.mIndices[j]);
        }
    }

    aiMaterial *assimpMat = scene->mMaterials[mesh->mMaterialIndex];
    Material horizonmaterial = processMaterial(assimpMat);
    std::unique_ptr<Mesh> horizonMesh = std::make_unique<Mesh>(*mDevice, &vertices, &indices, horizonmaterial);
    transform.Decompose(*(aiVector3D*)(&horizonMesh->getTransform().scale), *(aiVector3D*)(&horizonMesh->getTransform().rotation), *(aiVector3D*)(&horizonMesh->getTransform().translation));
    return horizonMesh;
}

Material Model::processMaterial(aiMaterial *mat) {
    return Material{};
}

void Model::draw(VkCommandBuffer commandBuffer) {
    for (auto& mesh : mMeshes) {
        mesh->bind(commandBuffer);
        mesh->draw(commandBuffer);
    }
}

} // namespace horizon
