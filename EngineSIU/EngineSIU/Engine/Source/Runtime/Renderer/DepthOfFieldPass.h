#pragma once
#include "RenderPassBase.h"
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
    FWString VertexShaderKey = L"DepthOfFieldVertexShader";
    FWString PixelShaderKey = L"DepthOfFieldPixelShader";
};

