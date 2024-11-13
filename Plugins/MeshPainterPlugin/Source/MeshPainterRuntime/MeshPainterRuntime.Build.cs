using System.IO;
using UnrealBuildTool;

public class MeshPainterRuntime : ModuleRules
{
	public MeshPainterRuntime(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.AddRange(new string[] {  });
        PublicDependencyModuleNames.AddRange(new string[] { "Core" });
        PrivateDependencyModuleNames.AddRange( new string[] { "CoreUObject", "Engine", "Slate", "SlateCore", "RenderCore", "Renderer", "RHI", "MeshPainterShaderCore" });
        DynamicallyLoadedModuleNames.AddRange(new string[] { });
    }
}
