#include "DepthOfFieldPass.h"
#include "RendererHelpers.h"
#include "UnrealClient.h"
#include "D3D11RHI/DXDShaderManager.h"
#include "UnrealEd/EditorViewportClient.h"
#include "PropertyEditor/ShowFlags.h"

void FDepthOfFieldPass::Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage)
{
    FRenderPassBase::Initialize(InBufferManager, InGraphics, InShaderManage);

    CreateShader();

    uint32 DepthOfFieldSize = sizeof(FDepthOfFieldConstant);
    HRESULT hr = BufferManager->CreateBufferGeneric<FDepthOfFieldConstant>(
        DoFConstantBufferKey, nullptr, DepthOfFieldSize,
        D3D11_BIND_CONSTANT_BUFFER, D3D11_USAGE_DYNAMIC, D3D11_CPU_ACCESS_WRITE);

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
    
    PrepareCoCPass(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpCoCPass(Viewport);

    PrepareBlurNearPass(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpBlurNearPass(Viewport);

    PrepareBlurFarPass(Viewport);
    Graphics->DeviceContext->Draw(6, 0);
    CleanUpBlurFarPass(Viewport);

    if (!(Viewport->GetShowFlag() & EEngineShowFlags::SF_DepthOfField_DebugLayer))
    {
        PrepareCompositePass(Viewport);
        Graphics->DeviceContext->Draw(6, 0);
        CleanUpCompositePass(Viewport);
    }
    else if (Viewport->GetShowFlag() & EEngineShowFlags::SF_DepthOfField_DebugLayer)
    {
        PrepareDebugPass(Viewport);
        Graphics->DeviceContext->Draw(6, 0);
        CleanUpDebugPass(Viewport);
    }

    CleanUpRender(Viewport);
}

void FDepthOfFieldPass::PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11VertexShader* VertexShader = ShaderManager->GetVertexShaderByKey(VS_DrawQuadKey);
    Graphics->DeviceContext->VSSetShader(VertexShader, nullptr, 0);

    Graphics->DeviceContext->IASetVertexBuffers(0, 0, nullptr, nullptr, nullptr);
    Graphics->DeviceContext->IASetInputLayout(nullptr);
    Graphics->DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    Graphics->DeviceContext->RSSetState(Graphics->RasterizerSolidBack);
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState_DepthTestDisabled,0);
    Graphics->DeviceContext->PSSetSamplers(0, 1, &Sampler);
}

void FDepthOfFieldPass::CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetDepthStencilState(Graphics->DepthStencilState_Default, 1);
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
}

void FDepthOfFieldPass::CreateShader()
{
    Sampler = Graphics->GetSamplerState(ESamplerType::LinearClamp);

    HRESULT hr = ShaderManager->AddVertexShader(L"VS_DrawQuad", L"Shaders/DepthOfFieldShader.hlsl", "mainVS");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"PS_GenerateCoC", L"Shaders/DepthOfFieldShader.hlsl", "PS_GenerateCoC");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"PS_BlurNear", L"Shaders/DepthOfFieldShader.hlsl", "PS_BlurNear");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"PS_BlurFar", L"Shaders/DepthOfFieldShader.hlsl", "PS_BlurFar");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"PS_Composite", L"Shaders/DepthOfFieldShader.hlsl", "PS_Composite");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }

    hr = ShaderManager->AddPixelShader(L"PS_DebugCoC", L"Shaders/DepthOfFieldShader.hlsl", "PS_DebugCoC");
    if (FAILED(hr))
    {
        UE_LOG(ELogLevel::Error, TEXT("Failed to create DepthOfFieldPixelShader shader!"));
    }
}

