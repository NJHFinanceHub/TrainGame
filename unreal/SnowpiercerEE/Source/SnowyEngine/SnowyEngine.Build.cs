// Copyright Snowpiercer: Eternal Engine. All Rights Reserved.

using UnrealBuildTool;
using System.IO;

public class SnowyEngine : ModuleRules
{
	public SnowyEngine(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Allow module-qualified includes (e.g. "SnowyEngine/Crafting/CraftingTypes.h")
		PublicIncludePaths.Add(Path.Combine(ModuleDirectory, ".."));

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"InputCore",
			"GameplayTags"
		});
	}
}
