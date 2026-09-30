#include "renderer.h"
#include <fstream>
#include <cstdio>
#include <wincodec.h>
#include "nanovg.h"
#include "nanovg_gl.h"
#pragma comment(lib, "windowscodecs.lib")
static NVGcontext* gNanoVG = NULL;
static NanoVGDX11Overlay gNanoVGOverlay;
Renderer::Renderer()
{
    windowHandle = NULL;
    swapChain = NULL;
    device = NULL;
    context = NULL;
    renderTarget = NULL;
    vertexShader = NULL;
    pixelShader = NULL;
    inputLayout = NULL;
    vertexBuffer = NULL;
    indexBuffer = NULL;
    constantBuffer = NULL;
    materialBuffer = NULL;
    diffuseTexture = NULL;
    samplerState = NULL;
    indexCount = 0;
    modelUses32BitIndices = false;
    materialHasTexture = false;
    aspect = 1.0f;
    materialColor[0] = materialColor[1] = materialColor[2] = materialColor[3] = 1.0f;
    objectPosition = Vec3 {
        0, 0, 0
    };
    objectRotation = Vec3 {
        0, 0, 0
    };
    objectScale = Vec3 {
        1, 1, 1
    };
    transformMode = TRANSFORM_MOVE;
    moveSensitivity = 0.01f;
    rotateSensitivity = 0.01f;
    scaleSensitivity = 0.01f;
    ZeroMemory(&viewport, sizeof(viewport));
}
bool Renderer::LoadShaderFile(const char* filename, unsigned char** data, unsigned long* size)
{
    std::ifstream file(filename, std::ios::binary|std::ios::ate);
    if (!file) return false;
    std::streamoff fileSize = file.tellg();
    if (fileSize<= 0) return false;
    file.seekg(0, std::ios::beg);
    *size = static_cast<unsigned long>(fileSize);
    *data = new unsigned char[*size];
    file.read(reinterpret_cast<char*>(*data), *size);
    if (!file) {
        delete[] *data;
        *data = NULL;
        *size = 0;
        return false;
    }
    return true;
}
bool Renderer::Initialize(HWND window, int width, int height)
{
    windowHandle = window;
    aspect = static_cast<float>(width)/height;
    DXGI_SWAP_CHAIN_DESC sd = {
    };
    sd.BufferCount = 1;
    sd.BufferDesc.Width = width;
    sd.BufferDesc.Height = height;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = window;
    sd.SampleDesc.Count = 1;
    sd.Windowed = TRUE;
    D3D_FEATURE_LEVEL fl = D3D_FEATURE_LEVEL_11_0;
    HRESULT hr = D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 0, &fl, 1, D3D11_SDK_VERSION,
    &sd, &swapChain, &device, NULL, &context);
    if (FAILED(hr)) return false;
    ID3D11Texture2D* back = NULL;
    hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<void**>(&back));
    if (FAILED(hr)) return false;
    hr = device->CreateRenderTargetView(back, NULL, &renderTarget);
    back->Release();
    if (FAILED(hr)) return false;
    viewport.TopLeftX = 0;
    viewport.TopLeftY = 0;
    viewport.Width = (float)width;
    viewport.Height = (float)height;
    viewport.MinDepth = 0;
    viewport.MaxDepth = 1;
    if (!CreateShaders()) return false;
    ModelData model;
    std::string error;
    if (LoadModelAssimp("model.glb", model, error) || LoadModelAssimp("model.fbx", model, error) || LoadModelAssimp("model.obj", model, error))
    {
        if (!CreateModelBuffers(model) || !CreateMaterialResources(model)) return false;
    }
    else
    {
        if (!CreateCube()) return false;
        ModelData fallback;
        if (!CreateMaterialResources(fallback)) return false;
    }
    if (!gNanoVGOverlay.Initialize(windowHandle, width, height)) return false;
    gNanoVG = gNanoVGOverlay.GetContext();
    return gNanoVG!= NULL;
}
bool Renderer::CreateShaders()
{
    unsigned char *vs = NULL, *ps = NULL;
    unsigned long vsSize = 0, psSize = 0;
    if (!LoadShaderFile("vertex.cso", &vs, &vsSize)) return false;
    if (!LoadShaderFile("pixel.cso", &ps, &psSize)) {
        delete[] vs;
        return false;
    }
    HRESULT hr = device->CreateVertexShader(vs, vsSize, NULL, &vertexShader);
    if (SUCCEEDED(hr)) hr = device->CreatePixelShader(ps, psSize, NULL, &pixelShader);
    D3D11_INPUT_ELEMENT_DESC e[2] = {
    };
    e[0].SemanticName = "POSITION";
    e[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    e[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    e[1].SemanticName = "TEXCOORD";
    e[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    e[1].AlignedByteOffset = 12;
    e[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
    if (SUCCEEDED(hr)) hr = device->CreateInputLayout(e, 2, vs, vsSize, &inputLayout);
    delete[] vs;
    delete[] ps;
    return SUCCEEDED(hr);
}
bool Renderer::CreateCube()
{
    Vertex v[] = {
        {
            -1, -1, -1, 0, 1
        }
        , {
            -1, 1, -1, 0, 0
        }
        , {
            1, 1, -1, 1, 0
        }
        , {
            1, -1, -1, 1, 1
        }
        ,
        {
            -1, -1, 1, 0, 1
        }
        , {
            -1, 1, 1, 0, 0
        }
        , {
            1, 1, 1, 1, 0
        }
        , {
            1, -1, 1, 1, 1
        }
    };
    unsigned short i[] = {
        0, 1, 2, 0, 2, 3, 4, 6, 5, 4, 7, 6, 0, 4, 5, 0, 5, 1, 3, 2, 6, 3, 6, 7, 1, 5, 6, 1, 6, 2, 0, 3, 7, 0, 7, 4
    };
    D3D11_BUFFER_DESC bd = {
    };
    D3D11_SUBRESOURCE_DATA data = {
    };
    bd.ByteWidth = sizeof(v);
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    data.pSysMem = v;
    if (FAILED(device->CreateBuffer(&bd, &data, &vertexBuffer))) return false;
    bd.ByteWidth = sizeof(i);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    data.pSysMem = i;
    if (FAILED(device->CreateBuffer(&bd, &data, &indexBuffer))) return false;
    bd.ByteWidth = sizeof(Mat4);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(device->CreateBuffer(&bd, NULL, &constantBuffer))) return false;
    indexCount = 36;
    modelUses32BitIndices = false;
    return true;
}
bool Renderer::CreateModelBuffers(const ModelData& model)
{
    if (model.vertices.empty() || model.indices.empty()) return false;
    D3D11_BUFFER_DESC bd = {
    };
    D3D11_SUBRESOURCE_DATA data = {
    };
    bd.ByteWidth = (UINT)(model.vertices.size()*sizeof(ModelVertex));
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    data.pSysMem = &model.vertices[0];
    if (FAILED(device->CreateBuffer(&bd, &data, &vertexBuffer))) return false;
    bd.ByteWidth = (UINT)(model.indices.size()*sizeof(unsigned int));
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    data.pSysMem = &model.indices[0];
    if (FAILED(device->CreateBuffer(&bd, &data, &indexBuffer))) return false;
    bd.ByteWidth = sizeof(Mat4);
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(device->CreateBuffer(&bd, NULL, &constantBuffer))) return false;
    indexCount = (unsigned int)model.indices.size();
    modelUses32BitIndices = true;
    return true;
}
bool Renderer::CreateTextureFromFile(const char* filename)
{
    if (!filename || !*filename) return false;
    int len = MultiByteToWideChar(CP_UTF8, 0, filename, -1, NULL, 0);
    if (len<= 0) return false;
    wchar_t* wide = new wchar_t[len];
    MultiByteToWideChar(CP_UTF8, 0, filename, -1, wide, len);
    IWICImagingFactory* factory = NULL;
    IWICBitmapDecoder* decoder = NULL;
    IWICBitmapFrameDecode* frame = NULL;
    IWICFormatConverter* converter = NULL;
    HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&factory));
    if (SUCCEEDED(hr)) hr = factory->CreateDecoderFromFilename(wide, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &decoder);
    delete[] wide;
    if (SUCCEEDED(hr)) hr = decoder->GetFrame(0, &frame);
    if (SUCCEEDED(hr)) hr = factory->CreateFormatConverter(&converter);
    if (SUCCEEDED(hr)) hr = converter->Initialize(frame, GUID_WICPixelFormat32bppRGBA, WICBitmapDitherTypeNone, NULL, 0, WICBitmapPaletteTypeCustom);
    UINT w = 0, h = 0;
    if (SUCCEEDED(hr)) hr = converter->GetSize(&w, &h);
    unsigned char* pixels = NULL;
    if (SUCCEEDED(hr) && w && h) {
        pixels = new unsigned char[w*h*4];
        hr = converter->CopyPixels(NULL, w*4, w*h*4, pixels);
    }
    if (SUCCEEDED(hr))
    {
        D3D11_TEXTURE2D_DESC td = {
        };
        td.Width = w;
        td.Height = h;
        td.MipLevels = 1;
        td.ArraySize = 1;
        td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        td.SampleDesc.Count = 1;
        td.Usage = D3D11_USAGE_DEFAULT;
        td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        D3D11_SUBRESOURCE_DATA sd = {
        };
        sd.pSysMem = pixels;
        sd.SysMemPitch = w*4;
        ID3D11Texture2D* texture = NULL;
        hr = device->CreateTexture2D(&td, &sd, &texture);
        if (SUCCEEDED(hr)) {
            hr = device->CreateShaderResourceView(texture, NULL, &diffuseTexture);
            texture->Release();
        }
    }
    delete[] pixels;
    if (converter)converter->Release();
    if (frame)frame->Release();
    if (decoder)decoder->Release();
    if (factory)factory->Release();
    return SUCCEEDED(hr) && diffuseTexture!= NULL;
}
bool Renderer::CreateFallbackTexture()
{
    const unsigned int white = 0xffffffff;
    D3D11_TEXTURE2D_DESC td = {
    };
    td.Width = 1;
    td.Height = 1;
    td.MipLevels = 1;
    td.ArraySize = 1;
    td.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    td.SampleDesc.Count = 1;
    td.Usage = D3D11_USAGE_DEFAULT;
    td.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    D3D11_SUBRESOURCE_DATA sd = {
    };
    sd.pSysMem = &white;
    sd.SysMemPitch = 4;
    ID3D11Texture2D* tex = NULL;
    HRESULT hr = device->CreateTexture2D(&td, &sd, &tex);
    if (SUCCEEDED(hr)) {
        hr = device->CreateShaderResourceView(tex, NULL, &diffuseTexture);
        tex->Release();
    }
    return SUCCEEDED(hr);
}
bool Renderer::CreateMaterialResources(const ModelData& model)
{
    for (int n = 0;n<4;++n) materialColor[n] = model.diffuseColor[n];
    materialHasTexture = !model.diffuseTexture.empty() && CreateTextureFromFile(model.diffuseTexture.c_str());
    if (!diffuseTexture && !CreateFallbackTexture()) return false;
    D3D11_BUFFER_DESC bd = {
    };
    bd.ByteWidth = sizeof(MaterialBuffer);
    bd.Usage = D3D11_USAGE_DEFAULT;
    bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    if (FAILED(device->CreateBuffer(&bd, NULL, &materialBuffer))) return false;
    D3D11_SAMPLER_DESC sd = {
    };
    sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    sd.MaxLOD = D3D11_FLOAT32_MAX;
    return SUCCEEDED(device->CreateSamplerState(&sd, &samplerState));
}
void Renderer::UpdateMouseTransform()
{
    static POINT last = {
        0, 0
    };
    static bool valid = false;
    POINT p;
    GetCursorPos(&p);
    ScreenToClient(windowHandle, &p);
    if (!(GetAsyncKeyState(VK_LBUTTON)&0x8000)) {
        valid = false;
        return;
    }
    if (!valid) {
        last = p;
        valid = true;
        return;
    }
    float dx = (float)(p.x-last.x), dy = (float)(p.y-last.y);
    last = p;
    if (transformMode == TRANSFORM_MOVE) {
        objectPosition.x+ = dx*moveSensitivity;
        objectPosition.y- = dy*moveSensitivity;
    }
    else if (transformMode == TRANSFORM_ROTATE) {
        objectRotation.y+ = dx*rotateSensitivity;
        objectRotation.x+ = dy*rotateSensitivity;
    }
    else {
        float d = (dx-dy)*scaleSensitivity;
        objectScale.x+ = d;
        objectScale.y+ = d;
        objectScale.z+ = d;
        if (objectScale.x<.01f)objectScale.x = .01f;
        if (objectScale.y<.01f)objectScale.y = .01f;
        if (objectScale.z<.01f)objectScale.z = .01f;
    }
}
void Renderer::DrawTransformPanel()
{
    if (!gNanoVG)return;
    char line[256];
    nvgBeginPath(gNanoVG);
    nvgRoundedRect(gNanoVG, 15, 15, 340, 205, 6);
    nvgFillColor(gNanoVG, nvgRGBA(30, 32, 38, 225));
    nvgFill(gNanoVG);
    nvgFontSize(gNanoVG, 18);
    nvgFillColor(gNanoVG, nvgRGBA(240, 240, 245, 255));
    nvgText(gNanoVG, 30, 45, "DX11Cube Transform", NULL);
    sprintf_s(line, sizeof(line), "Mode: %s", transformMode == TRANSFORM_MOVE?"Move [W]":transformMode == TRANSFORM_ROTATE?"Rotate [E]":"Scale [R]");
    nvgText(gNanoVG, 30, 75, line, NULL);
    sprintf_s(line, sizeof(line), "Position: %.2f %.2f %.2f", objectPosition.x, objectPosition.y, objectPosition.z);
    nvgText(gNanoVG, 30, 105, line, NULL);
    sprintf_s(line, sizeof(line), "Rotation: %.2f %.2f %.2f", objectRotation.x, objectRotation.y, objectRotation.z);
    nvgText(gNanoVG, 30, 135, line, NULL);
    sprintf_s(line, sizeof(line), "Scale: %.2f %.2f %.2f", objectScale.x, objectScale.y, objectScale.z);
    nvgText(gNanoVG, 30, 165, line, NULL);
}
void Renderer::Render(float deltaTime)
{
    UNREFERENCED_PARAMETER(deltaTime);
    if (GetAsyncKeyState('W')&1)transformMode = TRANSFORM_MOVE;
    if (GetAsyncKeyState('E')&1)transformMode = TRANSFORM_ROTATE;
    if (GetAsyncKeyState('R')&1)transformMode = TRANSFORM_SCALE;
    UpdateMouseTransform();
    float clear[4] = {
        .10f, .12f, .15f, 1
    };
    context->ClearRenderTargetView(renderTarget, clear);
    context->OMSetRenderTargets(1, &renderTarget, NULL);
    context->RSSetViewports(1, &viewport);
    UINT stride = sizeof(Vertex), offset = 0;
    context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
    context->IASetIndexBuffer(indexBuffer, modelUses32BitIndices?DXGI_FORMAT_R32_UINT:DXGI_FORMAT_R16_UINT, 0);
    context->IASetInputLayout(inputLayout);
    context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Mat4 world = MatrixMultiply(MatrixMultiply(MatrixScale(objectScale.x, objectScale.y, objectScale.z), MatrixRotationXYZ(objectRotation.x, objectRotation.y, objectRotation.z)), MatrixTranslation(objectPosition.x, objectPosition.y, objectPosition.z));
    Vec3 eye = {
        0, 2, -6
    }
    , target = {
        0, 0, 0
    }
    , up = {
        0, 1, 0
    };
    Mat4 mvp = MatrixMultiply(MatrixMultiply(world, MatrixLookAtLH(eye, target, up)), MatrixPerspectiveFovLH(70.0f*3.14159265358979323846f/180.0f, aspect, .1f, 100));
    context->UpdateSubresource(constantBuffer, 0, NULL, &mvp, 0, 0);
    MaterialBuffer mb = {
        {
            materialColor[0], materialColor[1], materialColor[2], materialColor[3]
        }
        , materialHasTexture?1:0, {
            0, 0, 0
        }
    };
    context->UpdateSubresource(materialBuffer, 0, NULL, &mb, 0, 0);
    context->VSSetShader(vertexShader, NULL, 0);
    context->VSSetConstantBuffers(0, 1, &constantBuffer);
    context->PSSetShader(pixelShader, NULL, 0);
    context->PSSetConstantBuffers(1, 1, &materialBuffer);
    context->PSSetShaderResources(0, 1, &diffuseTexture);
    context->PSSetSamplers(0, 1, &samplerState);
    context->DrawIndexed(indexCount, 0, 0);
    gNanoVGOverlay.BeginFrame();
    DrawTransformPanel();
    gNanoVGOverlay.EndFrame();
    swapChain->Present(1, 0);
}
void Renderer::Shutdown()
{
    gNanoVGOverlay.Shutdown();
    gNanoVG = NULL;
    if (context)context->ClearState();
    if (samplerState) {
        samplerState->Release();
        samplerState = NULL;
    }
    if (diffuseTexture) {
        diffuseTexture->Release();
        diffuseTexture = NULL;
    }
    if (materialBuffer) {
        materialBuffer->Release();
        materialBuffer = NULL;
    }
    if (constantBuffer) {
        constantBuffer->Release();
        constantBuffer = NULL;
    }
    if (indexBuffer) {
        indexBuffer->Release();
        indexBuffer = NULL;
    }
    if (vertexBuffer) {
        vertexBuffer->Release();
        vertexBuffer = NULL;
    }
    if (inputLayout) {
        inputLayout->Release();
        inputLayout = NULL;
    }
    if (pixelShader) {
        pixelShader->Release();
        pixelShader = NULL;
    }
    if (vertexShader) {
        vertexShader->Release();
        vertexShader = NULL;
    }
    if (renderTarget) {
        renderTarget->Release();
        renderTarget = NULL;
    }
    if (swapChain) {
        swapChain->Release();
        swapChain = NULL;
    }
    if (context) {
        context->Release();
        context = NULL;
    }
    if (device) {
        device->Release();
        device = NULL;
    }
}
