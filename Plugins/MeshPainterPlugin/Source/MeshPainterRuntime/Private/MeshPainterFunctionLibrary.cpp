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

inline FBufferRHIRef CreateTempVertexBuffer(FRHICommandListBase& RHICmdList)
{
	FRHIResourceCreateInfo CreateInfo(TEXT("TempMediaVertexBuffer"));
	/**	������� ������
	sizeof(FVector4f) ��� ��� �� ���������� GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
	*6 - �.�. ���������� list, � ��� strip ���� �� 4
	EBufferUsageFlags - �����, ����������� ������������� �������
	CreateInfo - �������������� ����� ������� */
	FBufferRHIRef VertexBufferRHI = RHICmdList.CreateVertexBuffer(sizeof(FVector4f) * 6, BUF_Volatile, CreateInfo);
	/**	��������� ��������� ������ ��� ����������, ����� � ���� �������� ������, ��� ���� ������������
	Offset - ������� �� ������ ������� ����������, 0 ������ ��������� �� ������
	SizeRHI - ������� ����� ������������� ����
	LockMode - ��������� ���������� */
	void* VoidPtr = RHICmdList.LockBuffer(VertexBufferRHI, 0, sizeof(FVector4f) * 6, RLM_WriteOnly);

	/** �������� ��� ��������� void* �� FVector4f* */
	FVector4f* Vertices = (FVector4f*)VoidPtr;
	Vertices[0] = FVector4f(-1.0f, -1.0f, 0.0f, 1.0f);
	Vertices[1] = FVector4f(-1.0f, 1.0f, 0.0f, 1.0f);
	Vertices[2] = FVector4f(1.0f, -1.0f, 0.0f, 1.0f);

	Vertices[3] = FVector4f(-1.0f, 1.0f, 0.0f, 1.0f);
	Vertices[4] = FVector4f(1.0f, -1.0f, 0.0f, 1.0f);
	Vertices[5] = FVector4f(1.0f, 1.0f, 0.0f, 1.0f);

	RHICmdList.UnlockBuffer(VertexBufferRHI);

	return VertexBufferRHI;
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

			/** ������������� ���� � ������� ������ � ������ ������� */
			RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, Resource2D->GetSizeX(), Resource2D->GetSizeY(), 1.0);

			FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
			TShaderMapRef<FSingleColorShaderVS> VertexShader(GlobalShaderMap);
			TShaderMapRef<FSingleColorShaderPS> PixelShader(GlobalShaderMap);

			/** ����������� GraphicsPSOInit (Graphics Pipeline state object)
			�� ��������� ��� ��� � ��� ��������, �������� ���������� ��������, ��������� ������� � ��. */
			FGraphicsPipelineStateInitializer GraphicsPSOInit;
			GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
			GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, false>::GetRHI();
			GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;

			/** ���������� ������� */
			GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
			GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

			/** ������������� Vertex type (��������� ��������), �� ��� ������ �������� �� ���� */
			GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();

			/** ���������� ����������� ���� GraphicsPSOInit ��� ��������� ��������� */
			SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

			/** ����������� FParameters (��������� ��������)
			����� ���, ��� ��� ���� FSingleColorShaderVS::FParameters ShaderParameters; */
			FSingleColorParameters ShaderParameters;
			ShaderParameters.Color = FVector3f(Color);
			ShaderParameters.Time = 0.f;

			/** ������������� ��������� ��� ������� �� ������� ����� */
			SetShaderParameters(RHICmdList, VertexShader, GraphicsPSOInit.BoundShaderState.VertexShaderRHI, ShaderParameters);
			SetShaderParameters(RHICmdList, PixelShader, GraphicsPSOInit.BoundShaderState.PixelShaderRHI, ShaderParameters);

			/** ���������, ����� ������� ����� �� ����� ������������ ��� ��������� */
			FBufferRHIRef VertexBuffer = CreateTempVertexBuffer(RHICmdList);

			/** ������� ��� ����� ��������, �������� Offset ��������� ������ */
			RHICmdList.SetStreamSource(0, VertexBuffer, 0);

			/** DrawPrimitive, ������� �������� - ���� ����� ������������� ������ ��������
			BaseVertexIndex - ��� ���� ������ �� ������, ������� �����������
			NumPrimitives - ���-�� �������������, ������� �� �������� �� ���� �������
			NumInstances - ���-�� ���������, ������� ������

			������ ������ �������� ���:
			[0, 1, 2] ������ �����������
			[3, 4, 5]
			...
			*/
			RHICmdList.DrawPrimitive(0, 2, 1);

			RHICmdList.EndRenderPass();

		});
	FlushRenderingCommands();
	RenderTarget->UpdateResourceImmediate(false);
}