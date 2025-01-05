#pragma once

#include "CoreMinimal.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSingleColorParameters, )
SHADER_PARAMETER(FVector3f, Color)
END_SHADER_PARAMETER_STRUCT()

void FFF()
{
	FSingleColorParameters X;
	X.Color = FVector3f(FColor::Red);
}


class FSingleColorShaderVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FSingleColorShaderVS);
	SHADER_USE_PARAMETER_STRUCT(FSingleColorShaderVS, FGlobalShader);

	using FParameters = FSingleColorParameters;
	//typedef FSingleColorParameters FParameters;

public:
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(InParameters, OutEnvironment);
		//OutEnvironment.SetDefine(TEXT("TILE_SIZE"), FComputeShaderUtils::kGolden2DGroupSize);
	}
};