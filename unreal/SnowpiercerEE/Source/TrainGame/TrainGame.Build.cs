// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class TrainGame : ModuleRules
{
	public TrainGame(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Allow module-qualified includes (e.g. "TrainGame/Core/CombatTypes.h")
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, ".."));

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"EnhancedInput",
			"GameplayAbilities",
			"GameplayTags",
			"GameplayTasks",
			"SnowyEngine",
			"AIModule",
			"NavigationSystem"
		});

		// Include paths only (no link dep) — allows Slate widgets to include SnowpiercerEE headers
		PrivateIncludePathModuleNames.AddRange(new string[]
		{
			"SnowpiercerEE"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Slate",
			"SlateCore",
			"UMG",
			"AudioMixer",
			"MetasoundFrontend",
			"MetasoundEngine",
			"AIModule",
			"NavigationSystem"
		});
	}
}
