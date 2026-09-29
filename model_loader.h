#pragma once

#include <string>
#include <vector>

struct ModelVertex
{
    float x, y, z;
    float u, v;
};

struct ModelData
{
    std::vector<ModelVertex> vertices;
    std::vector<unsigned int> indices;
};

// Assimp escolhe o importer pelo arquivo: OBJ, FBX, GLB/GLTF, etc.
bool LoadModelAssimp(const char* filename, ModelData& outModel, std::string& outError);
