struct UniformBuffer {
    float4x4 model;
    float4x4 view;
    float4x4 proj;
};
[[vk::binding(0, 1)]] ConstantBuffer<UniformBuffer> ubo;

struct Material
{
	float4 baseColorFactor;
	uint32_t baseColorTextureIndex;
    uint32_t normalTextureIndex;
    float normalScale;
	float metallicFactor;
	float roughnessFactor;
	uint32_t metallicRoughnessTextureIndex;
};
[[vk::binding(0, 0)]] StructuredBuffer<Material> materials;

[[vk::binding(1, 0)]] Sampler2D textures[];

struct PushData {
    int materialIndex;
	uint32_t baseColorTexcoordIndex;
	uint32_t normalTexcoordIndex;
	uint32_t metallicRoughnessTexcoordIndex;
    bool hasNormals;
    bool hasTangents;
};
[[vk::push_constant]] ConstantBuffer<PushData> pushData;

struct VSInput
{
    float3 inPosition;
    float3 inNormal;
    float3 inTangent;
    float2 inTexCoords[2];
    float3 inColor0;
};

struct VSOutput
{
    float4 pos : SV_Position;
    float3 worldPos;
    float3 normal;
    float2 normalMapUV;
    float3 tangent;
    float2 baseColorUV;
};

[shader("vertex")]
VSOutput vertMain(VSInput input)
{

    VSOutput output;
    output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.inPosition, 1.0))));
    output.worldPos = input.inPosition;
    output.normal = input.inNormal;
    output.normalMapUV = input.inTexCoords[pushData.normalTexcoordIndex];
    output.tangent = input.inTangent;
    output.baseColorUV = input.inTexCoords[pushData.baseColorTexcoordIndex];
    return output;
}

// Christian Schuler's "Normal Mapping Without Precomputed Tangents"
float3x3 cotangentFrame(float3 N, float3 p, float2 uv)
{
    float3 dp1  = ddx(p);
    float3 dp2  = ddy(p);
    float2 duv1 = ddx(uv);
    float2 duv2 = ddy(uv);

    float3 dp2perp = cross(dp2, N);
    float3 dp1perp = cross(N, dp1);
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;

    float invmax = rsqrt(max(dot(T, T), dot(B, B)));
    return float3x3(T * invmax, B * invmax, N);   // rows: T, B, N
}

[shader("fragment")]
float4 fragMain(VSOutput vertIn) : SV_TARGET
{
    float3 flatN = normalize(cross(ddx(vertIn.worldPos), ddy(vertIn.worldPos)));
    float3 N     = vertIn.normal;
    N = (!pushData.hasNormals || dot(N, N) < 1e-8) ? flatN : normalize(N); // Also checks if given normal is degenerate

    float3x3 flatTBN = cotangentFrame(N, vertIn.worldPos, vertIn.normalMapUV);
    float3 T = vertIn.tangent;
    float3 B = normalize(cross(N, T));
    float3x3 TBN = float3x3(T, B, N);
    TBN = (!pushData.hasTangents || dot(T, T) < 1e-8) ? flatTBN : TBN;

    Sampler2D normalTexture = textures[materials[pushData.materialIndex].normalTextureIndex];
    float3 normMap = normalTexture.Sample(vertIn.normalMapUV).xyz;
    normMap = normMap * 2.0f - 1.0f;
    float3 norm = normalize(mul(TBN, normMap));

    Sampler2D baseColorTexture = textures[materials[pushData.materialIndex].baseColorTextureIndex];
    //Sampler2D baseColorTexture = textures[1];
    float3 baseColor = baseColorTexture.Sample(vertIn.baseColorUV).xyz;
    if (all(baseColor == float3(0.0f, 0.0f, 0.0f)))
    {
        return materials[pushData.materialIndex].baseColorFactor;
    }
    return float4(baseColor, 1.0f);
}