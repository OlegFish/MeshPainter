//using UnrealBuildTool;
using System.IO;
using UnrealBuildTool;

public class MeshPainterShaderCore : ModuleRules
{
    public MeshPainterShaderCore(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        //		PublicIncludePaths.AddRange(
        //			new string[] {
        //				// ... add public include paths required here ...
        //			}
        //			);

        //		PrivateIncludePaths.AddRange(
        //			new string[] {
        //				// ... add other private include paths required here ...
        //			}
        //			);
        PrivateIncludePaths.AddRange(new string[] { Path.Combine(GetModuleDirectory("Renderer"), "Private"), });

        //		PublicDependencyModuleNames.AddRange(
        //			new string[]
        //			{
        //				"Core",
        //				// ... add other public dependencies that you statically link with here ...
        //			}
        //			);
        PublicDependencyModuleNames.AddRange(new string[] { "Core" });

        //		PrivateDependencyModuleNames.AddRange(
        //			new string[]
        //			{
        //				"CoreUObject",
        //				"Engine",
        //				"Slate",
        //				"SlateCore",
        //				// ... add private dependencies that you statically link with here ...	
        //			}
        //			);
        PrivateDependencyModuleNames.AddRange(new string[] { "CoreUObject", "Engine", "Projects", "RenderCore", "Renderer", "RHI" });

        //		DynamicallyLoadedModuleNames.AddRange(
        //			new string[]
        //			{
        //				// ... add any modules that your module loads dynamically here ...
        //			}
        //			);
        //	}
        //}
        DynamicallyLoadedModuleNames.AddRange(new string[] { });
    }
}
