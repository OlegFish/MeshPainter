#pragma once

#include "CoreMinimal.h"
#include "ShaderParameterStruct.h"
#include "DataDrivenShaderPlatformInfo.h"

BEGIN_SHADER_PARAMETER_STRUCT(FSingleColorParameters, MESHPAINTERSHADERCORE_API)
SHADER_PARAMETER(FVector3f, Color)
SHADER_PARAMETER(float, Time)
END_SHADER_PARAMETER_STRUCT()

class FSingleColorShaderVS : public FGlobalShader
{
	DECLARE_EXPORTED_GLOBAL_SHADER(FSingleColorShaderVS, MESHPAINTERSHADERCORE_API);
	SHADER_USE_PARAMETER_STRUCT(FSingleColorShaderVS, FGlobalShader);

	using FParameters = FSingleColorParameters;

public:
	// Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
	
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(InParameters, OutEnvironment);
		//OutEnvironment.SetDefine(TEXT("TILE_SIZE"), FComputeShaderUtils::kGolden2DGroupSize);
	}
};

class FSingleColorShaderPS : public FGlobalShader
{
	DECLARE_EXPORTED_GLOBAL_SHADER(FSingleColorShaderPS, MESHPAINTERSHADERCORE_API);
	SHADER_USE_PARAMETER_STRUCT(FSingleColorShaderPS, FGlobalShader);

	using FParameters = FSingleColorParameters;
	//typedef FSingleColorParameters FParameters;

public:
	// Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}

	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(InParameters, OutEnvironment);
		//OutEnvironment.SetDefine(TEXT("TILE_SIZE"), FComputeShaderUtils::kGolden2DGroupSize);
	}
};

class FSingleColorShaderCS : public FGlobalShader
{
	DECLARE_EXPORTED_GLOBAL_SHADER(FSingleColorShaderCS, MESHPAINTERSHADERCORE_API);
	SHADER_USE_PARAMETER_STRUCT(FSingleColorShaderCS, FGlobalShader);

	using FParameters = FSingleColorParameters;
	//typedef FSingleColorParameters FParameters;

public:
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(InParameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_X"), 1);
		OutEnvironment.SetDefine(TEXT("NUM_THREADS_Y"), 1);
	}
};