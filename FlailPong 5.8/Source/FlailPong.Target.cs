using UnrealBuildTool;
using System.Collections.Generic;

public class FlailPongTarget : TargetRules
{
	public FlailPongTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("FlailPong");
	}
}
