using UnrealBuildTool;
using System.IO;

public class ImGuiDebug : ModuleRules
{
	public ImGuiDebug(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// imgui headers accessible to this module and dependents
		string ImGuiPath = Path.Combine(PluginDirectory, "ThirdParty", "imgui");
		PublicIncludePaths.Add(ImGuiPath);

		// Export imgui symbols from this DLL so dependent modules can call ImGui functions
		PublicDefinitions.Add("IMGUI_API=IMGUIDEBUG_API");

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"RenderCore",
			"Slate",
			"SlateCore"
		});
	}
}
