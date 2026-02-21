using UnrealBuildTool;
using System.Collections.Generic;

public class SnowpiercerEEEditorTarget : TargetRules
{
    public SnowpiercerEEEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.AddRange(new string[] { "SnowpiercerEE", "TrainGame", "SnowyEngine" });
    }
}
