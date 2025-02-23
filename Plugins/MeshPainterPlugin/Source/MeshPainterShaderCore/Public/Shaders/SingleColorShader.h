#pragma once

#include "CoreMinimal.h"
#include "ShaderParameterStruct.h"

/** —оздаем структуру с параметрами дл€ шейдера
»спользуем здесь макросы, а не код, потомучто UE так надо чтобы состыковать —++ код с шейдерами */
BEGIN_SHADER_PARAMETER_STRUCT(FSingleColorParameters, MESHPAINTERSHADERCORE_API)
SHADER_PARAMETER(FVector3f, Color)
SHADER_PARAMETER(float, Time)
END_SHADER_PARAMETER_STRUCT()

/** —оздаем класс шейдера (shader class), в данном случае вертекс шейдер
“олько к шейдер классам можно прив€зывать шейдер-файлы (.usf файлы) */
class MESHPAINTERSHADERCORE_API FSingleColorShaderVS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FSingleColorShaderVS);
	SHADER_USE_PARAMETER_STRUCT(FSingleColorShaderVS, FGlobalShader);

	/** FParameters - тип параметров этого шейдера
	¬ качестве FParameters используем FSingleColorParameters */
	using FParameters = FSingleColorParameters;

public:
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(InParameters, OutEnvironment);
		//OutEnvironment.SetDefine(TEXT("TILE_SIZE"), FComputeShaderUtils::kGolden2DGroupSize);
	}
};

class MESHPAINTERSHADERCORE_API FSingleColorShaderPS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FSingleColorShaderPS);
	SHADER_USE_PARAMETER_STRUCT(FSingleColorShaderPS, FGlobalShader);

	using FParameters = FSingleColorParameters;
	//typedef FSingleColorParameters FParameters;

public:
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(InParameters, OutEnvironment);
		//OutEnvironment.SetDefine(TEXT("TILE_SIZE"), FComputeShaderUtils::kGolden2DGroupSize);
	}
};

class MESHPAINTERSHADERCORE_API FSingleColorShaderCS : public FGlobalShader
{
	DECLARE_GLOBAL_SHADER(FSingleColorShaderCS);
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