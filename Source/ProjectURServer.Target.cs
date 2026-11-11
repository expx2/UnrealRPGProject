// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class ProjectURServerTarget : TargetRules
{
	public ProjectURServerTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V6;
		IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
		ExtraModuleNames.Add("URGame");
	}
}
