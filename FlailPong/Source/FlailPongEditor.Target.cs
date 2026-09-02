using UnrealBuildTool;
using System.Collections.Generic;

public class FlailPongEditorTarget : TargetRules
{
	public FlailPongEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("FlailPong");
	}
}
