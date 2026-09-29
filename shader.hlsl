cbuffer TransformBuffer : register(b0)
{
    row_major float4x4 mvp;
};

struct VertexInput
{
    float3 position : POSITION;
    float2 uv       : TEXCOORD0;
};

struct PixelInput
{
    float4 position : SV_POSITION;
    float2 uv       : TEXCOORD0;
};

PixelInput VSMain(
    VertexInput input
)
{
    PixelInput output;

    output.position =
        mul(
            float4(
                input.position,
                1.0f
            ),
            mvp
        );

    output.uv =
        input.uv;

    return output;
}

float4 PSMain(
    PixelInput input
) : SV_TARGET
{
    return float4(
        input.uv.x,
        input.uv.y,
        1.0f,
        1.0f
    );
}
