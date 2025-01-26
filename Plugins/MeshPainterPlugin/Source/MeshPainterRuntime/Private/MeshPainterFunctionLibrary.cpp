#include "MeshPainterFunctionLibrary.h"
#include "PrimitiveSceneInfo.h"
#include "StaticMeshBatch.h"
#include "RenderCaptureInterface.h"
//#include "MeshPainterRender.h"

bool UMeshPainterFunctionLibrary::RenderMaterialOnMeshUVLayout(
	UObject* WorldContextObject,
	UPrimitiveComponent* MeshComponent,
	UMaterialInterface* Material,
	UTextureRenderTarget2D* BaseColor,
	UTextureRenderTarget2D* Emissive,
	UTextureRenderTarget2D* NormalMap,
	int32 LOD, bool bClearRenderTargets, ERenderMaterialOnMeshFilter Filter)
{
	FRenderMaterialOnMeshPrimitive PrimitiveInfo;
	PrimitiveInfo.DesiredLOD = LOD;
	PrimitiveInfo.MeshComponent = MeshComponent;
	return 1;
}

void UMeshPainterFunctionLibrary::RenderSolidColorShader(UTextureRenderTarget2D* RenderTarget, FColor Color)
{
	if (!IsValid(RenderTarget)) return;

	ENQUEUE_RENDER_COMMAND(RenderSolidColor)([RenderTarget, Color](FRHICommandListImmediate& RHICmdList)
	{		
		FTextureRenderTargetResource* Resource = RenderTarget->GetRenderTargetResource();
		if (!Resource) return;

		FTextureRenderTarget2DResource* Resource2D = Resource->GetTextureRenderTarget2DResource();
		if (!Resource2D) return;
		RenderCaptureInterface::FScopedCapture RenderCapture(true, &RHICmdList);

		FRHIRenderPassInfo RenderPassInfo;
		RenderPassInfo.bOcclusionQueries = false;
		RenderPassInfo.ColorRenderTargets[0].RenderTarget = Resource2D->GetRenderTargetTexture();
		RenderPassInfo.ColorRenderTargets[0].Action = ERenderTargetActions::Load_Store;
		RenderPassInfo.ColorRenderTargets[0].MipIndex = 0;
		RenderPassInfo.ColorRenderTargets[0].ArraySlice = 0;

		RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("RenderSolidColor"));

		RHICmdList.EndRenderPass();
	
	});
	FlushRenderingCommands();
	RenderTarget->UpdateResourceImmediate(false);
}
