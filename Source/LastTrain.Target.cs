using UnrealBuildTool;

public class LastTrainTarget : TargetRules
{
	public LastTrainTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V5;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_4;
		ExtraModuleNames.Add("LastTrain");
	}
}
