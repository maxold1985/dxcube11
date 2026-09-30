#pragma once
#include <string>
#include <vector>

struct ModelVertex { float x, y, z; float u, v; };

struct ModelData
{
    std::vector<ModelVertex> vertices;
    std::vector<unsigned int> indices;
    std::string diffuseTexture;
    float diffuseColor[4];
    ModelData()
    {
        diffuseColor[0]=1.0f; diffuseColor[1]=1.0f; diffuseColor[2]=1.0f; diffuseColor[3]=1.0f;
    }
};

bool LoadModelAssimp(const char* filename, ModelData& outModel, std::string& outError);
