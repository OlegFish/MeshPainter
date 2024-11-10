#pragma once

#include "Kismet/BlueprintFunctionLibrary.h"
#include "MeshPainterShaderCoreBPLibrary.generated.h"

UCLASS()
class UMeshPainterShaderCoreBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_UCLASS_BODY()

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Execute Sample function", Keywords = "MeshPainterRuntime sample test testing"), Category = "MeshPainterRuntimeTesting")
	static float MeshPainterShaderCoreSampleFunction(float Param);
};
