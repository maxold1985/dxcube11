#pragma once

#include <windows.h>
#include <d3d11.h>

#include "math3d.h"
#include "model_loader.h"

class Renderer
{
public:
    Renderer();

    bool Initialize(HWND window, int width, int height);
    void Render(float deltaTime);
    void Shutdown();

private:
    struct Vertex
    {
        float x, y, z;
        float u, v;
    };

    enum TransformMode
    {
        TRANSFORM_MOVE = 0,
        TRANSFORM_ROTATE = 1,
        TRANSFORM_SCALE = 2
    };

    bool LoadShaderFile(const char* filename, unsigned char** data, unsigned long* size);
    bool CreateShaders();
    bool CreateCube();
    bool CreateModelBuffers(const ModelData& model);
    void UpdateMouseTransform();
    void DrawTransformPanel();

private:
    HWND windowHandle;
    IDXGISwapChain* swapChain;
    ID3D11Device* device;
    ID3D11DeviceContext* context;
    ID3D11RenderTargetView* renderTarget;
    ID3D11VertexShader* vertexShader;
    ID3D11PixelShader* pixelShader;
    ID3D11InputLayout* inputLayout;
    ID3D11Buffer* vertexBuffer;
    ID3D11Buffer* indexBuffer;
    ID3D11Buffer* constantBuffer;
    unsigned int indexCount;
    bool modelUses32BitIndices;
    D3D11_VIEWPORT viewport;
    float aspect;

    Vec3 objectPosition;
    Vec3 objectRotation;
    Vec3 objectScale;
    TransformMode transformMode;
    float moveSensitivity;
    float rotateSensitivity;
    float scaleSensitivity;
};
