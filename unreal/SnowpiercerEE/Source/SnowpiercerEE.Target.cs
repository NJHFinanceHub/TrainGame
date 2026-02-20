using UnrealBuildTool;
using System.Collections.Generic;

public class SnowpiercerEETarget : TargetRules
{
    public SnowpiercerEETarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("SnowpiercerEE");
    }
}
