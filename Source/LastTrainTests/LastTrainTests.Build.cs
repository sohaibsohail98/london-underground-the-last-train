using UnrealBuildTool;

public class LastTrainTests : ModuleRules
{
	public LastTrainTests(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
			"LastTrain"
		});

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"UnrealEd"
		});
	}
}
