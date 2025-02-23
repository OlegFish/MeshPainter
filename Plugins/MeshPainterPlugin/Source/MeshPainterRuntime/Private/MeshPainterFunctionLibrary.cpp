#include "MeshPainterFunctionLibrary.h"
#include "PrimitiveSceneInfo.h"
#include "StaticMeshBatch.h"
#include "RenderCaptureInterface.h"
#include "Shaders/SingleColorShader.h"
//#include "MeshPainterRender.h"

TAutoConsoleVariable<bool> CVarEnableTestProfiling(TEXT("r.profile.RenderSolidColor"), false, TEXT("Enable solid color profiling"), ECVF_RenderThreadSafe | ECVF_Default);

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

void UMeshPainterFunctionLibrary::RenderSolidColorShader(UObject* WorldContextObject, UTextureRenderTarget2D* RenderTarget, FColor Color)
{
	if (!IsValid(RenderTarget)) return;
	UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (!IsValid(World)) return;
	ERHIFeatureLevel::Type FeatureLevel = World->GetFeatureLevel();

	ENQUEUE_RENDER_COMMAND(RenderSolidColor)([RenderTarget, Color, FeatureLevel](FRHICommandListImmediate& RHICmdList)
	{		
		FTextureRenderTargetResource* Resource = RenderTarget->GetRenderTargetResource();
		if (!Resource) return;

		FTextureRenderTarget2DResource* Resource2D = Resource->GetTextureRenderTarget2DResource();
		if (!Resource2D) return;
		RenderCaptureInterface::FScopedCapture RenderCapture(CVarEnableTestProfiling.GetValueOnRenderThread(), &RHICmdList);

		FRHIRenderPassInfo RenderPassInfo;
		RenderPassInfo.bOcclusionQueries = false;
		RenderPassInfo.ColorRenderTargets[0].RenderTarget = Resource2D->GetRenderTargetTexture();
		RenderPassInfo.ColorRenderTargets[0].Action = ERenderTargetActions::Load_Store;
		RenderPassInfo.ColorRenderTargets[0].MipIndex = 0;
		RenderPassInfo.ColorRenderTargets[0].ArraySlice = 0;

		RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("RenderSolidColor"));

		/** Устанавливаем зону в которой рисуем в рендер таргете */
		RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, Resource2D->GetSizeX(), Resource2D->GetSizeY(), 1.0);
		
		FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FSingleColorShaderVS> VertexShader(GlobalShaderMap);
		TShaderMapRef<FSingleColorShaderPS> PixelShader(GlobalShaderMap);

		/** Настраиваем GraphicsPSOInit (Graphics Pipeline state object)
		Он управляет тем как и что рисовать, указание конкретных шейдеров, настройки рендера и пр. */
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, false>::GetRHI();
		GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;

		/** Подключаем шейдера */
		GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
		GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

		/** Устанавливаем Vertex type (структуру вертекса), то что шейдер получает на вход */
		GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();

		/** Используем заполненный выше GraphicsPSOInit для настройки пайплайна */
		SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

		/** Настраиваем FParameters (параметры шейдеров)
		можно так, или как ниже FSingleColorShaderVS::FParameters ShaderParameters; */
		FSingleColorParameters ShaderParameters;
		ShaderParameters.Color = FVector3f(Color);
		ShaderParameters.Time = 0.f;

		// Останивились на пиксельном шейдере

		SetShaderParameters(RHICmdList, VertexShader, GraphicsPSOInit.BoundShaderState.VertexShaderRHI, ShaderParameters);

		RHICmdList.EndRenderPass();
	
	});
	FlushRenderingCommands();
	RenderTarget->UpdateResourceImmediate(false);
}
