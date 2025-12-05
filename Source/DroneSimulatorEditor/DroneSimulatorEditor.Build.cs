using UnrealBuildTool;

public class DroneSimulatorEditor : ModuleRules
{
	public DroneSimulatorEditor(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"AssetTools"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd",
			"Slate",
			"SlateCore",
			"AssetTools",
			"AssetRegistry",
			"EditorStyle",
			"ToolMenus",
			"DroneSimulator",
			"WorkspaceMenuStructure",
			"InputCore",
			"LevelEditor",
			"PropertyEditor",
			"Json",
			"JsonUtilities"
		});
	}
}
