#include "DepthOfFieldPass.h"
#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "PropertyEditor/ShowFlags.h"

void FDepthOfFieldPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    FRenderPassBase::Initialize(InBufferManager, InGraphics, InShaderManage);

    HRESULT hr = ShaderManager->AddVertexShader(ShaderKey, L"Shaders/DepthOfFieldShader.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldVertexShader shader!"));
    }
    hr = ShaderManager->AddPixelShader(ShaderKey, L"Shaders/DepthOfFieldShader.hlsl", "mainPS");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }

    VertexShader = ShaderManager->GetVertexShaderByKey(ShaderKey);
    PixelShader = ShaderManager->GetPixelShaderByKey(ShaderKey);
    Sampler = Graphics->GetSamplerState(ESamplerType::LinearClamp);
}

void FDepthOfFieldPass::PrepareRenderArr()
{
}

void FDepthOfFieldPass::ClearRenderArr()
{
}

void FDepthOfFieldPass::Render(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    const uint64 ShowFlag = Viewport->GetShowFlag();
    if (!(ShowFlag & EEngineShowFlags::SF_DepthOfField))
        return;

    PrepareRender(Viewport);

    UpdateDoFConstant(Viewport);
    
    Graphics->DeviceContext->Draw(6, 0);

    CleanUpRender(Viewport);
}

void FDepthOfFieldPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);

    Graphics->DeviceContext->PSSetSamplers(0, 1, &Sampler);

    TArray<FString> PSBufferKeys = {
        TEXT(DoFConstantBufferKey)
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_DepthOfField;
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);


    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, &ViewportResource->GetDepthStencil(EResourceType::ERT_Scene)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_Scene)->SRV);

}

void FDepthOfFieldPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState_Default, 1);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, nullptr);
}

void FDepthOfFieldPass::UpdateDoFConstant(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FCameraDoFConstants DoFConstants;
    DoFConstants.FocalLength = Viewport->FocalLength;
    DoFConstants.Aperture = Viewport->Aperture;
    DoFConstants.FocusDistance = Viewport->FocusDistance;
    DoFConstants.MaxCoC = Viewport->MaxCoC;
    FRect ViewportRect = Viewport->GetViewport()->GetRect();
    DoFConstants.ScreenSize = FVector2D(ViewportRect.Width, ViewportRect.Height);
    BufferManager->UpdateConstantBuffer<FCameraDoFConstants>(DoFConstantBufferKey, DoFConstants);
}
