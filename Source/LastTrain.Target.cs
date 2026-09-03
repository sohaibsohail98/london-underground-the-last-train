using UnrealBuildTool;

public class LastTrainTarget : TargetRules
{
	public LastTrainTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		DefaultBuildSettings = BuildSettingsVersion.V7;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_8;
		ExtraModuleNames.Add("LastTrain");
	}
}
