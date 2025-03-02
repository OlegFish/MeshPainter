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

	/**	Создаем буффер
	sizeof(FVector4f) так как мы используем GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetVertexDeclarationFVector4();
	*6 - т.к. используем list, а для strip было бы 4
	EBufferUsageFlags - флаги, управляющие особенностями буффера
	CreateInfo - дополнительные опции буффера */
	FBufferRHIRef VertexBufferRHI = RHICmdList.CreateVertexBuffer(WholeBufferSize, BUF_Volatile, CreateInfo);
	/**	Блокируем созданный буффер для видеокарты, чтобы в него спокойно писать, для себя разблокируем
	Offset - сколько от начала буффера пропускаем, 0 значит блокируем от начала
	SizeRHI - сколько хотим заблокировать байт
	LockMode - параметры блокировки */
	void* VoidPtr = RHICmdList.LockBuffer(VertexBufferRHI, 0, WholeBufferSize, RLM_WriteOnly);

	/** Изменяем тип указателя void* на FVector4f* */
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

				/** Устанавливаем зону в которой рисуем в рендер таргете */
				RHICmdList.SetViewport(0.0f, 0.0f, 0.0f, Resource2D->GetSizeX(), Resource2D->GetSizeY(), 1.0);

				FGlobalShaderMap* GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
				TShaderMapRef<FSingleColorShaderVS> VertexShader(GlobalShaderMap);
				TShaderMapRef<FSingleColorShaderPS> PixelShader(GlobalShaderMap);

				/** Настраиваем GraphicsPSOInit (Graphics Pipeline state object)
				Он управляет тем как и что рисовать, указание конкретных шейдеров, настройки рендера и пр. */
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, ERasterizerDepthClipMode::DepthClip, false>::GetRHI();
				GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;

				/** Подключаем шейдера */
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();

				/** Устанавливаем Vertex type (структуру вертекса), то что шейдер получает на вход */
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GSingleColorVertexDeclaration.VertexDeclarationRHI;

				/** Используем заполненный выше GraphicsPSOInit для настройки пайплайна */
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit, 0);

				/** Настраиваем FParameters (параметры шейдеров)
				можно так, или как ниже FSingleColorShaderVS::FParameters ShaderParameters; */
				FSingleColorParameters ShaderParameters;
				ShaderParameters.Color = FVector3f(Color);
				ShaderParameters.Time = 0.f;

				/** Устанавливаем параметры для шейдера на комманд листе */
				SetShaderParameters(RHICmdList, VertexShader, GraphicsPSOInit.BoundShaderState.VertexShaderRHI, ShaderParameters);
				SetShaderParameters(RHICmdList, PixelShader, GraphicsPSOInit.BoundShaderState.PixelShaderRHI, ShaderParameters);

				/** Указывает, какой вертекс буфер мы будем использовать для рисования */
				FBufferRHIRef VertexBuffer = CreateTempVertexBuffer(RHICmdList);

				/** Указали где брать вертексы, параметр Offset начальный отступ */
				RHICmdList.SetStreamSource(0, VertexBuffer, 0);

				/** DrawPrimitive, команда рисовать - один вызов соответствует одному дроуколу
				BaseVertexIndex - еще один отступ от начала, отступы суммируются
				NumPrimitives - кол-во треугольников, которые мы нарисуем за один дроукол
				NumInstances - кол-во инстансов, которые рисуем

				Индекс буффер выглядит так:
				[0, 1, 2] первый треугольник
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