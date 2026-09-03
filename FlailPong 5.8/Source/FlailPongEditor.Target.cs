using UnrealBuildTool;
using System.Collections.Generic;

public class FlailPongEditorTarget : TargetRules
{
	public FlailPongEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("FlailPong");
	}
}
