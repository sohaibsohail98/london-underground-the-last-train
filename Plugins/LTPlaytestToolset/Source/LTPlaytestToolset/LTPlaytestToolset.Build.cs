using UnrealBuildTool;

public class LTPlaytestToolset : ModuleRules
{
	public LTPlaytestToolset(ReadOnlyTargetRules Target) : base(Target)
	{
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"ToolsetRegistry"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"EnhancedInput",
			"UnrealEd"
		});
	}
}
