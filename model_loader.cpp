#include "model_loader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/material.h>

static std::string GetDirectory(const char* filename)
{
    std::string path = filename ? filename : "";
    std::string::size_type p = path.find_last_of("/\\");
    return p == std::string::npos ? "" : path.substr(0, p + 1);
}

bool LoadModelAssimp(const char* filename, ModelData& outModel, std::string& outError)
{
    outModel.vertices.clear();
    outModel.indices.clear();
    outModel.diffuseTexture.clear();
    outModel.diffuseColor[0]=1.0f; outModel.diffuseColor[1]=1.0f;
    outModel.diffuseColor[2]=1.0f; outModel.diffuseColor[3]=1.0f;
    outError.clear();

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filename,
        aiProcess_Triangulate |
        aiProcess_JoinIdenticalVertices |
        aiProcess_ImproveCacheLocality |
        aiProcess_GenSmoothNormals |
        aiProcess_ConvertToLeftHanded);

    if (scene == NULL || scene->mRootNode == NULL)
    {
        outError = importer.GetErrorString();
        return false;
    }

    unsigned int totalVertices=0, totalIndices=0;
    for (unsigned int m=0; m<scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh=scene->mMeshes[m];
        totalVertices += mesh->mNumVertices;
        totalIndices += mesh->mNumFaces * 3;
    }
    outModel.vertices.reserve(totalVertices);
    outModel.indices.reserve(totalIndices);

    bool materialRead=false;
    const std::string directory=GetDirectory(filename);

    for (unsigned int m=0; m<scene->mNumMeshes; ++m)
    {
        const aiMesh* mesh=scene->mMeshes[m];
        const unsigned int baseVertex=static_cast<unsigned int>(outModel.vertices.size());

        if (!materialRead && mesh->mMaterialIndex < scene->mNumMaterials)
        {
            aiMaterial* material=scene->mMaterials[mesh->mMaterialIndex];
            aiColor4D color;
            if (AI_SUCCESS == aiGetMaterialColor(material, AI_MATKEY_COLOR_DIFFUSE, &color))
            {
                outModel.diffuseColor[0]=color.r; outModel.diffuseColor[1]=color.g;
                outModel.diffuseColor[2]=color.b; outModel.diffuseColor[3]=color.a;
            }

            aiString texturePath;
            if (AI_SUCCESS == material->GetTexture(aiTextureType_DIFFUSE, 0, &texturePath))
            {
                const char* p=texturePath.C_Str();
                // Embedded textures (*0 etc.) need a dedicated decoder; external textures are supported here.
                if (p != NULL && p[0] != '*')
                    outModel.diffuseTexture = directory + p;
            }
            materialRead=true;
        }

        for (unsigned int i=0; i<mesh->mNumVertices; ++i)
        {
            ModelVertex vertex;
            vertex.x=mesh->mVertices[i].x; vertex.y=mesh->mVertices[i].y; vertex.z=mesh->mVertices[i].z;
            if (mesh->HasTextureCoords(0))
            {
                vertex.u=mesh->mTextureCoords[0][i].x;
                vertex.v=mesh->mTextureCoords[0][i].y;
            }
            else { vertex.u=0.0f; vertex.v=0.0f; }
            outModel.vertices.push_back(vertex);
        }

        for (unsigned int f=0; f<mesh->mNumFaces; ++f)
        {
            const aiFace& face=mesh->mFaces[f];
            if (face.mNumIndices != 3) continue;
            outModel.indices.push_back(baseVertex+face.mIndices[0]);
            outModel.indices.push_back(baseVertex+face.mIndices[1]);
            outModel.indices.push_back(baseVertex+face.mIndices[2]);
        }
    }

    if (outModel.vertices.empty() || outModel.indices.empty())
    {
        outError="Assimp carregou o arquivo, mas nao encontrou triangulos.";
        return false;
    }
    return true;
}
