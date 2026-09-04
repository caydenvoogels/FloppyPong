using UnrealBuildTool;

public class FlailPong : ModuleRules
{
	public FlailPong(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"PhysicsCore",
			"Niagara",
			"ApplicationCore",
			"Slate",
			"SlateCore"
		});
	}
}
