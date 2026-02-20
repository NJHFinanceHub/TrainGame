using UnrealBuildTool;
using System.Collections.Generic;

public class SnowpiercerEETarget : TargetRules
{
    public SnowpiercerEETarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_5;
        ExtraModuleNames.Add("SnowpiercerEE");
    }
}
