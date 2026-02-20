using UnrealBuildTool;

public class SnowpiercerEE : ModuleRules
{
    public SnowpiercerEE(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "EnhancedInput",
            "UMG",
            "Slate",
            "SlateCore",
            "AIModule",
            "NavigationSystem",
            "GameplayTasks",
            "TrainGame",
            "SnowyEngine"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AudioMixer",
            "MetasoundFrontend",
            "MetasoundEngine"
        });
    }
}