void FDepthOfFieldPass::UpdateDoFConstant(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    FDepthOfFieldConstant DoFConstants;
    DoFConstants.F_Stop = Viewport->F_Stop;
    DoFConstants.FocalLength_mm = Viewport->FocalLength_mm;
    DoFConstants.FocalDistance_World = Viewport->FocusDistance;
    DoFConstants.CoCScaleFactor = Viewport->CoCScaleFactor;
    DoFConstants.SensorWidth_mm = Viewport->SensorWidth_mm;
    DoFConstants.MaxBokehRadius = 4.0f;
    FRect ViewportRect = Viewport->GetViewport()->GetRect();
    DoFConstants.ScreenSize = FVector2D(ViewportRect.Width, ViewportRect.Height);
    BufferManager->UpdateConstantBuffer<FDepthOfFieldConstant>(DoFConstantBufferKey, DoFConstants);
}

void FDepthOfFieldPass::PrepareCoCPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"PS_GenerateCoC");
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    TArray<FString> PSBufferKeys = {
        TEXT(DoFConstantBufferKey)
    };

    UpdateDoFConstant(Viewport);
    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_DoF_CoC;
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, &ViewportResource->GetDepthStencil(EResourceType::ERT_Scene)->SRV);
    
}

void FDepthOfFieldPass::CleanUpCoCPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_SceneDepth), 1, NullSRV);

}

void FDepthOfFieldPass::PrepareBlurNearPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"PS_BlurNear");
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    TArray<FString> PSBufferKeys = {
        TEXT(DoFConstantBufferKey)
    };

    UpdateDoFConstant(Viewport);
    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_DoF_BlurNear;
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_DoF_CoC)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_Scene)->SRV);

}

void FDepthOfFieldPass::CleanUpBlurNearPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);

}

void FDepthOfFieldPass::PrepareBlurFarPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"PS_BlurFar");
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    TArray<FString> PSBufferKeys = {
        TEXT(DoFConstantBufferKey)
    };

    UpdateDoFConstant(Viewport);
    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_DoF_BlurFar;
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_DoF_CoC)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_Scene)->SRV);


}

void FDepthOfFieldPass::CleanUpBlurFarPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);

}

void FDepthOfFieldPass::PrepareCompositePass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"PS_Composite");
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);

    TArray<FString> PSBufferKeys = {
        TEXT(DoFConstantBufferKey)
    };

    BufferManager->BindConstantBuffers(PSBufferKeys, 0, EShaderStage::Pixel);

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_DoF_Result;
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_DoF_CoC)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_Scene)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_BlurNear), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_DoF_BlurNear)->SRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_BlurFar), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_DoF_BlurFar)->SRV);

}

void FDepthOfFieldPass::CleanUpCompositePass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);

    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1,NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_Scene), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_BlurNear), 1, NullSRV);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_BlurFar), 1, NullSRV);

}

void FDepthOfFieldPass::PrepareDebugPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    ID3D11PixelShader* PixelShader = ShaderManager->GetPixelShaderByKey(L"PS_DebugCoC");
    Graphics->DeviceContext->PSSetShader(PixelShader, nullptr, 0);    

    FViewportResource* ViewportResource = Viewport->GetViewportResource();
    const EResourceType ResourceType = EResourceType::ERT_DoF_Result;
    FRenderTargetRHI* RenderTargetRHI = ViewportResource->GetRenderTarget(ResourceType);

    Graphics->DeviceContext->OMSetDepthStencilState(nullptr, 1);
    Graphics->DeviceContext->OMSetRenderTargets(1, &RenderTargetRHI->RTV, nullptr);
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, &ViewportResource->GetRenderTarget(EResourceType::ERT_DoF_CoC)->SRV);

}

void FDepthOfFieldPass::CleanUpDebugPass(const std::shared_ptr<FEditorViewportClient>& Viewport)
{
    Graphics->DeviceContext->OMSetRenderTargets(0, nullptr, nullptr);
    ID3D11ShaderResourceView* NullSRV[1] = { nullptr };
    Graphics->DeviceContext->PSSetShaderResources(static_cast<UINT>(EShaderSRVSlot::SRV_DoF_CoC), 1, NullSRV);

}
