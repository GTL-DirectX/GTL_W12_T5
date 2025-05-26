#include "ShaderRegisters.hlsl"

Texture2D SceneDepthTexture : register(t99); // 깊이 맵 텍스처
Texture2D SceneColorTexture : register(t100); // 씬 텍스처
SamplerState LinearSampler : register(s0); // 샘플러 상태

cbuffer CameraDoFConstants : register(b0)
{
    float FocalLength; // 초점 거리
    float Aperture; // 조리개 값
    float FocusDistance; // 초점 거리
    float MaxCoC; // 최대 COC (Circle of Confusion)
    float2 ScreenSize; // 화면 크기
}

struct PS_INPUT
{
    float4 Position : SV_POSITION; // 정점 위치
    float2 UV : TEXCOORD; // UV 좌표
};

float LinearizeDepth(float z)
{
    return NearClip * FarClip / (FarClip - z * (FarClip - NearClip));
}

float ComputeCoC(float ViewZ)
{
    float FocusRange = FocusDistance * 0.5;
    float coc = abs(ViewZ - FocusDistance) / FocusRange;
    return saturate(coc) * MaxCoC;
}

float3 GaussianBlur(float2 uv, float coc)
{
    float2 texelSize = 1.0 / ScreenSize;
    float3 color = 0;
    float weightSum = 0;

    const int radius = 8;

    for (int y = -radius; y <= radius; ++y)
    {
        for (int x = -radius; x <= radius; ++x)
        {
            float2 offset = float2(x, y) * texelSize * coc;
            float2 sampleUV = uv + offset;

            float2 dist = float2(x, y);
            float weight = exp(-dot(dist, dist) / 20.0); // 가우시안 커널

            color += SceneColorTexture.Sample(LinearSampler, sampleUV) * weight;
            weightSum += weight;
        }
    }

    return color / weightSum;
}

PS_INPUT mainVS(uint VertexID : SV_VertexID)
{
    PS_INPUT Output;

    float2 QuadPositions[6] =
    {
        float2(-1, 1), // Top Left
        float2(1, 1), // Top Right
        float2(-1, -1), // Bottom Left
        float2(1, 1), // Top Right
        float2(1, -1), // Bottom Right
        float2(-1, -1) // Bottom Left
    };

    float2 UVs[6] =
    {
        float2(0, 0), float2(1, 0), float2(0, 1),
        float2(1, 0), float2(1, 1), float2(0, 1)
    };

    Output.Position = float4(QuadPositions[VertexID], 0, 1);
    Output.UV = UVs[VertexID];

    return Output;
}

float4 mainPS(PS_INPUT input) : SV_Target
{
    float RawDepth = SceneDepthTexture.Sample(LinearSampler, input.UV).r;
    float ViewZ = LinearizeDepth(RawDepth);
    float CoC = ComputeCoC(ViewZ);
    float3 BlurredColor = GaussianBlur(input.UV, CoC);
    float Blend = saturate(CoC / MaxCoC);
    return float4(BlurredColor, Blend);
}
