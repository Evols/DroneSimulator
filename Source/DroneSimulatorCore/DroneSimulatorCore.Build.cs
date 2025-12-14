using UnrealBuildTool;

public class DroneSimulatorCore : ModuleRules
{
	public DroneSimulatorCore(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine" });

		var bWithDroneInput = true;
		if (bWithDroneInput)
		{
			PrivateDependencyModuleNames.Add("DroneSimulatorInput");
			PublicDefinitions.Add("WITH_DRONE_INPUT=1");
		}
	}
}
