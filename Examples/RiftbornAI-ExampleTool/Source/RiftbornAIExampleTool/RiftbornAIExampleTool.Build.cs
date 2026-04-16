// Copyright Your Studio. All Rights Reserved.
// Example plugin Build.cs — minimal dependencies required to author RiftbornAI tools.

using UnrealBuildTool;

public class RiftbornAIExampleTool : ModuleRules
{
	public RiftbornAIExampleTool(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core",
			"CoreUObject",
			"Engine",
		});

		// RiftbornAI is the only dependency needed to register tools.
		// Pull in Projects for FApp (used by the example get_project_name tool).
		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"RiftbornAI",
			"Projects",
		});
	}
}
