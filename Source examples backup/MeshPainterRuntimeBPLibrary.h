#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshPainterRuntimeBPLibrary.generated.h"

UCLASS()
class UMeshPainterRuntimeBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "MeshPainterRuntime sample test testing"), Category = "MeshPainterRuntimeTesting")
	static float MeshPainterRuntimeSampleFunction(float Param);

	//static FVector MakeTestVector(float X, float Y, float Z);
};