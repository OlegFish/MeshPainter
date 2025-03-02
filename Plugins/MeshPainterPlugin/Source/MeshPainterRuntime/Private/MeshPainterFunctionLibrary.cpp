#include "MeshPainterFunctionLibrary.h"
#include "PrimitiveSceneInfo.h"
#include "StaticMeshBatch.h"
#include "RenderCaptureInterface.h"
#include "Shaders/SingleColorShader.h"

DECLARE_GPU_STAT_NAMED(SingleColorRenderPass, TEXT("Single Color Render Pass"));

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

class FSingleColorVertexDeclaration : public FRenderResource
{
public:
	struct FVertex
	{
		FVector2f Position;
		FVector2f Texcoord;
	};

	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI(FRHICommandListBase& RHICmdList) override
	{
		FVertexDeclarationElementList Elements;
		Elements.Add(FVertexElement(0, offsetof(FVertex, Position), VET_Float2, 0, sizeof(FVertex)));
		Elements.Add(FVertexElement(0, offsetof(FVertex, Texcoord), VET_Float2, 1, sizeof(FVertex)));
		VertexDeclarationRHI = PipelineStateCache::GetOrCreateVertexDeclaration(Elements);
	}
	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

TGlobalResource<FSingleColorVertexDeclaration> GSingleColorVertexDeclaration;

inline FBufferRHIRef CreateTempVertexBuffer(FRHICommandListBase& RHICmdList)
{
	FRHIResourceCreateInfo CreateInfo(TEXT("TempVertexBuffer"));
	const int32 TotalVertices = 6;
	const size_t SingleVertexSize = sizeof(FSingleColorVertexDeclaration::FVertex);
	const size_t WholeBufferSize = SingleVertexSize * TotalVertices;

	/**	������� ������
	sizeof(FVector4f) ��� ��� �� ���������� GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
	*6 - �.�. ���������� list, � ��� strip ���� �� 4
	EBufferUsageFlags - �����, ����������� ������������� �������
	CreateInfo - �������������� ����� ������� */
	FBufferRHIRef VertexBufferRHI = RHICmdList.CreateVertexBuffer(WholeBufferSize, BUF_Volatile, CreateInfo);
	/**	��������� ��������� ������ ��� ����������, ����� � ���� �������� ������, ��� ���� ������������
	Offset - ������� �� ������ ������� ����������, 0 ������ ��������� �� ������
	SizeRHI - ������� ����� ������������� ����
	LockMode - ��������� ���������� */
	void* VoidPtr = RHICmdList.LockBuffer(VertexBufferRHI, 0, WholeBufferSize, RLM_WriteOnly);

	/** �������� ��� ��������� void* �� FVector4f* */
	FSingleColorVertexDeclaration::FVertex* Vertices = static_cast<FSingleColorVertexDeclaration::FVertex*>(VoidPtr);
	Vertices[0].Position = FVector2f(-1.0f, -1.0f);
	Vertices[1].Position = FVector2f(-1.0f, 1.0f);
	Vertices[2].Position = FVector2f(1.0f, -1.0f);

	Vertices[3].Position = FVector2f(-1.0f, 1.0f);
	Vertices[4].Position = FVector2f(1.0f, -1.0f);
	Vertices[5].Position = FVector2f(1.0f, 1.0f);

	Vertices[0].Texcoord = FVector2f(0.0f, 0.0f);
	Vertices[1].Texcoord = FVector2f(0.0f, 1.0f);
	Vertices[2].Texcoord = FVector2f(1.0f, 0.0f);

	Vertices[3].Texcoord = FVector2f(0.0f, 1.0f);
	Vertices[4].Texcoord = FVector2f(1.0f, 0.0f);
	Vertices[5].Texcoord = FVector2f(1.0f, 1.0f);

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

			FTextureRHIRef RenderTargetTexture = Resource2D->GetRenderTargetTexture();
			RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::SRVMask, ERHIAccess::RTV));
			FRHIRenderPassInfo RenderPassInfo(RenderTargetTexture, ERenderTargetActions::Load_Store);
			
			RHICmdList.BeginRenderPass(RenderPassInfo, TEXT("RenderSolidColor"));
			{
				SCOPED_GPU_STAT(RHICmdList, SingleColorRenderPass);

				/** ������������� ���� � ������� ������ � ������ ������� */
				RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, Resource2D->GetSizeX(), Resource2D->GetSizeY(), 1.0);

				FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
				TShaderMapRef<FSingleColorShaderVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FSingleColorShaderPS> PixelShader(GlobalShaderMap);

				/** ����������� GraphicsPSOInit (Graphics Pipeline state object)
				�� ��������� ��� ��� � ��� ��������, �������� ���������� ��������, ��������� ������� � ��. */
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, false>::GetRHI();
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;

				/** ���������� ������� */
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				/** ������������� Vertex type (��������� ��������), �� ��� ������ �������� �� ���� */
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSingleColorVertexDeclaration.VertexDeclarationRHI;

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
			}
			RHICmdList.EndRenderPass();
			RHICmdList.Transition(FRHITransitionInfo(RenderTargetTexture, ERHIAccess::RTV, ERHIAccess::SRVMask));

		});
	FlushRenderingCommands();
	RenderTarget->UpdateResourceImmediate(false);
}