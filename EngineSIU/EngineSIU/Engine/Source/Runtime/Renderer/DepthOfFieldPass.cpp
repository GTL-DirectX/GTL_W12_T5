#include "DepthOfFieldPass.h"
#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorViewportClient.h"

void FDepthOfFieldPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    FRenderPassBase::Initialize(InBufferManager, InGraphics, InShaderManage);
    HRESULT hr = ShaderManager->AddVertexShader(VertexShaderKey, L"Shaders/DepthOfFieldShader.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldVertexShader shader!"));
    }
    hr = ShaderManager->AddPixelShader(PixelShaderKey, L"Shaders/DepthOfFieldShader.hlsl", "mainPS");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }
}

void FDepthOfFieldPass::PrepareRenderArr()
{
}

void FDepthOfFieldPass::ClearRenderArr()
{
}

void FDepthOfFieldPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    PrepareRender(Viewport);

    CleanUpRender(Viewport);
}

void FDepthOfFieldPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{

}

void FDepthOfFieldPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
}
