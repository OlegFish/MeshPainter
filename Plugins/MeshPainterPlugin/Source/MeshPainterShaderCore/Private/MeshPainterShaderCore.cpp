#include "MeshPainterShaderCore.h"
#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FMeshPainterShaderCoreModule"

void FMeshPainterShaderCoreModule::StartupModule()
{
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("MeshPainterPlugin"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugin/MeshPainterPlugin"), PluginShaderDir);
}

void FMeshPainterShaderCoreModule::ShutdownModule()
{
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FMeshPainterShaderCoreModule, MeshPainterShaderCore)