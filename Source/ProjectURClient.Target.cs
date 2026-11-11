// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectURClientTarget : TargetRules
{
	public ProjectURClientTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Client;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("URGame");
	}
}
