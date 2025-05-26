#include "ShaderRegisters.hlsl"

Texture2D SceneDepthTexture : register(t99); // 깊이 맵 텍스처
Texture2D SceneColorTexture : register(t100); // 씬 텍스처
Texture2D CoCTexture : register(t107); // Circle of Confusion 텍스처
Texture2D NearBlurTexture : register(t108); // 근거리 블러 텍스처
Texture2D FarBlurTexture : register(t109); // 원거리 블러 텍스처

SamplerState LinearSampler : register(s0); // 샘플러 상태

cbuffer CameraDoFConstants : register(b0)
{
    float FocalLength; // 초점 거리
    float Aperture; // 조리개 값
    float FocusDistance; // 초점 거리
    float MaxCoC; // 최대 COC (Circle of Confusion)
    float2 ScreenSize; // 화면 크기
}

cbuffer DepthOfFieldConstant : register(b1)
{
    float F_Stop; // f-number
    float SensorWidth_mm; // 센서 너비 (예: 36.0f)
    float FocalDistance_World; // 초점 거리 (cm)
    float FocalLength_mm; // 렌즈 초점 거리 (mm)
    float CoCScaleFactor; // 흐림 강도 스케일 팩터
    float InFocusThreshold; // (생략)
    float MaxBokehRadius; // 최대 보케 반경
    float pad1;
};

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

float ComputePhysicalCoC(float SceneDepth_World)
{
    // 1) 단위 통일: cm → mm
    float Z_mm = SceneDepth_World * 10.0f;
    float D_mm = FocalDistance_World * 10.0f;

    // 2) 0에 가까워지는 것을 방지
    Z_mm = max(Z_mm, 0.001f);
    D_mm = max(D_mm, 0.001f);

    // 3) 카메라 공식을 위한 분모/분자
    //    CoC_sensor = (F^2 / N) * (Z - D) / (Z * D)
    float numerator = FocalLength_mm * FocalLength_mm / F_Stop * (Z_mm - D_mm);
    float denominator = Z_mm * D_mm;

    // 4) 분모가 너무 작으면 CoC = 0 처리
    float CoC_sensor = (abs(denominator) > 1e-5f)
                     ? numerator / denominator
                     : 0.0f;

    // 5) 센서 크기로 정규화 (unitless)
    float NormalizedCoC = CoC_sensor / SensorWidth_mm;

    // 6) 디자이너 튜닝용 스케일 팩터 적용
    float ScaledCoC = NormalizedCoC * CoCScaleFactor;

    return ScaledCoC;
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

static const int NUM_SAMPLES = 19;
static const float2 PoissonDisk[NUM_SAMPLES] =
{
    float2(0.000000f, 0.000000f), // Center
    float2(0.527837f, -0.085868f),
    float2(-0.040088f, 0.536087f),
    float2(-0.670445f, -0.179949f),
    float2(-0.419418f, -0.616039f),
    float2(0.440453f, -0.639399f),
    float2(-0.757088f, 0.349334f),
    float2(0.574619f, 0.685879f),
    float2(0.976331f, 0.153460f),
    float2(-0.624817f, 0.765323f),
    float2(0.122747f, 0.970479f),
    float2(0.840895f, -0.524980f),
    float2(-0.043655f, -0.967251f),
    float2(-0.848312f, -0.519516f),
    float2(-0.998088f, 0.054414f),
    float2(0.285328f, 0.418364f),
    float2(-0.273026f, -0.340141f),
    float2(0.725791f, 0.326734f),
    float2(-0.311553f, 0.148081f)
};

float3 VariableRadiusBlur(float2 uv, float radius)
{
    // 화면 해상도에 대한 텍셀 크기
    float2 texelSize = 1.0f / ScreenSize;

    float3 accumColor = float3(0, 0, 0);
    float totalWeight = 0.0f;

    // Poisson Disk 샘플링
    [unroll]
    for (int i = 0; i < NUM_SAMPLES; ++i)
    {
        // PoissonDisk[i] 범위가 [-1,+1]이라면
        float2 offset = PoissonDisk[i] * radius * texelSize;
        float3 sample = SceneColorTexture.Sample(LinearSampler, uv + offset).rgb;

        // (옵션) 거리 기반 가중치: Gaussian
        // float w = exp(-dot(offset,offset) / (2.0 * radius*radius));
        // accumColor += sample * w;
        // totalWeight += w;

        // 단순 평균
        accumColor += sample;
        totalWeight += 1.0f;
    }

    return accumColor / totalWeight;
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


float4 PS_GenerateCoC(PS_INPUT IN) : SV_Target
{
    float depth = LinearizeDepth(SceneDepthTexture.Sample(LinearSampler, IN.UV).r);
    float coc = ComputePhysicalCoC(depth); // signed: 음수=near, 양수=far
    return float4(coc, 0, 0, 0);
}

float4 PS_BlurNear(PS_INPUT IN) : SV_Target
{
    // 부호 있는 CoC 가져오기
    float signedCoC = CoCTexture.Sample(LinearSampler, IN.UV).r;

    // coc >= 0 이면 원본 반환 (in-focus + far)
    if (signedCoC >= 0.0f)
        return SceneColorTexture.Sample(LinearSampler, IN.UV);

    // 음수(coc<0)일 때만 Near blur
    float radius = abs(signedCoC) * MaxBokehRadius;
    float3 blur = VariableRadiusBlur(IN.UV, radius);

    return float4(blur, 1.0f);
}

float4 PS_BlurFar(PS_INPUT IN) : SV_Target
{
    // 부호 있는 CoC 가져오기
    float signedCoC = CoCTexture.Sample(LinearSampler, IN.UV).r;

    // coc <= 0 이면 원본 반환 (in-focus + near)
    if (signedCoC <= 0.0f)
        return SceneColorTexture.Sample(LinearSampler, IN.UV);

    // 양수(coc>0)일 때만 Far blur
    float radius = signedCoC * MaxBokehRadius;
    float3 blur = VariableRadiusBlur(IN.UV, radius);

    return float4(blur, 1.0f);
}

float4 PS_Composite(PS_INPUT IN) : SV_Target
{
    float coc = CoCTexture.Sample(LinearSampler, IN.UV).r;
    float3 orig = SceneColorTexture.Sample(LinearSampler, IN.UV).rgb;
    float3 blurN = NearBlurTexture.Sample(LinearSampler, IN.UV).rgb;
    float3 blurF = FarBlurTexture.Sample(LinearSampler, IN.UV).rgb;

    // in-focus는 원본, out-of-focus는 near/far 블러로 lerp
    float blend = saturate(abs(coc) / MaxBokehRadius);
    float3 result = orig;

    if (coc < 0)
        result = lerp(orig, blurN, blend);
    else if (coc > 0)
        result = lerp(orig, blurF, blend);

    return float4(result, 1);
}
