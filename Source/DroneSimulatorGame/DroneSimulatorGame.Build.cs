using UnrealBuildTool;

public class DroneSimulatorGame : ModuleRules
{
	public DroneSimulatorGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PhysicsCore" });
		PublicDependencyModuleNames.AddRange(new string[] { "DroneSimulatorInput", "DroneSimulatorCore" });

		// PrivateDependencyModuleNames.AddRange(new string[] { "DeveloperSettings" });

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "AssetTools", "UnrealEd" });
		}

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
