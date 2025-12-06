using UnrealBuildTool;

public class DroneSimulatorInput : ModuleRules
{
	public DroneSimulatorInput(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "PhysicsCore" });


		PrivateDependencyModuleNames.AddRange(new string[] { "InputDevice", "DeveloperSettings", "Json", "JsonUtilities" });

		// Uncomment if you are using Slate UI
		PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

		// Uncomment if you are using online features
		// PrivateDependencyModuleNames.Add("OnlineSubsystem");

		// To include OnlineSubsystemSteam, add it to the plugins section in your uproject file with the Enabled attribute set to true


		if (Target.Platform == UnrealTargetPlatform.Win64)
		{
			PrivateDependencyModuleNames.Add("RawInput");
			PublicSystemLibraries.AddRange(new string[] { "hid.lib", "setupapi.lib" });
		}
		else if (Target.Platform == UnrealTargetPlatform.Mac)
		{
			PublicFrameworks.AddRange(new string[] { "IOKit", "CoreFoundation" });
		}
	}
}
