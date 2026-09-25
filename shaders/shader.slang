struct VSInput
{
    float3 inPosition;
    float2 inTexCoords;
};

/*
struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
ConstantBuffer<UniformBuffer> ubo;
*/

[[vk::binding(0, 0)]] Sampler2D textures[];

struct VSOutput
{
    float4 pos : SV_Position;
    float2 texCoords;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{
    VSOutput output;
    // output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
    output.pos = float4(input.inPosition, 1.0);
    output.texCoords = input.inTexCoords;
    return output;
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET
{
    return textures[1].Sample(vertIn.texCoords);
}