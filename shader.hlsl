cbuffer TransformBuffer : register(b0) { row_major float4x4 mvp; };
cbuffer MaterialBuffer : register(b1)
{
    float4 materialColor;
    int useTexture;
    float3 materialPadding;
};

Texture2D diffuseTexture : register(t0);
SamplerState diffuseSampler : register(s0);

struct VertexInput { float3 position : POSITION; float2 uv : TEXCOORD0; };
struct PixelInput { float4 position : SV_POSITION; float2 uv : TEXCOORD0; };

PixelInput VSMain(VertexInput input)
{
    PixelInput output;
    output.position = mul(float4(input.position, 1.0f), mvp);
    output.uv = input.uv;
    return output;
}

float4 PSMain(PixelInput input) : SV_TARGET
{
    float4 base = materialColor;
    if (useTexture != 0)
        base *= diffuseTexture.Sample(diffuseSampler, input.uv);
    return base;
}
