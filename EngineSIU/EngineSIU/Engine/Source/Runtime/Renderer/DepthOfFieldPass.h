#pragma once
#include "RenderPassBase.h"

class ID3D11VertexShader;
class ID3D11PixelShader;
class ID3D11SamplerState;

class FDepthOfFieldPass : public FRenderPassBase
{
public:
    FDepthOfFieldPass() = default;
    virtual ~FDepthOfFieldPass() override = default;

    virtual void Initialize(FDXDBufferManager* InBufferManager, FGraphicsDevice* InGraphics, FDXDShaderManager* InShaderManage) override;
    virtual void PrepareRenderArr() override;
    virtual void ClearRenderArr() override;
    virtual void Render(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
protected:
    virtual void PrepareRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
    virtual void CleanUpRender(const std::shared_ptr<FEditorViewportClient>& Viewport) override;
private:
    FWString ShaderKey = L"DepthOfFieldShader";
    FString DoFConstantBufferKey = "FCameraDoFConstants";
    ID3D11VertexShader* VertexShader = nullptr;
    ID3D11PixelShader* PixelShader = nullptr;
    ID3D11SamplerState* Sampler = nullptr;
private:
    void UpdateDoFConstant(const std::shared_ptr<FEditorViewportClient>& Viewport);
};

