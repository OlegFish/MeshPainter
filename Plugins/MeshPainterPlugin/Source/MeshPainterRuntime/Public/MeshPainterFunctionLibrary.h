#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Engine/TextureRenderTarget2D.h"
#include "MeshPainterFunctionLibrary.generated.h"

UENUM(BlueprintType)
enum class ERenderMaterialOnMeshFilter : uint8
{
	// No filtering
	None,

	// Apply 4x dilation
	Dilation4x,

	// Apply 8x dilation
	Dilation8x
};

USTRUCT(BlueprintType)
struct FRenderMaterialOnMeshPrimitive
{
	GENERATED_BODY()

	FRenderMaterialOnMeshPrimitive() : MeshComponent(nullptr), DesiredLOD(0), DesiredUV(0), UVRegion(FBox2D(FVector2D::Zero(), FVector2D::One())) {};

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPrimitiveComponent* MeshComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DesiredLOD;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 DesiredUV;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FBox2D UVRegion;
};

USTRUCT(BlueprintType)
struct FRenderMaterialOnMeshViewConfiguration
{
	GENERATED_BODY()

	FRenderMaterialOnMeshViewConfiguration() : ViewOrigin(FVector::ZeroVector), ViewRotationMatrix(FMatrix::Identity), ProjectionMatrix(FOrthoMatrix(512, 512, 1.0f, 0.0f)) {}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector ViewOrigin;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMatrix ViewRotationMatrix;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FMatrix ProjectionMatrix;
};