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
    FWString VS_DrawQuadKey = L"VS_DrawQuad";
    FString DoFConstantBufferKey = "FDepthOfFieldConstant";
    ID3D11SamplerState* Sampler = nullptr;

private:
    void CreateShader();
    void UpdateDoFConstant(const std::shared_ptr<FEditorViewportClient>& Viewport);
private:
    void PrepareCoCPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpCoCPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void PrepareBlurNearPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpBlurNearPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void PrepareBlurFarPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpBlurFarPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void PrepareCompositePass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpCompositePass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void PrepareDebugPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
    void CleanUpDebugPass(const std::shared_ptr<FEditorViewportClient>& Viewport);
};

