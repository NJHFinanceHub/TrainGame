using UnrealBuildTool;
using System.IO;

public class SnowpiercerEE : ModuleRules
{
    public SnowpiercerEE(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        // Module root in include path so subdirs (UI/, VFX/, Exploration/) can find root headers
        PublicIncludePaths.Add(ModuleDirectory);
        // Allow module-qualified includes (e.g. "SnowpiercerEE/SEETypes.h")
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, ".."));

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
            "SnowyEngine",
            "Niagara"
        });

        PrivateDependencyModuleNames.AddRange(new string[]
        {
            "AudioMixer",
            "MetasoundFrontend",
            "MetasoundEngine"
        });
    }
}
