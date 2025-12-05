using UnrealBuildTool;

public class DroneSimulator : ModuleRules
{
	public DroneSimulator(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PhysicsCore", "DroneSimulatorInput" });

		// PrivateDependencyModuleNames.AddRange(new string[] { "DeveloperSettings" });

		if (Target.bBuildEditor)
		{
			PrivateDependencyModuleNames.AddRange(new string[] { "AssetTools", "UnrealEd" });
		}

		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });
	}
}
